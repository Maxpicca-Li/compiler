// 代码生成 ==> 中间代码 四元组 ==> 目标代码 可执行MIPS
// 这次代码写的真的烂！！！
#pragma once
#include<bits/stdc++.h>
#include "GrammarAnalyzer.hpp"
#include "info.hpp"
#include "SystemTableManager.hpp"

#define PUTINT  quads.emplace_back("li","1","","$v0"); quads.emplace_back("syscall");
#define PUTSTR  quads.emplace_back("li","4","","$v0"); quads.emplace_back("syscall");
#define PUTCHAR quads.emplace_back("li","11","","$v0"); quads.emplace_back("syscall");
#define PUTCRLF quads.emplace_back("li","10","","$a0"); quads.emplace_back("li","11","","$v0");quads.emplace_back("syscall"); // 换行符
#define GETINT  quads.emplace_back("li","5","","$v0"); quads.emplace_back("syscall");
#define GETSTR  quads.emplace_back("li","8","","$v0"); quads.emplace_back("syscall");
#define GETCHAR quads.emplace_back("li","12","","$v0"); quads.emplace_back("syscall");
#define REGT "$t"+to_string(reg_t)
#define REGT1 "$t"+to_string(reg_t+1)
#define REGT2 "$t"+to_string(reg_t+2)
#define DCHAR ".byte"
#define DINT ".word"
#define DSTR ".asciiz"
using namespace std;

struct Data{
    string name, type, value; // type = [.asciiz, .word, .byte] 不考虑数组.space
    
    Data(string name, string type, string value):name(name),type(type),value(value){}
    
    friend ostream& operator << (ostream& out,const Data& d){
        if (d.type==".asciiz"){
            out << d.name << ": " << d.type << " \"" << d.value << '\"';
        }else if(d.type==".word"){
            out << d.name << ": " << d.type << " " << d.value;
        }else if(d.type==".byte"){
            out << d.name << ": " << d.type << " \'" << d.value << '\'';
        }else {cout << d.type; assert(false);}
        return out;
    }
};

struct Quad{
    string op,arg1,arg2,result;

    Quad(string op, string arg1="", string arg2="", string result=""):op(op),arg1(arg1),arg2(arg2),result(result){}

    string to_mips(){
        if (op == "label") return result + ":";
        else if (op=="li" || op=="lw" || op=="la") return op +" "+ result + "," + arg1;
        else if (op=="sw") return op+" "+arg1+","+result;
        else if (op=="move") return op + " " + result + "," + arg1;
        else if (op=="syscall") return op;
        else if (op=="mult" || op=="div") return op + " " + arg1 + "," + arg2;
        else if (op=="mflo" || op=="mfhi" || op=="mtlo" || op=="mthi") return op + " " + arg1;
        else return op + " " + result + "," + arg1 + "," + arg2; // all alu instr
    }

};

map<StateID,StateID> attention_curr2next={
    {PROGRAM,DEFINE_FUNC_MAIN},
    {DEFINE_FUNC_MAIN,SENTENCE_COMPOUND},
    {SENTENCE_COMPOUND,SENTENCE_MULTI},
    {SENTENCE_MULTI,SENTENCE},
};

class TargetCodeGenerator{
private:
    vector<Data> datas;
    vector<Quad> quads;
    ofstream ofp;
    int strcount = 0;

public:
    GrammarAnalyzer parser;
    TargetCodeGenerator(){}

    void doGenerate(string inFile){
        this->parser.init(inFile);
        this->parser.doParser();
        // data段
        // 遍历全局变量
        for(auto p:this->parser.varStaticTable){
            Variable& var = p.second;
            _addData(var);
        }
        // TODO:函数内部声明的变量，应该在堆，还是栈，还是data区域？
        for(auto f:this->parser.funcTable){
            for(auto p:f.second.varTable){
                Variable& var = p.second;
                _addData(var);
            }
        }
        // text段
        // 自顶向下获取text
        _generate(this->parser.root, 0);
    }

    void printMIPS(string outFile){
        this->ofp.open(outFile);
        // 打印数据
        _printData();
        _printText();
        this->ofp.close();
    }


private:
    void _addData(Variable& var){
        string type,value;
        if (var.varType==VARINT){
            type= ".word";
            int t;
            memcpy(&t, var.valueP, var.size);
            value = int2str(t);
        }else if(var.varType==VARCHAR){
            type= ".byte";
            char t;
            memcpy(&t, var.valueP, var.size);
            value = string(&t);
        }
        datas.emplace_back(var.name,type,value);
    }

