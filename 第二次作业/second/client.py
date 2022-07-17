import requests
from multiprocessing import Process
from threading import Thread
import time
import sys
import json

url = "http://127.0.0.1:8080/lac"
def get_json(text):
    return {"text": text}

def thr_(pro_id, thr_id, cut_res):
    file_name = "./result/" + chr(ord('a') + pro_id) + chr(ord('a') + thr_id)
    cut_len = len(cut_res)
    session_ = requests.session()
    with open(file_name,"w") as res_fd:
        for i in range (cut_len):
            text = cut_res[i].split('\t')
            text_len = len(text)
            for j in range (1, text_len-3):
                data = get_json(text[j])
                tmp_res = session_.post(url=url, data=json.dumps(data))
                res_fd.write(tmp_res.text+"\n")
    session_.close()

def pro_(pro_id, cut_res, thread_num):
    thr = []
    text_len = len(cut_res)
    cut_size = text_len//thread_num
    for i in range(thread_num):
        left = i*cut_size
        right = min((i+1)*cut_size, text_len)
        text_split = cut_res[left:right]
        thr_tmp = Thread(target = thr_, args = (pro_id, i, text_split))
        thr.append(thr_tmp)
    for i in range (thread_num):
        thr[i].start()
    for i in range (thread_num):
        thr[i].join()
        
if __name__ == '__main__':
    t_start = time.time()
    pro_num = int(sys.argv[1])
    thread_num = int(sys.argv[2])
    file_open = open(sys.argv[3])
    text = file_open.readlines()
    text_len = len(text)
    cut_size = text_len//pro_num
    pro = []
    for i in range (pro_num):
        left = i*cut_size
        right = min((i+1)*cut_size, text_len)
        text_split = text[left:right]
        pro_tmp = Process(target = pro_, args = (i, text_split, thread_num))
        pro.append(pro_tmp)
    for i in range (pro_num):
        pro[i].start()
    for i in range (pro_num):
        pro[i].join()
    t_end = time.time()
    print("second",t_end-t_start)
    print("min:",(t_end-t_start)/60)
