import re
sens=input()
cut_sen=sens.split("，")
f_strs=open("./config_file","r").readlines()
for sen in cut_sen:
    for f_str in f_strs:
        f_str=re.split("[:\s]",f_str.strip("\n").replace(".",".*?"))
        for k in range (0,len(f_str),2):
            res=re.findall(f_str[k],sen)
            if len(res)!=0:
                rres=re.findall(f_str[k].replace(".*?","(.*?)"),sen)
                for t in range (len(res)):
                    sen=sen.replace(res[t],''.join(rres[t]),1)
                print(f_str[k].replace(".*?","."),f_str[k+1].replace(".*?","."))
                break
