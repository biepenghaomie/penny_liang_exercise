/*
  Copyright (c) 2019 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

	  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Xie Han (xiehan@sogou-inc.com;63350856@qq.com)
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include<iostream>
#include<curl/curl.h>
#include "nlohmann/json.hpp"

using namespace std;
using json=nlohmann::json;

size_t req_reply(void *ptr,size_t size,size_t nmemb,void *stream){
    string *str=(string*)stream;
    (*str).append((char*)ptr,size*nmemb);
    return size*nmemb;
}
/**
 * @brief 获取api返回结果
 * @param title 需要用于检测情感的内容
 */
void get_api_result(string title,bool *ans){
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl=curl_easy_init();
    string result;
    if(curl){
        const char *url="http://baobianapi.pullword.com:9091/get.php";
        curl_easy_setopt(curl,CURLOPT_URL,url);
        curl_easy_setopt(curl,CURLOPT_POST,1);
        curl_easy_setopt(curl,CURLOPT_POSTFIELDS,title.c_str());
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,req_reply);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *)&result);
    }   
    CURLcode res=curl_easy_perform(curl);
    bool flag=false;
    if(res==CURLE_OK){
        json api_res=json::parse(result);
        double d_api_res=api_res["result"];
        //cout<<d_api_res;
        flag=d_api_res<-0.5?true:false;
    }   
    else{
        cout<<"cant receive result,try again\n";
    }   
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    *ans=flag;
}
/**
 * @brief 对获取的html文件进行初步加工，提取出所有形如<a href="http...</a>的字符串
 * @param str 需进行提取的html字符串
*/
void rough_cut(string &str){
    string head="<a href=\"http";
    string end="</a>";
    int last_pos=str.find(head);
    if(last_pos<0)return;
    str.erase(str.begin(),str.begin()+last_pos);
    last_pos=0;
    while(1){
        int link_start=str.find(end,last_pos);
        if(link_start<0)break;
        last_pos=link_start;
        int link_end=str.find(head,last_pos);
        auto it=str.begin();
        if(link_end<0){
            str.erase(it+link_start+4,str.end());
            break;
        }
        str.erase(it+link_start+4,it+link_end);
        last_pos=str.find(head,last_pos);
    }
}
/**
 * @brief 删除html文本中标题含有的特殊字符
 */
void del_char(string &str,string p,int len){
    int del_pos=str.find(p);
    if(del_pos>=0)str.erase(del_pos,len);
}
/**
 * @brief 获取百度新闻上的新闻
 */
vector<pair<string,string>> get_news(){
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl=curl_easy_init();
    string get_html;
    if(curl){
        const char *url="http://news.baidu.com/";
        curl_easy_setopt(curl,CURLOPT_URL,url);
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,req_reply);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *)&get_html);
    }
    CURLcode res=curl_easy_perform(curl);
    curl_global_cleanup();
    rough_cut(get_html);
	vector<pair<string,string>>vec;
    while(1){
        int st=get_html.find("<a href");
        if(st<0)break;
        int ed=get_html.find("</a>");
        string str=get_html.substr(st,ed-st+4);
        get_html.erase(get_html.begin()+st,get_html.begin()+ed+4);
        if((int)str.find("target")<0)continue;
        //删掉标题中的多余html字符
        del_char(str,"<b>",3);
        del_char(str,"</b>",4);
        del_char(str,"<br/>",5);

        ed=str.find("</a>");
        int title_pos=str.rfind(">",ed);
        string title=str.substr(title_pos+1,ed-title_pos-1);
        if(title.size()==0)continue;
		st=str.find("http");
		ed=str.find("\"",st);
		string url=str.substr(st,ed-st-1);
		vec.push_back(make_pair(title,url));
/*
        if(get_api_result(title)){
			str.find("http");
            cout<<title<<"\n";

        }
*/
    }
	return vec;
    //cout<<get_html<<endl;
}

void process(WFHttpTask *server_task)
{
	protocol::HttpRequest *req = server_task->get_req();
	protocol::HttpResponse *resp = server_task->get_resp();
	long long seq = server_task->get_task_seq();
	protocol::HttpHeaderCursor cursor(req);
	std::string name;
	std::string value;
	//char buf[8192]="1234567890";
	//int len;
	vector<pair<string,string>>news=get_news();
	vector<thread>t(news.size());
	unique_ptr<bool[]>res(new bool[news.size()]);
	for(int i=0;i<news.size();i++){
		t[i]=thread(get_api_result,news[i].first,res.get()+i);
	}
	for(int i=0;i<t.size();i++){
		t[i].join();
	}
	for(int i=0;i<news.size();i++){
		if(res[i]){
			json j;
			j["title"]=news[i].first;
			j["url"]=news[i].second;
			string tmp=j.dump()+"\n";
			resp->append_output_body(tmp.c_str(),tmp.size());
		}
	}
	/* Set response message body. */
/*
	resp->append_output_body_nocopy("<html>", 6);
	len = snprintf(buf, 8192, "<p>%s %s %s</p>", req->get_method(),
				   req->get_request_uri(), req->get_http_version());
	resp->append_output_body(buf, len);

	while (cursor.next(name, value))
	{
		len = snprintf(buf, 8192, "<p>%s: %s</p>", name.c_str(), value.c_str());
		resp->append_output_body(buf, len);
	}
*/
//	resp->append_output_body_nocopy("</html>", 7);

	/* Set status line if you like. */
	server_task->set_send_timeout(3*1000);//add
	resp->set_http_version("HTTP/1.1");
	resp->set_status_code("200");
	resp->set_reason_phrase("OK");
	
	resp->add_header_pair("Content-Type","text/json;;charset=UTF-8");//add
//	resp->add_header_pair("Content-Type", "text/html");
	resp->add_header_pair("Server", "Sogou WFHttpServer");
	if (seq == 9) /* no more than 10 requests on the same connection. */
		resp->add_header_pair("Connection", "close");

	/* print some log */
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;

	server_task->get_peer_addr((struct sockaddr *)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
	else
		strcpy(addrstr, "Unknown");

	fprintf(stderr, "Peer address: %s:%d, seq: %lld.\n",
			addrstr, port, seq);
}

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

int main(int argc, char *argv[])
{
	unsigned short port;

	if (argc != 2)
	{
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		exit(1);
	}

	signal(SIGINT, sig_handler);

	WFHttpServer server(process);
	port = atoi(argv[1]);
	if (server.start(port) == 0)
	{
		wait_group.wait();
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}

	return 0;
}

