# -*- coding: utf-8 -*-
"""
Created on Tue Aug 11 15:27:51 2020

@author: 67022
"""
import web 
from LAC import LAC
import logging
import json

#logger = logging.getLogger()

urls = (
    '/lac', 'lac'
)
app = web.application(urls, globals())
                  
class lac():
     #默认分词
     def POST(self):
        web.header("Access-Control-Allow-Origin", "*")
        web.header('content-type','text/json')    
        try:
          data = str(web.data(),"utf-8")
          #print("get request:",data)
          dataDic = json.loads(data)
          #print("1")
          #待分词文本
          text =  dataDic['text']
          #print("2")
          lacmodel= ''
          meddledic=''
          #装载指定模型
          if 'model' in dataDic.keys():
              lacmodel =  dataDic['model']
              #可以通过model 参数加载自己训练的模型
              lac = LAC(model_path=lacmodel)
          else: 
              lac = LAC(mode='seg')
          #装载干预词典, sep参数表示词典文件采用的分隔符，为None时默认使用空格或制表符'\t'
          if 'meddledic' in dataDic.keys():
              meddledic=  dataDic['meddledic']
              #可以加载自定义的干预词典，进行自定义的语义分词，改文件需要放在与当前文件同级 目录下
              lac.load_customization(meddledic, sep=None)
          #logger.info('lac input text:%s   lacmodel:%s   meddledic:%s' %(text,lacmodel,meddledic))
          #print("text is:",text)
          seg_result = lac.run(text)
          #logger.info(seg_result)
          #print("resutl:",seg_result)
          return seg_result
        except Exception as e:
          #logger.info("lac exception: %s: " %(str(e)))
          return [[],[]]
      
if __name__ == "__main__":
    app.run()
