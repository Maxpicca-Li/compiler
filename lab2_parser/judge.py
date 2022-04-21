import sys
import os
import difflib

def check_difference(file1, file2):
    st1 = os.stat(file1)
    st2 = os.stat(file2)
    with open(file1, 'r',encoding='gbk') as fp1, open(file2, 'r',encoding='gbk') as fp2:
        t1 = fp1.readlines()
        t2 = fp2.readlines()
        for i in range(min(len(t1),len(t2))):
            t1[i] = t1[i].strip() # 去除换行符
            t2[i] = t2[i].strip()
            if t1[i] != t2[i]:
                print("%s-%d,%s-%d"%(t1[i],len(t1[i]),t2[i],len(t2[i])))
                return True
    return False

if __name__=="__main__":
    src_str = "./edata/%d/output%d.txt"
    my_str = "./edata/%d/eoutput%d.txt"
    # src_str = "../data/%d/output%d.txt"
    # my_str = "../data/%d/myOutput%d.txt"
    for j in range(1,3):
        for i in range(1,11):
            src_file = src_str%(j,i)
            my_file = my_str%(j,i)
            print("===============")
            dif = check_difference(src_file, my_file)
            if dif:
                src_lines = ''
                my_lines = ''
                with open(src_file, "r", encoding='gbk') as f:
                    src_lines = f.readlines()
                with open(my_file, "r", encoding='gbk') as f:
                    my_lines = f.readlines()
                d = difflib.HtmlDiff()
                html_content = d.make_file(src_lines, my_lines)
                with open('diff.html','w') as f:
                    f.write(html_content)
                print(f'testfile {j}-{i} fail, check diff.html')
                exit()
            else:
                print(f'testfile {j}-{i} pass')