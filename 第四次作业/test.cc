#include<iostream>
#include<bits/stdc++.h>
#include<fstream>
#include<unistd.h>
#include<sys/wait.h>

using namespace std;

const int maxn = 5.2e7;
const int f_size = 200e6;
const int p_num = 8;
void write_d(ofstream &ofs, int data[], int len, int op, int flag){
	if(op == 0){
		for(int i = 0; i < len; i++){
			ofs.write((char*)&data[i], sizeof(int));
		}	
	}
	else{
		for(int i = 0; i < len; i++){
			ofs << data[i];
			if(!flag || i != len-1){
				ofs << '\n';
			}
		}
	}
}
void w_in_t(ofstream &ofs, int t[], int &len, int val, int op, int flag){
	if(len == maxn || flag == 1){
		write_d(ofs, t, len, op, flag);
		len = 0;
	}
	t[len++] = val;
}
queue<string> q;
void r_val(ifstream &ifs, int &val){
	ifs.read((char*)&val, sizeof(int));
}
void merge(vector<string> vec, ofstream &ofs, int op){
	vector<ifstream> ifs(vec.size());
	for(int i = 0; i < vec.size(); i++){
		ifs[i].open(vec[i].c_str(), ios::in | ios::binary);
	}
	int *t = new int[maxn << 1];
	int len = 0;
	int mn_pos = 0, mn_val;
	vector<int> val(vec.size());
	for(int i = 0; i < ifs.size(); i++){
		if(ifs[i].is_open()){
			r_val(ifs[i], val[i]);
			mn_val = val[i];
			mn_pos = i;
		}
	}
	int now = 0;
	while(1){
		mn_val = (1LL << 31) - 1;
		int flag = 0;
		for(int i = 0; i < ifs.size(); i++){
			if(ifs[i].is_open()){
				if(mn_val >= val[i]){
					mn_val = val[i];
					mn_pos = i;
					flag = 1;
				}
			}
		}
		if(!flag)break;
		w_in_t(ofs, t, len, val[mn_pos], op, 0);
		if(ifs[mn_pos].peek() == EOF){
			ifs[mn_pos].close();
			continue;
		}
		r_val(ifs[mn_pos], val[mn_pos]);
		now++;
	}
	w_in_t(ofs, t, len, 0, op, 1);
	for(int i = 0; i < ifs.size(); i++){
		if(ifs[i].is_open())ifs[i].close();
		remove(vec[i].c_str());
	}
	ofs.close();
	delete []t;
}

void mem_merge(int l, int r,int mid, int a[], int tmp[]){
	int lp = l, rp = mid+1, st = l;
	while(lp <= mid && rp <= r){
		if(a[lp] <= a[rp]){
			tmp[st++] = a[lp++];
		}
		else{
			tmp[st++] = a[rp++];
		}
	}
	while(lp <= mid){
		tmp[st++] = a[lp++];
	}
	while(rp <= r){
		tmp[st++] = a[rp++];
	}
	for(int i = l; i <= r; i++){
		a[i] = tmp[i];
	}
}

void m_sort(int l, int r, int a[], int tmp[]){
	if(l < r){
		int mid = l + r >> 1;
		m_sort(l, mid, a, tmp);
		m_sort(mid + 1, r, a, tmp );
		mem_merge(l, r, mid, a, tmp);
	}
}

void mem_m_sort(char *f_name, int len, int a[]){
	if(len != 1){
		int *tmp = new int[maxn];
		m_sort(0, len - 1, a, tmp);
		delete []tmp;
	}
	ofstream ofs(f_name, ios::out | ios::binary);
	write_d(ofs, a, len, 0, 0);
	ofs.close();
}

int t_data[maxn];
vector<pair<int,int>> vis;
map<int,int> done;
map<int,string> p_done;
int main(){
	auto start = chrono::high_resolution_clock::now();
	ifstream ifs("/data/int16GB.txt", ios::in);
	int len = 0 ,now = 1, sum_p = 0;
	char s[10] = {0};
	while(!ifs.eof() || len){
		int tmp;
		ifs >> tmp;
		if(!ifs.eof()){
			t_data[len++] = tmp;
		}
		if(len == maxn || ifs.eof() && len){
			sprintf(s, "%02d", now);
			if(sum_p >= p_num){
				int status;
				wait(&status);
				int fin = WEXITSTATUS(status);
				done[fin] = 1;
				sum_p--;
			}
			if(sum_p < p_num){
				int p_id = fork();
				if(p_id == 0){
					mem_m_sort(s, len, t_data);
					exit(now);
				}
				vis.push_back(make_pair(now, p_id));
				sum_p++;
			}
			len = 0;
			now++;
		}
		if(ifs.eof())break;
	}
	ifs.close();
	for(int i = 0; i < vis.size(); i++){
		if(done.find(vis[i].first) == done.end()){
			int status;
			waitpid(vis[i].second, &status, 0);
			int fin = WEXITSTATUS(status);
			done[fin] = 1;
			sum_p--;
		}
	}
	for(int i = 1; i <= vis.size(); i++){
		sprintf(s, "%02d", i);
		string t_s(s);
		q.push(t_s);
	}
	while(1){
		int p_id;
		while(q.size() >= 10){
			vector<string> vec;
			string n_name;
			for(int i = 0; i < 10; i++){
				n_name+=q.front();
				vec.push_back(q.front());
				q.pop();
			}
			if(sum_p >= p_num){
				p_id = wait(NULL);
				sum_p--;
				if(p_id != -1){
					q.push(p_done[p_id]);
				}
			}
			int p_id = fork();
			if(p_id == 0){
				ofstream ofs(n_name.c_str(), ios::out | ios::binary);
				merge(vec, ofs, 0);
				ofs.close();
				exit(0);
			}
			p_done[p_id] = n_name;
			sum_p++;
		}
		p_id = wait(NULL);
		if(p_id == -1){
			ofstream ofs("data.out", ios::out);
			vector<string> vec;
			while(!q.empty()){
				vec.push_back(q.front());
				q.pop();
			}
			merge(vec, ofs, 1);
			ofs.close();
			break;
		}
		q.push(p_done[p_id]);
	}
	auto end = chrono::high_resolution_clock::now();
	double total = chrono::duration<double, milli>(end - start).count();
	cout << "时间：" << total / 1000 << "s\n";
}