#pragma once
#include <bits/stdc++.h>
#include "info.hpp"
#include "ErrorHandler.hpp"
using namespace std;

inline bool isChar(char c){
    return isalnum(c) || (c=='_') || (c=='+') || (c=='-') || (c=='*') || (c=='/');
}

inline bool isStrChar(char c){
    return (c>=35&&c<=126) || (c==32) || (c==33);
}

class LexicalAnalyzer{
private:
    ifstream ifp;
    char ch;
    int currLineNumber = 1, currCol = 1;
    stack< tuple<char,int,int> > leftBrack_LN; // 匹配号，行号， 列
    vector<Token> tokens;
    int currIdx = 0,tot = 0;
    
public:
    Token currToken;
    
    void init(string inFile){
        if(inFile!="") ifp.open(inFile, ios::binary);  // 因为涉及到字节回退，这里最好是binary打开
        ifp.seekg(ios::beg);
        tokens.clear();
    }

    void doLexer(){
        tokens.clear();
        ifp.seekg(ios::beg);
        while(!ifp.eof()){
            next();
        }   
        ifp.seekg(ios::beg);
    }

    void printAllTokens(ostream& ofp){
        for(auto t:tokens){
            ofp << tokenId_str[t.type] << " " << t.valueStr << endl; // 输出结果
        }
    }

    /* 获取下一个，移动ifp指针 */
    inline void getch(){
        ifp.read((char*)&ch,1);
        if(ifp.eof()) ch='\0'; // EOF的判断 
        currCol++;
        if(ch=='\n') {
            currCol=0;
            currLineNumber++;
        }
    }

    /* 查看下一个，不移动ifp指针 */
    inline void peekch(){
        ch = ifp.peek();
        if(ifp.eof()) ch='\0'; // EOF的判断 
    }
    
    /* 回退n个位置 */
    inline void roll_back(long n){
        ifp.seekg(n,ios::cur);
    }

    Token nextToken(){
        int tmp = currIdx;
        if(tmp==this->tot) {
            cout << "到底了~，返回最后一个token"<<endl;
            currIdx -= 1;
        }
        return this->tokens[this->currIdx++];
    }

    Token peekToken(){
        int tmp = currIdx+1;
        if(tmp==this->tot) {
            cout << "到底了~，无法peek下一个Token,返回最后一个Token"<<endl;
            tmp -= 1;
        }
        return this->tokens[tmp];
    }

    bool roll_back_currIdx(){
        this->currIdx--;
        if(currIdx<0) {
            cout<<"Lexer无法再回退"<<endl;
            this->currIdx++;
            return false;
        }
        return true;
    }

