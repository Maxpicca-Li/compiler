import sys
import os
import difflib

def check_difference(file1, file2):
    st1 = os.stat(file1)
    st2 = os.stat(file2)
    # if st1.st_size != st2.st_size:
    #     return True
    with open(file1, 'r') as fp1, open(file2, 'r') as fp2:
        t1 = fp1.readlines()
        t2 = fp2.readlines()
        for i in range(len(t1)):
            if t1[i] != t2[i]:
                return True
    return False

if __name__=="__main__":
    src_str = "./edata/%d/output%d.txt"
    my_str = "./edata/%d/eoutput%d.txt"
    for j in range(1,3):
        for i in range(1,11):
            src_file = src_str%(j,i)
            my_file = my_str%(j,i)
            print("===============")
            # os.system(f"copy ./t1/{j}/testfile{i}.txt ./a.in")
            # os.system('./a')
            src_lines = ''
            my_lines = ''
            with open(src_file, "r", encoding='utf8') as f:
                src_lines = f.readlines()
            with open(my_file, "r", encoding='utf8') as f:
                my_lines = f.readlines()

            dif = check_difference(src_file, my_file)
            if dif:
                d = difflib.HtmlDiff()
                html_content = d.make_file(src_lines, my_lines)
                with open('diff.html','w') as f:
                    f.write(html_content)
                print(f'testfile {j}-{i} fail, check diff.html')
                exit()
            else:
                print(f'testfile {j}-{i} pass')