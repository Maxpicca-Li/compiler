#pragma once
#include <string>
#include "info.hpp"
using namespace std;

// 变量类型
enum VartypeID{
    VARVOID, 
    VARINT,VARINT1D,VARINT2D, 
    VARCHAR,VARCHAR1D,VARCHAR2D,
    VARPOINTER,
};

map<VartypeID, string> vartypeId_str = {
    {VARVOID, "VARVOID" },
    {VARINT, "VARINT"},
    {VARINT1D, "VARINT1D"},
    {VARINT2D, "VARINT2D" },
    {VARCHAR, "VARCHAR"},
    {VARCHAR1D, "VARCHAR1D"},
    {VARCHAR2D, "VARCHAR2D"},
    {VARPOINTER, "VARPOINTER"},
};

// 变量
struct Variable{
    string name="";
    VartypeID varType=VARVOID;
    bool isConst=false;
    int size=0;
    int sizeA=0, sizeB=0; // 针对数组备用的两个变量
    char* valueP=NULL; // int 4字节，char 1字节 
    
    Variable(){}
    Variable(const Variable& x){
        /* 拷贝构造 */
        this->name = x.name;
        this->varType = x.varType;
        this->isConst = x.isConst;
        this->size = x.size;
        if(this->valueP!=NULL){
            delete [] valueP;
        }
        if(x.valueP==NULL) this->valueP = NULL;
        else {
            this->valueP = new char[this->size];
            memcpy(this->valueP, x.valueP, this->size);
        }
    }
    
    Variable& operator=(const Variable& x){
        /* 重载赋值 */
        this->name = x.name;
        this->varType = x.varType;
        this->isConst = x.isConst;
        this->size = x.size;
        if(this->valueP!=NULL){
            delete [] valueP;
        }
        if(x.valueP==NULL) this->valueP = NULL;
        else {
            this->valueP = new char[this->size];
            memcpy(this->valueP, x.valueP, this->size);
        }
        return *this;
    }

    ~Variable(){
        /* 析构函数 */
        if(valueP!=NULL){
            delete [] valueP;
        }
    }
};
// 函数
struct Function{
    string name;
    vector<VartypeID> argsType;
    VartypeID returnType;
    map<string, Variable> varTable;
};
