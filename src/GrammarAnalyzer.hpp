#pragma once
#include "info.hpp"
#include "util.hpp"
#include "LexicalAnalyzer.hpp"
#include "GrammarAnalyzer.hpp"
#include "ErrorHandler.hpp"
using namespace std;

/* 程序说明
由于"历史bug原因"，做如下程序说明：
形如 parase_***() 会探到下一个token
形如 __parase_***() 不会探到下一个token
 */

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
/* 有关宏定义，懒懒懒 */
#define NEXTTOKEN currToken = lexer.nextToken()
#define NEWLEAF new TreeNode(currToken)

private:
    // 构建语法分析树，递归的时候分析即可 ==> 之后再考虑吧
    LexicalAnalyzer lexer;
    TreeNode* root = NULL;
    Token currToken;
    ofstream ofp;
    // 记录数组范围
    int arrayA = 0, arrayB = 0;

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
        NEXTTOKEN;
        while(!lexer.isEnd()){
            if(currMatch(CONSTTK,0)){
                // 常量声明
                TreeNode* node1 = new TreeNode(DECLARE_CONST);
                root->children.push_back(node1);
                parser_const_declare(node1);  // return时, 已经读取到了下一个
            }else if(currMatch(INTTK,0) || currMatch(CHARTK,0)) {
                // 变量声明|声明头部
                parser_var_declare(root);   
            }else{
                NEXTTOKEN;
            }
        }
    }

    /* 变量说明|声明头部: 先建立tmp, 判断过程中先挂载在tmp节点上 */
    void parser_var_declare(TreeNode* root){ // root=程序
        // 可能的变量声明
        TreeNode* node1 = new TreeNode(DECLARE_VAR); 
        bool flag = true; // node1的children是否为空
        while(true){
            TreeNode* tmp = new TreeNode(); // 终结符的父亲节点
            // defineType
            TokenID defineType = currToken.type;
            tmp->children.push_back(NEWLEAF);
            // 标识符
            NEXTTOKEN;
            if(!currMatch(IDENFR)) { return; }
            tmp->children.push_back(NEWLEAF);
            
            // ! ! ! 重要判断流程判断 ! ! !
            NEXTTOKEN;
            // root ==> 头部声明tmp ==> 解析参数列表tmp.childern
            if(currMatch(LPARENT,0)){
                if(flag){ delete node1; } // 如果中间节点node1的子结点为空
                tmp->stateId = DECLARE_HEADER;
                root->children.push_back(tmp);
                // TODO: 跳转到解析参数列表
                return;
            }
            // root ==> 变量声明 ==> 变量定义 ==> 有无初始化tmp ==> tmp.children
            if(flag){ 
                root->children.push_back(node1);
                flag = false;
            }
            TreeNode* node2 = new TreeNode(DEFINE_VAR); // 变量定义
            node1->children.push_back(node2);
            node2->children.push_back(tmp);
            parser_var_define(tmp,defineType); // {标识符}+{=值} 的判断, '{}'标识可选
            
            // 分号
            if(currMatch(SEMICN,0)){
                node1->children.push_back(NEWLEAF); // 变量说明中的分号
                // defineType
                NEXTTOKEN;
                if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { return; } // 结束
            } 
        }
        return ;
    }

    void parser_var_define(TreeNode* root, TokenID defineType){ // root=变量定义有无初始化
        // defineSub: 
        // 类标识符解决了; 标识符第一次判断解决了, ','后的子句判断没解决
        if(currMatch(COMMA,0)){
            // 子句中的变量定义有无初始化, 需要解决标识符问题
            root->children.push_back(NEWLEAF);
            NEXTTOKEN;
            if(!currMatch(IDENFR)) { return; }
            root->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }
        if(currMatch(LBRACK,0)){  
            // 数组
            arrayA = 0;arrayB = 0;
            parse_brack(root); // 解析维度
            if(currMatch(ASSIGN)) { 
                // 有初始化
                root->stateId = DEFINE_VAR_INIT;    
                root->children.push_back(NEWLEAF);
                NEXTTOKEN;
                parse_array_init(root,defineType); // 初始化解析
            }else{ 
                // 无初始化
                root->stateId = DEFINE_VAR_NO;
            }
        }else{
            // 普通变量
            if(currMatch(ASSIGN,0)){
                // 有初始化
                root->stateId = DEFINE_VAR_INIT;
                root->children.push_back(NEWLEAF); NEXTTOKEN;
                if(defineType==INTTK) parse_int(root);
                else if(defineType==CHARTK) parse_char(root);
            }else{
                // 无初始化
                root->stateId = DEFINE_VAR_NO;
            }
        }
        // 已经到下一个了
        if(currMatch(COMMA,0)){
            parser_var_define(root,defineType);
        }
    }

    void parser_const_declare(TreeNode* root){ // root==>常量说明
        // 判断
        if (!currMatch(CONSTTK)) return;        
        root->children.push_back(NEWLEAF);
        
        // 常量定义
        TreeNode* node = new TreeNode(DEFINE_CONST);
        root->children.push_back(node);

        NEXTTOKEN;
        if(!currMatch(INTTK) && !currMatch(CHARTK)) { return; }
        node->children.push_back(NEWLEAF);
        parser_const_define(node, currToken.type); 
        
        // 当前已经处理到了分号
        if(!currMatch(SEMICN)){return;}
        root->children.push_back(NEWLEAF);
        
        // 判断下一个是不是const
        NEXTTOKEN;
        if(currMatch(CONSTTK,0)) parser_const_declare(root);
    }

    void parser_const_define(TreeNode* root, TokenID defineType){ // root ==> 常量定义
        // 标识符
        NEXTTOKEN;
        if(!currMatch(IDENFR)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;

        // =
        if(!currMatch(ASSIGN)) { return; }
        root->children.push_back(NEWLEAF); 
        
        NEXTTOKEN;
        if(defineType==INTTK){ // 数字
            parse_int(root);
        }else{ // 字符
            parse_char(root);
        }

        // 逗号和分号的区别
        if(currMatch(COMMA,0)){
            root->children.push_back(NEWLEAF);
            parser_const_define(root,defineType); // 继续常量定义
        } 
    }

    int parse_int(TreeNode* root, bool unsign=false){
        TreeNode* node1 = new TreeNode(INT);
        root->children.push_back(node1);
        TreeNode* node2 = new TreeNode(UNSIGNED_INT);
        node1->children.push_back(node2);
        // +，-
        bool minus = false;
        if(currMatch(PLUS,0)){
            node2->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }else if(currMatch(MINU,0)){
            minus = true;
            // 判断数字是否为正
            if(unsign) {error(currToken.line, currToken.col, arrayDimError);}
            node2->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }
        // 数字
        if(!currMatch(INTCON)){return -1;}
        node2->children.push_back(NEWLEAF); NEXTTOKEN;
        // 结果
        int res = str2int(currToken.valueStr);
        return minus ? -res : res;
    }

    void parse_char(TreeNode* root){
        // char
        if(!currMatch(CHARCON)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
    }

    void parse_brack(TreeNode* root){ // 叶结点的上方
        // [
        if(!currMatch(LBRACK)) { return; }
        root->children.push_back(NEWLEAF);
        // 无符号整数
        NEXTTOKEN;
        arrayA = parse_int(root, true);
        // ]
        NEXTTOKEN;
        if(!currMatch(RBRACK)) { return; }
        root->children.push_back(NEWLEAF);
        NEXTTOKEN;

        if(currMatch(LBRACK,0)) {
            // [
            root->children.push_back(NEWLEAF);
            // 无符号整数
            NEXTTOKEN;
            arrayB = parse_int(root, true);
            // ]
            NEXTTOKEN;
            if(!currMatch(RBRACK)) { return; }
            root->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }
    }

    /* FIXME: parse_array_init */
    void parse_array_init(TreeNode* root, TokenID defineType){
        if(arrayB==0){ // 只有一维度
            __array_init(root, arrayA, defineType);
            NEXTTOKEN;
        }else{
            // 开头的}
            if(!currMatch(LBRACE)) {return;}
            root->children.push_back(NEWLEAF); 
            NEXTTOKEN;
            __array_init(root, arrayA, defineType);
            // 中间的，
            NEXTTOKEN;
            if(!currMatch(COMMA)) {return;}
            root->children.push_back(NEWLEAF); 
            NEXTTOKEN;
            __array_init(root, arrayB, defineType);
            // 结尾的}
            if(!currMatch(RBRACE)) {return;}
            root->children.push_back(NEWLEAF); 
            NEXTTOKEN;
        }
    }
    void __array_init(TreeNode* root, int d, TokenID defineType){
        if(!currMatch(LBRACE)) {return;}
        root->children.push_back(NEWLEAF); 
        NEXTTOKEN;
        for(int i=0;i<d;i++){
            if(i && !currMatch(COMMA)){
                return;
            }else{
                root->children.push_back(NEWLEAF);
                NEXTTOKEN;
            }
            if(defineType==INTTK) parse_int(root); 
            else parse_char(root); 
        }
        if(!currMatch(RBRACE)) {
            error(currToken.line, currToken.col, arrayCntError);
            return; // TODO: 如何跳过知道下一个可以
        }
        root->children.push_back(NEWLEAF); 
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