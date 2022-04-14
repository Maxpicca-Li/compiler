#pragma once
#include<bits/stdc++.h>
using namespace std;

const int specialStrCode = 0;
const int strCode = 1;
const int intCode = 2;
const int charCode = 3;
const int illegalCode = 4;

enum TokenID{
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


enum StateID{
    // declare
    DECLARE_CONST, // 声明常量
    DECLARE_VAR, // 声明变量
    DECLARE_HEADER, // 声明头部

    // define
    DEFINE_CONST, // 常量定义
    DEFINE_VAR, // 变量定义
    DEFINE_VAR_NO, // 变量定义，无初始化
    DEFINE_VAR_INIT, // 变量定义，有初始化
    DEFINE_FUNC, // 函数定义，无返回
    DEFINE_FUNC_RETURN, // 函数定义，有返回
    DEFINE_FUNC_MAIN, // 主函数

    // argument
    LIST_ARGUMENT, // 参数表
    LIST_ARGUMENT_VALUE, // 传值传参

    // value
    INT, // 整数
    UNSIGNED_INT, // 无符号整数
    STRING, // 字符串
    
    // expression
    EXPRESSION, // 表达式
    TERM, // 项
    FACTOR, // 因子

    // sentence 11种
    SENTENCE_LOOP, STEP, // 步长
    SENTENCE_IF, CONDITION, // 条件
    SENTENCE_CALLFUNC_RETURN, 
    SENTENCE_CALLFUNC, 
    SENTENCE_ASSIGN, 
    SENTENCE_READ, 
    SENTENCE_WRITE, 
    SENTENCE_SWITCH, SUB_SENTENCE_SWITCH, SUB_SENTENCE_DEFAULT,LIST_CASE, // 情况表
    SENTENCE_NULL,
    SENTENCE_RETURN,
    SENTENCE_MULTI_BLOCK, 
    
    // 组合
    SENTENCE_COMPOUND,  // 复合语句
    SENTENCE_MULTI, // 语句列
    PROGRAM, // 程序
};

map<TokenID, string> tokenId_str = {
    {IDENFR, "IDENFR"},
    {INTCON, "INTCON"},
    {CHARCON, "CHARCON"},
    {STRCON, "STRCON"},
    {ELSETK, "ELSETK"},
    {SWITCHTK, "SWITCHTK"},
    {CASETK, "CASETK"},
    {DEFAULTTK, "DEFAULTTK"},
    {CONSTTK, "CONSTTK"},
    {WHILETK, "WHILETK"},
    {INTTK, "INTTK"},
    {FORTK, "FORTK"},
    {CHARTK, "CHARTK"},
    {SCANFTK, "SCANFTK"},
    {VOIDTK, "VOIDTK"},
    {PRINTFTK, "PRINTFTK"},
    {MAINTK, "MAINTK"},
    {RETURNTK, "RETURNTK"},
    {IFTK, "IFTK"},
    {ASSIGN, "ASSIGN"},
    {PLUS, "PLUS"},
    {MINU, "MINU"},
    {MULT, "MULT"},
    {DIV, "DIV"},
    {EQL, "EQL"},
    {NEQ, "NEQ"},
    {LSS, "LSS"},
    {LEQ, "LEQ"},
    {GRE, "GRE"},
    {GEQ, "GEQ"},
    {SEMICN, "SEMICN"},
    {COMMA, "COMMA"},
    {LPARENT, "LPARENT"},
    {RPARENT, "RPARENT"},
    {LBRACK, "LBRACK"},
    {RBRACK, "RBRACK"},
    {LBRACE, "LBRACE"},
    {RBRACE, "RBRACE"},
    {COLON, "COLON"},
};

map<string, TokenID> specialCateCodeMap = {
    // 关键字
    {"else", ELSETK},
    {"switch", SWITCHTK},
    {"case", CASETK},
    {"default", DEFAULTTK},
    {"const", CONSTTK},
    {"while", WHILETK},
    {"int", INTTK},
    {"for", FORTK},
    {"char", CHARTK},
    {"scanf", SCANFTK},
    {"void", VOIDTK},
    {"printf", PRINTFTK},
    {"main", MAINTK},
    {"return", RETURNTK},
    {"if", IFTK},
    // 赋值
    {"=", ASSIGN},
    // 数字运算
    {"+", PLUS},
    {"-", MINU},
    {"*", MULT},
    {"/", DIV},
    // 比较运算
    {"==", EQL},
    {"!=", NEQ},
    {"<", LSS},
    {"<=", LEQ},
    {">", GRE},
    {">=", GEQ},
    // 标点符号
    {";", SEMICN},
    {",", COMMA},
    {"(", LPARENT},
    {")", RPARENT},
    {"[", LBRACK},
    {"]", RBRACK},
    {"{", LBRACE},
    {"}", RBRACE},
    {":", COLON},
};


map<StateID, string> stateId_str = {
    // declare
    {DECLARE_CONST,"常量说明"},
    {DECLARE_VAR,"变量说明"},
    {DECLARE_HEADER,"声明头部"},

    // define
    {DEFINE_CONST, "常量定义"},
    {DEFINE_VAR, "变量定义"},
    {DEFINE_VAR_NO, "变量定义无初始化"},
    {DEFINE_VAR_INIT, "变量定义及初始化"},
    {DEFINE_FUNC, "无返回值函数定义"},
    {DEFINE_FUNC_RETURN, "有返回值函数定义"},
    {DEFINE_FUNC_MAIN, "主函数"},

    // argument
    {LIST_ARGUMENT, "参数表"},
    {LIST_ARGUMENT_VALUE, "值参数表"},
    {LIST_CASE, "情况表"},

    // value
    {INT, "整数"},
    {UNSIGNED_INT, "无符号整数"},
    {STRING, "字符串"},
    
    // expression
    {EXPRESSION, "表达式"},
    {TERM, "项"},
    {FACTOR, "因子"},

    // sentence derive
    {SUB_SENTENCE_SWITCH, "情况子语句"},
    {SUB_SENTENCE_DEFAULT, "缺省"},
    {CONDITION, "条件"},
    {STEP, "步长"},

    // sentence 11种
    {SENTENCE_LOOP, "循环语句"},
    {SENTENCE_IF, "条件语句"}, 
    {SENTENCE_CALLFUNC_RETURN, "有返回值函数调用语句"},
    {SENTENCE_CALLFUNC, "无返回值函数调用语句"},
    {SENTENCE_ASSIGN, "赋值语句"},
    {SENTENCE_READ, "读语句"},
    {SENTENCE_WRITE, "写语句"},
    {SENTENCE_SWITCH, "情况语句"},
    {SENTENCE_NULL, "空"},
    {SENTENCE_RETURN, "返回语句"},
    {SENTENCE_MULTI, "语句列"},
    
    // 组合
    {SENTENCE_COMPOUND, "复合语句"},
    {PROGRAM, "程序"},
};

struct Token{
    TokenID type;
    string valueStr;
    int line, col;
    
    friend ostream& operator << (ostream& out,const Token& t){
        out << "line:" << t.line << ", col:" << t.col << ", valueStr:" << t.valueStr << ", token:" << tokenId_str[t.type] << endl;
        return out;
    }
};