    void _generate(TreeNode* root, int reg_t=0){
        if(root==NULL) return;
        if(!root->isleaf && root->stateId==SENTENCE_READ){ // 读语句
            int i=0;
            for(auto node:root->children){  // 获取标识符
                if(node->token.type==IDENFR) break;
                i++;
            }
            Token& tk = root->children[i]->token; // scanf(标识符)
            string name = _getLower(tk.valueStr);
            VartypeID varType = _getVarType(tk.valueStr);
            if(varType==VARINT){
                GETINT;
            }else if(varType==VARCHAR){
                GETCHAR;
            }else {
                cout << varType;
                assert(false);
            }
            // 将数据地址放到t0
            quads.emplace_back("la",name,"",REGT);  // 取址
            quads.emplace_back("sw","$v0","","0($t"+to_string(reg_t)+")"); // 寄存器地址赋值
        }else if(!root->isleaf && root->stateId==SENTENCE_WRITE){ // 写语句
            for(auto node:root->children){
                if(node->stateId == STRING){
                    _generate(node,reg_t); // 记录变量+传址传参
                    PUTSTR;
                }else if(node->stateId==EXPRESSION){
                    // 获取表达式的值，放到reg_t
                    _generate(node,reg_t);
                    quads.emplace_back("move",REGT,"","$a0"); // 传值传参
                    if(node->varType==VARINT) {PUTINT;}
                    else if(node->varType==VARCHAR) {PUTCHAR;}
                    else {cout << node->varType; assert(false);}
                }
            }
            PUTCRLF;
        }else if(!root->isleaf && root->stateId==SENTENCE_ASSIGN){ // 赋值语句
            string varName;
            for(auto node:root->children){
                Token& tk = node->token;
                if(node->isleaf && tk.type==IDENFR){ // 获取标识符
                    varName = _getLower(tk.valueStr);
                }else if(!node->isleaf && node->stateId==EXPRESSION){
                    // 获取表达式的值，放到reg_t+1
                    _generate(node,reg_t+1);
                }
            }
            quads.emplace_back("la",varName,"",REGT); // 取址
            quads.emplace_back("sw",REGT1,"","0($t"+to_string(reg_t)+")");  // 寄存器地址赋值
        }else if(!root->isleaf && root->stateId==EXPRESSION){ // 表达式
            // TODO: 表达式的构造这里，才意识到应该要建立抽象语义树ast（二叉树）的，后续遍历好像就可以了
            // TODO: emmm，语义分析，中间代码，目标代码都没学好。。。
            // 比较拉跨的做法
            if(root->children.size()==1){
                _generate(root->children[0],reg_t);
                return;
            }
            quads.emplace_back("move","$0","",REGT);  // reg_t 置零
            string op="add";
            for(auto node:root->children){
                if(node->isleaf){
                    if(node->token.valueStr=="+"){
                        op = "add";
                    }else if(node->token.valueStr=="-"){
                        op = "sub";
                    }
                }
                if(!node->isleaf){
                    // 把值放在reg_t+1
                    _generate(node,reg_t+1);
                    quads.emplace_back(op,REGT, REGT1,REGT);
                }
            }
        }else if(root->stateId==TERM){
            if(root->children.size()==1){
                _generate(root->children[0],reg_t);
                return;
            }
            // 剩下，怎么视作 3项
            // 结果 reg_t, 第一项 reg_t+1
            // 获取第一个因子
            int i=0;
            TreeNode* node = root->children[i];
            if(node->stateId==FACTOR){
                _generate(node,reg_t+1);
            }
            i++;
            string op="mult";
            for(;i<int(root->children.size());i++){
                node = root->children[i];
                if(node->isleaf){
                    if(node->token.valueStr=="*"){
                        op = "mult";
                    }else if(node->token.valueStr=="/"){
                        op = "div";
                    }
                }
                if(!node->isleaf){
                    // 把值放在reg_t+2  第二操作数
                    _generate(node,reg_t+2);
                    quads.emplace_back(op,REGT1, REGT2,"");
                    quads.emplace_back("mflo",REGT1,"","");
                }
            }
            // reg_t
            quads.emplace_back("move",REGT1,"",REGT);
        }else if(root->token.type==IDENFR){
            // 可能是函数，也可能是变量
            if(root->token.flag==VARFLAG){
                string name = _getLower(root->token.valueStr);
                // token_name
                quads.emplace_back("la",name,"",REGT1);  // 取址
                quads.emplace_back("lw","0($t"+to_string(reg_t+1)+")","",REGT); // 寄存器地址赋值
            }
        }else if(root->token.type==INTCON || root->token.type==CHARCON){
            quads.emplace_back("addiu","$0",root->token.valueStr,REGT);
        }else if(root->token.type==STRCON){
            string name = "str"+to_string(strcount);
            strcount++;
            datas.emplace_back(name, DSTR,root->token.valueStr); // 记录变量
            quads.emplace_back("la",name,"","$a0");  // 传址传参
        }else if(root->stateId==INT){
            string op="";
            for(auto node:root->children){
                if(node->token.valueStr=="-") op="sub";
                else if(node->stateId==UNSIGNED_INT) _generate(node,reg_t);
            }
            if(op=="sub"){
                quads.emplace_back(op,"$0",REGT,REGT);
            }
        }else if(attention_curr2next.find(root->stateId)!=attention_curr2next.end()){
            for(auto node:root->children){
                if(node->stateId==attention_curr2next[root->stateId]){ // 如果是关注的数据
                    _generate(node,reg_t);
                }
            }
        }else if(root->stateId==SENTENCE || root->stateId==FACTOR || root->stateId==UNSIGNED_INT || root->stateId==STRING){
            // 如果是无符号整数，卧槽，他居然有个负号
            for(auto node:root->children){
                _generate(node,reg_t);
            }
        }
    }

    void _printData(){
        this->ofp << ".data" << endl;
        for(auto data:this->datas){
            this->ofp << data << endl;
        }
    }

    void _printText(){
        this->ofp << ".text" << endl;
        for(auto quad:this->quads){
            this->ofp << quad.to_mips() << endl;
        }
    }

    string _getLower(string name){
        string _name = name;
        for(char& ch:_name) ch = tolower(ch);
        return _name;
    }

    VartypeID _getVarType(string name){
        // 这里由先后顺序
        for(char& ch:name) ch = tolower(ch);
        for (auto f:this->parser.funcTable){
            if(f.second.varTable.find(name)!=f.second.varTable.end()){
                return f.second.varTable[name].varType;
            }
        }
        for(auto p:this->parser.varStaticTable){
            if(this->parser.varStaticTable.find(name)!=this->parser.varStaticTable.end()){
                return this->parser.varStaticTable[name].varType;
            }
        }
        return VARVOID;
    }
};

