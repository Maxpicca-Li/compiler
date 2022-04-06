#pragma once
#include<bits/stdc++.h>
using namespace std;

const string IDENFR = "IDENFR";
const string INTCON = "INTCON";
const string CHARCON = "CHARCON";
const string STRCON = "STRCON";

int currLineNumber = 1;
int errorCnt;
stack<char> leftBrack;

const int specialStrCode = 0;
const int commonStrCode = 1;
const int numberCode = 2;
const int charCode = 3;

const string illegalInt = "非法int";
const string illegalOp = "非法操作数";
const string illegalComma = "非法引号";
const string mismatchLittle = "不匹配(";
const string mismatchBig = "不匹配{";
const string mismatchMiddle = "不匹配[";

map<char, string> mismatchError = {
    {'(',"不匹配("},
    {'[',"不匹配]"},
    {'{',"不匹配{"},
    {'\'',"不匹配单引号"},
    {'\"',"不匹配双引号"},
};

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