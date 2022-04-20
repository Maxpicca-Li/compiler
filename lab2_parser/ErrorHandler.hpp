#pragma once
#include<string>
#include<iostream>
#include<map>
#include"info.hpp"

using std::string;
using std::map;
using std::cout;

enum ErrorID{
    illegalLexcial='a', redefined, undefined, funcParamsNumber, funcParamsType, 
    illegalCondition, funcNoReturn, funcWithReturn, arrayIdxError, constChange, 
    shouldSEMICN, shouldRsmall, shouldRmid, arrayCntError, constType, lackDeafult
};

map<char, string> errorId_str = {
    {illegalLexcial, "非法符号或不符合词法"},
    {redefined, "名字重定义"},
    {undefined, "未定义的名字"},
    {funcParamsNumber, "函数参数个数不匹配"},
    {funcParamsType, "函数参数类型不匹配"},
    {illegalCondition, "条件判断中出现不合法的类型"},
    {funcNoReturn, "无返回值的函数存在不匹配的return语句"},
    {funcWithReturn, "有返回值的函数缺少return语句或存在不匹配的return语句"},
    {arrayIdxError, "数组元素的下标只能是整型表达式"},
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

class ErrorHandler{
public:
    int errorCnt = 0;
    vector< pair<int,ErrorID> > errorList;
    
    void init(){
        errorCnt = 0;
        errorList.clear();
    }

    void printError(string outFile){
        ofstream ofp;
        ofp.open(outFile);
        sort(errorList.begin(),errorList.end());
        for(auto e:errorList){
            ofp << e.first << " " << char(e.second) << endl;
        }
        ofp.close();
    }

    void errorUnkown(int errorLineNumber, int errorCol, string msg){
        cout << "UnkownError: " << msg << ", line:" << errorLineNumber << ", col:" << errorCol << '\n'; 
    }

    void errorUnkown(Token t, string msg){
        cout << "UnkownError: " << msg << ", line:" << t.line << ", col:" << t.col  << ", token:" << tokenId_str[t.type] << ", value:" << t.valueStr << '\n';
    }

    void error(int errorLineNumber, int errorCol, ErrorID errorId){
        cout << "Error: " << errorId_str[errorId] << ", line:" << errorLineNumber << ", col:" << errorCol << '\n';
        errorList.push_back({errorLineNumber, errorId});
        ++errorCnt;
    }

    void error(Token t, ErrorID errorId){
        cout << "Error: " << errorId_str[errorId] << ", line:" << t.line << ", col:" << t.col  << ", token:" << tokenId_str[t.type] << ", value:" << t.valueStr << '\n';
        errorList.push_back({t.line, errorId});
        ++errorCnt;
    }
} ehandler;
