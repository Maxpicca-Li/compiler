#pragma once
#include<string>
#include<iostream>
#include<map>
#include"info.hpp"

using std::string;
using std::map;
int errorCnt = 0;

enum ErrorID{
    illegalLexcial='a', redefinition, undefined, funcParamsNumber, funcParamsType, 
    illegalCondition, funcNoReturn, funcWithReturn, arrayDimError, constChange, 
    shouldSEMICN, shouldRsmall, shouldRmid, arrayCntError, constType, lackDeafult
};

map<char, string> errorId_str = {
    {illegalLexcial, "非法符号或不符合词法"},
    {redefinition, "名字重定义"},
    {undefined, "未定义的名字"},
    {funcParamsNumber, "函数参数个数不匹配"},
    {funcParamsType, "函数参数类型不匹配"},
    {illegalCondition, "条件判断中出现不合法的类型"},
    {funcNoReturn, "无返回值的函数存在不匹配的return语句"},
    {funcWithReturn, "有返回值的函数缺少return语句或存在不匹配的return语句"},
    {arrayDimError, "数组元素的下标只能是整型表达式"},
    {constChange, "不能改变常量的值"},
    {shouldSEMICN, "应为分号"},
    {shouldRsmall, "应为右小括号')'"},
    {shouldRmid, "应为右中括号']'"},
    {arrayCntError, "数组初始化个数不匹配"},
    {constType, "<常量>类型不一致"},
    {lackDeafult, "缺少缺省语句"},
    {'(',"不匹配("},
    {')',"不匹配)"},
    {'[',"不匹配["},
    {']',"不匹配]"},
    {'{',"不匹配{"},
    {'}',"不匹配}"},
    {'\'',"不匹配单引号"},
    {'\"',"不匹配双引号"},
};

void error(int errorLineNumber, int errorCol, ErrorID errorId){
    std::cout << "line:" << errorLineNumber << ", col:" << errorCol << ", error:" << errorId_str[errorId] << '\n';
    ++errorCnt;
}

void error(Token t, ErrorID errorId){
    // std::cout << "line:" << t.line << ", col:" << t.col << ", error:" << errorId_str[errorId] << '\n';
    std::cout << "ERROR: " << errorId_str[errorId] << ", line:" << t.line << ", col:" << t.col  << ", token:" << t.type << '\n';
    ++errorCnt;
}