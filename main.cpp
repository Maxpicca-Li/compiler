#include<bits/stdc++.h>
#include "info.hpp"
// #define SUBMIT 1
using namespace std;

#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
#else
    // const string inFile = "./data/1/testfile8.txt";
    const string inFile = "./testfile_lyq.txt";
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

inline void clearAll(){
    ifp.close();
    ofp.close();
}

inline void roll_back(long n){ // 回退几个位置
    ifp.seekg(n,ios::cur);
}

/* 根据getsym的结果，进行判断 */
void judge(string& srcStr, int caseCode=0){
    string copyStr(srcStr);
    for(char& ch:copyStr){
        ch = tolower(ch);
    }
    string cateCodeStr;
    switch (caseCode){
    case specialStrCode:
        if(specialCateCodeMap.find(copyStr)!=specialCateCodeMap.end()){ // 在已知关键字里
            cateCodeStr = specialCateCodeMap[copyStr];
        }else{
            cateCodeStr = IDENFR; // 不在，则为标识符
        }
        break;
    case commonStrCode:
        cateCodeStr = STRCON;
        break;
    case numberCode:
        cateCodeStr = INTCON; // 如果要计算number,可以单独写个函数
        break;
    case charCode:
        cateCodeStr = CHARCON;
        break;
    default:
        break;
    }
    ofp << cateCodeStr << " " << srcStr << endl; // 输出结果
}


/* 判断下一个可以规约的串 */
void getsym(){
    getch(ch); // 获取首字符
    if(ch=='\0') return; // 首字符为空
    string srcStr;
    if(isalpha(ch) || ch=='_'){ // 关键字TK或标识符idenfr
        while(isalnum(ch) || ch=='_'){ // 需要记录原始字符串
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr,specialStrCode);
        roll_back(-1);// 回退一个指针
    }else if(isdigit(ch)){ // intcon
        while(isdigit(ch)){
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr, numberCode);
        roll_back(-1);
    }else if(ch=='='){ // 赋值
        while(ch=='='){ // 可能的逻辑运算 ==
            srcStr+=ch;
            getch(ch);
        }
        judge(srcStr, specialStrCode);
        roll_back(-1);
    }else if(ch=='+'){ // 数字运算，单独写，为了后续方便运算标识
        srcStr+=ch;
        judge(srcStr, specialStrCode);
    }else if(ch=='-'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
    }else if(ch=='*'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
    }else if(ch=='/'){
        srcStr+=ch;
        judge(srcStr, specialStrCode);
    }else if(ch=='!'){ // 逻辑运算
        srcStr+=ch;
        getch(ch);
        if(ch != '='){  // !=
            error(currLineNumber,illegalOp);
            roll_back(-1);
            return;
        }else{
            srcStr+=ch;
            judge(srcStr, specialStrCode);
        }
    }else if(ch=='<'){
        srcStr+=ch;
        getch(ch);
        if(ch == '='){  // <=
            srcStr+=ch;
        }else{
            roll_back(-1);
        }
        judge(srcStr, specialStrCode);
    }else if(ch=='>'){
        srcStr+=ch;
        getch(ch);
        if(ch == '='){ // <=
            srcStr+=ch;
        }else{
            roll_back(-1);
        }
        judge(srcStr, specialStrCode);
    }else if(ch==';' || ch==',' || ch==':'){ // 标点符号
        srcStr+=ch;
        judge(srcStr, specialStrCode);
    }else if(ch=='\"'){ // strcon
        int save_currLineNumber = currLineNumber;
        char lastch = ch;
        while(true){  // ", 成对处理
            getch(ch);
            if(lastch!='\\' && ch=='\"') {
                break;
            }else if(ch=='\n'){
                error(save_currLineNumber, illegalComma);
                break;
            }
            srcStr+=ch;
            lastch = ch;
        }
        judge(srcStr,commonStrCode);
    }else if(ch=='\''){ // charcon
        if(ch=='\\'){ // 转义字符
            getch(ch);
            if(isdigit(ch)) ch=ch-'0';
        }else{
            getch(ch); 
        }
        srcStr += ch;
        getch(ch);
        if(ch!='\''){  // 判断：一个完整的字符,且只有一个字符
            error(currLineNumber,illegalComma);
            roll_back(-1);
            return;
        }
        judge(srcStr,charCode);
    }else if(ch=='('){ // 小括号
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        while(ch!=')' && ch!='\0' && ch!='}') { // 递归调用gensym，直到反向括号出现
            getsym(); // FIXME: 对于大的程序不友好，容易卡栈空间
        }
        if(ch==')'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
        }else if(ch=='}') { // 程序段的结束
            error(save_currLineNumber,mismatchLittle);
            return; 
        }else error(save_currLineNumber,mismatchLittle);
    }else if(ch=='{'){ // 大括号
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        while(ch!='}' && ch!='\0') { // 递归调用gensym，直到反向括号出现
            getsym();
        }
        if(ch=='}'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
        }else error(save_currLineNumber,mismatchBig);
    }else if(ch=='['){ // 中括号
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        judge(srcStr,specialStrCode);
        while(ch!=']' && ch!='\0' && ch!=']') {  // 递归调用gensym，直到反向括号出现
            getsym();
        }
        if(ch==']'){
            string srcStr;
            srcStr+=ch;
            judge(srcStr,specialStrCode);
        }else if(ch=='}') { // 程序段的结束
            error(save_currLineNumber,mismatchLittle);
            return; 
        }else error(save_currLineNumber,mismatchMiddle);
    }
}

int main(){
    ifp.open(inFile, ios::binary);  // 因为涉及到字节回退，这里最好是binary打开
    ofp.open(outFile);
    while(!ifp.eof()){
        getsym();
    }
    clearAll();
    return 0;
}