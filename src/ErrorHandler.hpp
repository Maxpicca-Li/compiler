#pragma once
#include<string>
#include<iostream>
#include<map>

using std::string;
using std::map;
int errorCnt = 0;

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

void error(int errorLineNumber, string errorCode){
    std::cout << "[Line " << errorLineNumber << "] 出现" << errorCode << "错误\n";
    ++errorCnt;
}