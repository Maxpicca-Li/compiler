#include<bits/stdc++.h>
#include "info.hpp"
#include "error.hpp"
#include "util.hpp"
using namespace std;

#define SUBMIT 1
#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
#else
    // const string inFile = "./data/1/testfile8.txt";
    const string inFile = "./testerror.txt";
    const string outFile = "output_lyq.txt";
#endif

ifstream ifp;
ofstream ofp;
char ch;
vector<string> tokens;

inline void getch(char& ch){
    ifp.read((char*)&ch,1);
    if(ifp.eof()) ch='\0'; // EOF的判断 
    currCol++;
    if(ch=='\n') {
        currCol=0;
        currLineNumber++;
    }
}

inline void clearAll(){
    ifp.close();
    ofp.close();
}

inline void roll_back(long n){ // 回退几个位置
    ifp.seekg(n,ios::cur);
}

/* 根据getsym的结果，进行判断 */
void genToken(string& srcStr, int caseCode=0){
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
    
    // 一个token的输入 ==> 如何就地进行语法分析 ==> 巨大的状态表和分析表
    tokens.push_back(cateCodeStr);
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
        genToken(srcStr,specialStrCode);
        roll_back(-1);// 回退一个指针
    }else if(isdigit(ch)){ // intcon
        while(isdigit(ch)){
            srcStr+=ch;
            getch(ch);
        }
        genToken(srcStr, numberCode);
        roll_back(-1);
    }else if(ch=='='){ // 赋值
        while(ch=='='){ // 可能的逻辑运算 ==
            srcStr+=ch;
            getch(ch);
        }
        genToken(srcStr, specialStrCode);
        roll_back(-1);
    }else if(ch=='+'){ // 数字运算，单独写，为了后续方便运算标识
        srcStr+=ch;
        genToken(srcStr, specialStrCode);
    }else if(ch=='-'){
        srcStr+=ch;
        genToken(srcStr, specialStrCode);
    }else if(ch=='*'){
        srcStr+=ch;
        genToken(srcStr, specialStrCode);
    }else if(ch=='/'){
        srcStr+=ch;
        genToken(srcStr, specialStrCode);
    }else if(ch=='!'){ // 逻辑运算
        srcStr+=ch;
        getch(ch);
        if(ch != '='){  // !=
            error(currLineNumber,illegalOp);
            roll_back(-1);
            return;
        }else{
            srcStr+=ch;
            genToken(srcStr, specialStrCode);
        }
    }else if(ch=='<'){
        srcStr+=ch;
        getch(ch);
        if(ch == '='){  // <=
            srcStr+=ch;
        }else{
            roll_back(-1);
        }
        genToken(srcStr, specialStrCode);
    }else if(ch=='>'){
        srcStr+=ch;
        getch(ch);
        if(ch == '='){ // <=
            srcStr+=ch;
        }else{
            roll_back(-1);
        }
        genToken(srcStr, specialStrCode);
    }else if(ch==';' || ch==',' || ch==':'){ // 标点符号
        srcStr+=ch;
        genToken(srcStr, specialStrCode);
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
        genToken(srcStr,commonStrCode);
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
        genToken(srcStr,charCode);
    }else if(ch=='('){ // 小括号
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        leftBrack_LN.push({ch,currLineNumber});
    }else if(ch=='{'){ // 大括号
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        leftBrack_LN.push({ch,currLineNumber});
    }else if(ch=='['){ // 中括号
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        leftBrack_LN.push({ch,currLineNumber});
    }else if(ch==')'){
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        char nearBrack = leftBrack_LN.top().first;  // 匹配判断
        if(nearBrack!='(') error(save_currLineNumber,mismatchError[ch]);
        else leftBrack_LN.pop(); // FIXME: pop的位置不知道对不对            
    }else if(ch==']'){
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        char nearBrack = leftBrack_LN.top().first;  // 匹配判断
        if(nearBrack!='[') error(save_currLineNumber,mismatchError[ch]);
        else leftBrack_LN.pop(); // FIXME: pop的位置不知道对不对            
    }else if(ch=='}'){
        int save_currLineNumber = currLineNumber;
        srcStr+=ch;
        genToken(srcStr,specialStrCode);
        // 一段的结束
        while(!leftBrack_LN.empty() && leftBrack_LN.top().first!='{') {
            error(leftBrack_LN.top().second, mismatchError[leftBrack_LN.top().first]);
            leftBrack_LN.pop();
        }
        if(leftBrack_LN.empty() || leftBrack_LN.top().first!='{'){
            error(save_currLineNumber,mismatchError[ch]);
        }else{
            leftBrack_LN.pop();
        }
    }
}

// int next(){

// }
void getParser(){
    // 逐步对token进行分析，感觉可以依次读入那个文件夹
}



int main(){
    // testStrTransfer();

    ifp.open(inFile, ios::binary);  // 因为涉及到字节回退，这里最好是binary打开
    ofp.open(outFile);
    while(!ifp.eof()){
        getsym();
    }
    clearAll();
    return 0;
}