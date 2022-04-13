#pragma once
#include<bits/stdc++.h>
using namespace std;

int currLineNumber = 1, currCol = 1;
stack< pair<char,int> > leftBrack_LN; // 匹配号，行号

enum tokenID{
    // value
    IDENFR = 256, INTCON, CHARCON, STRCON,
    // 关键字
    ELSETK,SWITCHTK,CASETK,DEFAULTTK,CONSTTK,WHILETK,INTTK,FORTK,CHARTK,SCANFTK,VOIDTK,PRINTFTK,MAINTK,RETURNTK,IFTK,
    // 计算符
    ASSIGN,PLUS,MINU,MULT,DIV,
    // 比较符
    EQL,NEQ,LSS,LEQ,GRE,GEQ,
    // 标点符号
    SEMICN,COMMA,LPARENT,RPARENT,LBRACK,RBRACK,LBRACE,RBRACE,COLON
};

// 非终结符，存在值
const string IDENFR = "IDENFR"; // 名字, string
const string INTCON = "INTCON"; // value, int
const string CHARCON = "CHARCON"; // value, char
const string STRCON = "STRCON"; // value, string

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