    bool isEnd(){
        return (this->currIdx == this->tot);
    }

private:
    /* 判断下一个可以规约的串 */
    void next(){
        getch(); // 获取首字符
        while(ch==' ') getch();
        if(ch=='\0') return; // 首字符为空
        string srcStr;
        if(isalpha(ch) || ch=='_'){ // 关键字TK或标识符idenfr
            while(isalnum(ch) || ch=='_'){ // 需要记录原始字符串
                srcStr+=ch;
                getch();
            }
            genToken(srcStr,specialStrCode);
            roll_back(-1);// 回退一个指针
        }else if(isdigit(ch)){ // intcon
            while(isdigit(ch)){
                srcStr+=ch;
                getch();
            }
            genToken(srcStr, intCode);
            roll_back(-1);
        }else if(ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch==';' || ch==',' || ch==':'){ // 单符号
            srcStr+=ch;
            genToken(srcStr, specialStrCode);
        }else if(ch=='!'){ // 逻辑运算
            srcStr+=ch;
            getch();
            if(ch != '='){  // !=
                // 非法符号
                ehandler.error(currLineNumber,currCol, illegalLexcial);
                srcStr+='='; // 错误处理，加上非法符号
                roll_back(-1);
            }else{
                srcStr+=ch;    
            }
            genToken(srcStr, specialStrCode);
        }else if(ch=='<' || ch=='>' || ch=='='){
            srcStr+=ch;
            getch();
            if(ch == '='){  // <=
                srcStr+=ch;
            }else{
                roll_back(-1);
            }
            genToken(srcStr, specialStrCode);
        }else if(ch=='\"'){ // strcon
            char lastch = ch;
            while(true){  // ", 成对处理
                getch();
                if(lastch=='\"' && ch=='\"'){ // 空字符串
                    ehandler.error(currLineNumber, currCol, illegalLexcial);
                    srcStr += defaultChar;
                    break;
                }
                if(lastch!='\\' && ch=='\"') { // 正常退出
                    break;
                }else if(!isStrChar(ch)) { // 非法符号
                    ehandler.error(currLineNumber, currCol, illegalLexcial);
                    ch = defaultChar;
                }
                srcStr+=ch;
                lastch = ch;
            }
            genToken(srcStr,strCode);
        }else if(ch=='\''){      // charcon
            getch();             // 这里不会涉及转义字符
            if(ch=='\'') {       // 符号串中无字符
                ehandler.error(currLineNumber, currCol, illegalLexcial);
                ch = defaultChar;
                srcStr += ch; 
                genToken(srcStr,charCode);
            }else if(!isChar(ch)){ // 符号串为非法字符
                ehandler.error(currLineNumber, currCol, illegalLexcial);
                ch = defaultChar;
            }
            srcStr += ch; 
            getch();
            if(ch!='\''){
                ehandler.errorUnkown(currLineNumber, currCol, "少了一单引号");
                roll_back(-1);
            }
            genToken(srcStr,charCode);
        }else if(ch=='(' || ch=='[' || ch=='{'){ // 括号
            srcStr+=ch;
            genToken(srcStr,specialStrCode);
            // 这里不做匹配判断，由语法分析做
            // leftBrack_LN.push({ch,currLineNumber,currCol});
        }else if(ch==')'){
            srcStr+=ch;
            genToken(srcStr,specialStrCode);
            /* 
            char nearBrack = get<0>(leftBrack_LN.top());  // 匹配判断
            if(nearBrack!='(') ehandler.error(currLineNumber,currCol,shouldRsmall);
            else leftBrack_LN.pop();
            */
        }else if(ch==']'){
            srcStr+=ch;
            genToken(srcStr,specialStrCode);
            /* 
            char nearBrack = get<0>(leftBrack_LN.top());  // 匹配判断
            if(nearBrack!='[') ehandler.error(currLineNumber,currCol,shouldRmid);
            else leftBrack_LN.pop(); 
            */
        }else if(ch=='}'){
            srcStr+=ch;
            genToken(srcStr,specialStrCode);
            /* // 一段的结束
            while(!leftBrack_LN.empty() && get<0>(leftBrack_LN.top())!='{') {
                ehandler.error(get<1>(leftBrack_LN.top()), get<2>(leftBrack_LN.top()), illegalLexcial);
                leftBrack_LN.pop();
            }
            if(leftBrack_LN.empty() || get<0>(leftBrack_LN.top())!='{'){
                ehandler.error(currLineNumber,currCol, illegalLexcial);
            }else{
                leftBrack_LN.pop();
            }
            */
        }
    }

    /* 根据getsym的结果，进行判断 */
    void genToken(string& srcStr, int caseCode=0){
        string copyStr(srcStr);
        for(char& ch:copyStr){
            ch = tolower(ch);
        }
        TokenID tokenId = DEFAULT;
        switch (caseCode){
        case specialStrCode:
            if(specialCateCodeMap.find(copyStr)!=specialCateCodeMap.end()){ // 在已知关键字里
                tokenId = specialCateCodeMap[copyStr];
            }else{
                tokenId = IDENFR; // 不在，则为标识符
            }
            break;
        case strCode:
            tokenId = STRCON;
            break;
        case intCode:
            tokenId = INTCON; // 如果要计算number,可以单独写个函数
            break;
        case charCode:
            tokenId = CHARCON;
            break;
        default:
            break;
        }
        
        // 一个token的输入 ==> 如何就地进行语法分析 ==> 巨大的状态表和分析表
        currToken.line = currLineNumber;
        currToken.col = currCol;
        currToken.type = tokenId;
        currToken.valueStr = copyStr; // 需要都转为小写
        
        // FIXME: 重构
        Token t = currToken;
        tokens.push_back(t);
        ++this->tot;
    }

};