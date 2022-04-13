#pragma once
#include<bits/stdc++.h>
using namespace std;

struct Token{
    string type;
    string valueStr;
    int lineNumber;
};

class Lexer{
private:
    vector<Token> tokens;
    int curr,tot;
public:
    Lexer(){
        this->curr = 0;
        this->tot = 0;
    }
    
    void add_token(Token t){
        this->tokens.push_back(t);
        ++this->tot;
    }

    Token& next_token(){
        return this->tokens[this->curr++];
    }

    bool roll_back(){
        this->curr--;
        if(curr<0) {
            cout<<"Lexer无法再回退"<<endl;
            this->curr++;
            return false;
        }
        return true;
    }

}; 

// 非终结符，存在值
const string IDENFR = "IDENFR"; // 名字, string
const string INTCON = "INTCON"; // value, int
const string CHARCON = "CHARCON"; // value, char
const string STRCON = "STRCON"; // value, string

int currLineNumber = 1;
int errorCnt;
stack< pair<char,int> > leftBrack_LN; // 匹配号，行号

const int specialStrCode = 0;
const int commonStrCode = 1;
const int numberCode = 2;
const int charCode = 3;

const string illegalInt = "非法int";
const string illegalOp = "非法操作数";
const string illegalComma = "非法引号";

map<char, string> mismatchError = {
    {'(',"不匹配("},
    {')',"不匹配)"},
    {'[',"不匹配["},
    {']',"不匹配]"},
    {'{',"不匹配{"},
    {'}',"不匹配}"},
    {'\'',"不匹配单引号"},
    {'\"',"不匹配双引号"},
};

// 终结符
map<string, string> specialCateCodeMap = {
    // 关键字
    {"else", "ELSETK"},
    {"switch", "SWITCHTK"},
    {"case", "CASETK"},
    {"default", "DEFAULTTK"},
    {"const", "CONSTTK"},
    {"while", "WHILETK"},
    {"int", "INTTK"},
    {"for", "FORTK"},
    {"char", "CHARTK"},
    {"scanf", "SCANFTK"},
    {"void", "VOIDTK"},
    {"printf", "PRINTFTK"},
    {"main", "MAINTK"},
    {"return", "RETURNTK"},
    {"if", "IFTK"},
    // 赋值
    {"=", "ASSIGN"},
    // 数字运算
    {"+", "PLUS"},
    {"-", "MINU"},
    {"*", "MULT"},
    {"/", "DIV"},
    // 比较运算
    {"==", "EQL"},
    {"!=", "NEQ"},
    {"<", "LSS"},
    {"<=", "LEQ"},
    {">", "GRE"},
    {">=", "GEQ"},
    // 标点符号
    {";", "SEMICN"},
    {",", "COMMA"},
    {"(", "LPARENT"},
    {")", "RPARENT"},
    {"[", "LBRACK"},
    {"]", "RBRACK"},
    {"{", "LBRACE"},
    {"}", "RBRACE"},
    {":", "COLON"},
};


void error(int errorLineNumber, string errorCode){
    cout << "[Line " << errorLineNumber << "] 出现" << errorCode << "错误\n";
    ++errorCnt;
}