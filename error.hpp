#pragma once
#include<string>
#include<iostream>
#include<map>

int errorCnt = 0;

const int specialStrCode = 0;
const int commonStrCode = 1;
const int numberCode = 2;
const int charCode = 3;

const std::string illegalInt = "非法int";
const std::string illegalOp = "非法操作数";
const std::string illegalComma = "非法引号";

std::map<char, std::string> mismatchError = {
    {'(',"不匹配("},
    {')',"不匹配)"},
    {'[',"不匹配["},
    {']',"不匹配]"},
    {'{',"不匹配{"},
    {'}',"不匹配}"},
    {'\'',"不匹配单引号"},
    {'\"',"不匹配双引号"},
};

void error(int errorLineNumber, std::string errorCode){
    std::cout << "[Line " << errorLineNumber << "] 出现" << errorCode << "错误\n";
    ++errorCnt;
}