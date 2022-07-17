# -*- coding: utf-8 -*-
"""
Created on Tue Aug 18 15:34:25 2020

@author: 67022
"""
import logging
import tornado
import tornado.ioloop
import tornado.web
import tornado.httpserver
import tornado.gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor
import json
from LAC import LAC
import nest_asyncio
nest_asyncio.apply()

LISTEN_PORT     = 8080
PROCESS_NUM     = 1

LOG_FORMAT = "%(asctime)s %(thread)s %(name)s %(levelname)s %(pathname)s %(message)s "#配置输出日志格式
DATE_FORMAT = '%Y-%m-%d  %H:%M:%S %a ' #配置输出时间的格式，注意月份和天数不要搞乱了
lacmod = LAC(mode='seg')
lacmod.load_customization('custom.txt', sep=None)
class LacHandler(tornado.web.RequestHandler):
    executor = ThreadPoolExecutor(100)
    @tornado.web.asynchronous
    @tornado.gen.coroutine
    def get(self):
        # 假如你执行的异步会返回值被继续调用可以这样(只是为了演示),否则直接yield就行
        res = yield self.handle()
        self.writejson(res)

    @tornado.gen.coroutine    
    def post(self):
        # 假如你执行的异步会返回值被继续调用可以这样(只是为了演示),否则直接yield就行
        res = yield self.handle()
        self.writejson(res)

    @run_on_executor 
    def handle(self):      
        try:
            bodyStr = str(self.request.body,"utf-8")   
            data = json.loads(bodyStr)
            text = data['text']
            seg_result = lacmod.run(text)
        except Exception as e:
            logging.error("%s" % str(e)+" "+str(self.request.remote_ip))
        finally:
            return seg_result
    
    def prepare_head(self):
        self.set_header("Content-Type","application/json;chatset=utf-8")
        self.set_status(200)
        
    def writejson(self,obj):
        jsonstr = json.dumps(obj,ensure_ascii=False)
        self.prepare_head()
        self.write(jsonstr)

settings = {

}

application = tornado.web.Application([
    (r"/lac",             LacHandler),      
], **settings)

if __name__ == "__main__":   
 
    myapplog = logging.getLogger()
    myapplog.setLevel(logging.INFO)
    formatter = logging.Formatter(LOG_FORMAT)
    filehandler = logging.handlers. RotatingFileHandler("log/lacsize.log", mode='a', maxBytes=1024*1024*100, backupCount=10)#每 102400Bytes重写一个文件,保留5(backupCount) 个旧文件
    filehandler.setFormatter(formatter)
    myapplog.addHandler(filehandler)    
    
    #application.listen(config.LISTEN_PORT)
    #tornado.ioloop.IOLoop.instance().start()
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.bind(LISTEN_PORT)
    http_server.start(PROCESS_NUM)
    tornado.ioloop.IOLoop.instance().start()

