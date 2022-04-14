#pragma once
#include "info.hpp"
#include "LexicalAnalyzer.hpp"
#include "GrammarAnalyzer.hpp"
#include "ErrorHandler.hpp"
using namespace std;

class TreeNode{
public:
    bool isleaf=false; // choose (isleaf ? token : stateId);
    StateID stateId; // 表示性质 
    Token token;
    vector<TreeNode*> children;
    
    TreeNode(){}
    TreeNode(Token t,bool isleaf=true):isleaf(isleaf),token(t){}
    TreeNode(StateID stateId,bool isleaf=false):isleaf(isleaf),stateId(stateId){}
};

class GrammarAnalyzer{
private:
    // 构建语法分析树，递归的时候分析即可 ==> 之后再考虑吧
    LexicalAnalyzer lexer;
    TreeNode* root = NULL;
    Token currToken;
    ofstream ofp;

public:
    GrammarAnalyzer(string inFile, string outFile){
        this->lexer.init(inFile);
        this->lexer.doLexer();
        this->ofp.open(outFile);
        // this->lexer.printAllTokens();
    }

    ~GrammarAnalyzer(){
        deleteRoot(root);
        this->ofp.close();
    }
    
    /* 语法分析，建立语法树的过程 */    
    void doParser(){
        // 输入程序
        if(root==NULL){
            root = new TreeNode(PROGRAM);
        }
        parser_program(root);
    }

    void print_res(){
        __print_res(root);
    }

private:

    void parser_program(TreeNode* root){
        currToken = lexer.nextToken();
        // 常量声明
        if (!currMatch(CONSTTK)) return;
        if(currToken.type==CONSTTK){
            TreeNode* node = new TreeNode(DECLARE_CONST);
            parser_const_declare(node);
            root->children.push_back(node);
        }
        // 变量说明
    }

    void parser_const_declare(TreeNode* root){ // root==>常量说明
        // 判断
        if (!currMatch(CONSTTK)) return;        
        root->children.push_back(new TreeNode(currToken));
        
        // 常量定义
        TreeNode* node = new TreeNode(DEFINE_CONST);
        root->children.push_back(node);
        
        currToken = lexer.nextToken();
        if(!currMatch(INTTK) && !currMatch(CHARTK)) { return; }
        node->children.push_back(new TreeNode(currToken));
        parser_const_define(node, currToken.type); 
        
        // 当前已经处理到了分号
        if(!currMatch(SEMICN)){return;}
        root->children.push_back(new TreeNode(currToken));
        
        // 判断下一个是不是const
        currToken = lexer.nextToken();
        if(currMatch(CONSTTK,0)) parser_const_declare(root);
    }

    void parser_const_define(TreeNode* root, TokenID defineType){ // root ==> 常量定义
        // 标识符
        currToken = lexer.nextToken();
        if(!currMatch(IDENFR)) { return; }
        root->children.push_back(new TreeNode(currToken));

        // =
        currToken = lexer.nextToken();
        if(!currMatch(ASSIGN)) { return; }
        root->children.push_back(new TreeNode(currToken));

        if(defineType==INTTK){ // 数字
            parse_int(root);
        }else{ // 字符
            parse_char(root);
        }

        // 逗号和分号的区别
        currToken = lexer.nextToken();
        if(currMatch(COMMA,0)){
            root->children.push_back(new TreeNode(currToken));
            parser_const_define(root,defineType); // 继续常量定义
        } 
    }

    void parse_int(TreeNode* root){
        TreeNode* node1 = new TreeNode(INT);
        root->children.push_back(node1);
        TreeNode* node2 = new TreeNode(UNSIGNED_INT);
        node1->children.push_back(node2);
        // +，-
        currToken = lexer.nextToken();
        if(currMatch(PLUS,0) || currMatch(MINU,0)){
            node2->children.push_back(new TreeNode(currToken));
            currToken = lexer.nextToken();
        }
        if(!currMatch(INTCON)){return;}
        node2->children.push_back(new TreeNode(currToken));
        return;
    }

    void parse_char(TreeNode* root){
        // char
        currToken = lexer.nextToken();
        if(!currMatch(CHARCON)) { return; }
        root->children.push_back(new TreeNode(currToken));
    }

    bool currMatch(TokenID tId, int log=1){
        if(log){
            if(currToken.type!=tId){
                // TODO: error
                cout << "预期出现<" << tokenId_str[tId] << ">, 实际出现:" << currToken;
            }
        }
        return (currToken.type==tId);
    }

    void __print_res(TreeNode* root){
        if(root==NULL) return;
        for(auto tn:root->children){
            __print_res(tn);
        }
        if(root->isleaf){
            this->ofp << tokenId_str[root->token.type] << " " << root->token.valueStr << endl;
        }else{
            this->ofp << "<" << stateId_str[root->stateId] << ">" << endl;
        }
    }

    void deleteRoot(TreeNode* root){
        if(root==NULL) return;
        for(auto tn:root->children){
            deleteRoot(tn);
        }
        delete root;
    }

};