#include<bits/stdc++.h>
#include "info.hpp"
#define SUBMIT 1
using namespace std;

#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
#else
    const string inFile = "./data/1/testfile10.txt";
    const string outFile = "output_lyq.txt";
#endif

ifstream ifp;
ofstream ofp;
char ch;

inline void getch(char& ch){
    ifp.read((char*)&ch,1);
    if(ifp.eof()) ch='\0'; // EOF的判断 
    if(ch=='\n') currLineNumber++;
}

void clearAll(){
    ifp.close();
    ofp.close();
}


void judge(string& srcStr, int caseCode=0){
    // 根据getsym的结果，进行判断
    // 可能的结果：标识符/关键字
    // 需要全转为小写
    string copyStr(srcStr);
    for(char& ch:copyStr){
        ch = tolower(ch);
    }
    string cateCodeStr;
    switch (caseCode){
    case specialStrCode:
        // 是否在已知关键字里
        if(specialCateCodeMap.find(copyStr)!=specialCateCodeMap.end()){
            // 输出结果
            cateCodeStr = specialCateCodeMap[copyStr];
        }else{
            // 不在，则为标识符
            cateCodeStr = IDENFR;
        }
        break;
    case commonStrCode:
        cateCodeStr = STRCON;
        break;
    case numberCode:
        // 计算number,可以单独写个函数
        cateCodeStr = INTCON;
        break;
    case charCode:
        cateCodeStr = CHARCON;
        break;
    default:
        break;
    }
    // 输出结果
    // cout << cateCodeStr << " " << srcStr << endl;
    ofp << cateCodeStr << " " << srcStr << endl;
}


/* TODO 判断下一个可以规约的串 */
void getsym(){
    // ch当前就是指向下一个字符吧
    // getch(ch); // 获取首字符
    if(ch=='\0') return; // 首字符为空
    string srcStr;
    if(isalpha(ch) || ch=='_'){ // 关键字TK或标识符idenfr
        // 需要记录原始字符串
        while(isalnum(ch) || ch=='_'){
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr,specialStrCode);
    }else if(isdigit(ch)){ // 数字
        // intcon
        while(isdigit(ch)){
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr, numberCode);
    }else if(ch=='='){ // 赋值
        // 可能的逻辑运算 ==
        while(ch=='='){
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr, specialStrCode);
    }else if(ch=='+'){ // 数字运算，单独写，为了后续方便运算标识
        srcStr+=ch;
        judge(srcStr, specialStrCode);
        getch(ch); // 记得读取下一个字符
    }else if(ch=='-'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
        getch(ch); // 记得读取下一个字符
    }else if(ch=='*'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
        getch(ch); // 记得读取下一个字符
    }else if(ch=='/'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
        getch(ch); // 记得读取下一个字符
    }else if(ch=='!'){ // 逻辑运算
        // !=
        srcStr+=ch;
        getch(ch);
        if(ch != '='){
            error(currLineNumber,illegalOp);
            return;
        }else{
            srcStr+=ch;
            judge(srcStr, specialStrCode);
            // 读取
            getch(ch);
        }
    }else if(ch=='<'){
        // <=
        srcStr+=ch;
        getch(ch); // 记得读取下一个字符
        if(ch == '='){
            srcStr+=ch;
            getch(ch); // 记得读取下一个字符
        }
        judge(srcStr, specialStrCode);
    }else if(ch=='>'){
        // <=
        srcStr+=ch;
        getch(ch); // 记得读取下一个字符
        if(ch == '='){
            srcStr+=ch;
            getch(ch); // 记得读取下一个字符
        }
        judge(srcStr, specialStrCode);
    }else if(ch==';' || ch==',' || ch==':'){ // 标点符号
        srcStr+=ch;
        judge(srcStr, specialStrCode);
        getch(ch); // 记得读取下一个字符
    }else if(ch=='\"'){ // 成对的处理
        // 如果是冒号，里面是一个完整的字符换
        // strcon
        // 处理字符串
        string srcStr;
        char lastch = ch;
        while(true){
            getch(ch);
            if(lastch!='\\' && ch=='\"') {
                getch(ch); // 读取下一个字符
                break;
            }
            srcStr+=ch;
            lastch = ch;
        }
        judge(srcStr,commonStrCode);
    }else if(ch=='\''){
        // 如果是冒号，里面是一个完整的字符,且只有一个字符
        // charcon
        getch(ch);
        srcStr += ch;
        getch(ch); // 记得读取下一个字符
        if(ch!='\''){    
            error(currLineNumber,illegalComma);
            return;
        }else{
            getch(ch); // 记得读取下一个字符    
        }
        judge(srcStr,charCode);
    }else if(ch=='('){ // 括号
        // 递归调用gensym，直到反向括号出现
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        getch(ch);
        // 递归抵用
        while(ch!=')' && ch!='\0') {
            getsym();
        }
        // 此时ch指向下一个
        if(ch==')'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
            getch(ch);
        }
        else error(currLineNumber,mismatchLittle);
    }else if(ch=='{'){ // 括号
        // 递归调用gensym，直到反向括号出现
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        getch(ch);
        // 递归抵用
        while(ch!='}' && ch!='\0') {
            getsym();
        }
        if(ch=='}'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
            getch(ch);
        }
        else error(currLineNumber,mismatchBig);
    }else if(ch=='['){ // 括号
        // 递归调用gensym，直到反向括号出现
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        getch(ch);
        // 递归抵用
        while(ch!=']' && ch!='\0') {
            getsym();
        }
        if(ch==']'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
            getch(ch);
        }
        else error(currLineNumber,mismatchMiddle);
    }else{
        getch(ch);
    }
}

int main(){
    // 从头到尾判断
    ifp.open(inFile);
    ofp.open(outFile);
    getch(ch);
    while(!ifp.eof()){
        getsym();
    }
    clearAll();
    return 0;
}