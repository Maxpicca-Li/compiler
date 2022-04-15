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
            // root ==> 有返回值函数定义 ==> 头部声明tmp ==> 解析参数列表tmp.childern
            if(currMatch(LSMALL,0)){
                if(flag){ delete node1; } // 如果中间节点node1的子结点为空
                tmp->stateId = DECLARE_HEADER;
                TreeNode* node0 = new TreeNode(DEFINE_FUNC_RETURN); // 有|无返回值函数定义
                root->children.push_back(node0);
                node0->children.push_back(tmp);
                parser_arguments(node0); 
                // 里面比较复杂，就先匹配外面的大括号
                if(!currMatch(LBIG)) { return; } // error
                node0->children.push_back(NEWLEAF);NEXTTOKEN;
                TreeNode* sentences =  new TreeNode(SENTENCE_COMPOUND); // 复合语句
                parser_sentence_compound(sentences);
                if(!currMatch(RBIG)) { return; } // error
                node0->children.push_back(NEWLEAF);NEXTTOKEN;
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

    void parser_arguments(TreeNode* root){ // 有|无返回值函数定义
        if(!currMatch(LSMALL)) return;
        root->children.push_back(NEWLEAF);
        NEXTTOKEN;
        TreeNode* args = new TreeNode(LIST_ARGUMENT);
        root->children.push_back(args);
        while(!currMatch(RSMALL,0) && !currMatch(LBIG,0)){
            if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { 
                error(currToken.line, currToken.col, illegalLexcial);
            }else{
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }
            if(!currMatch(IDENFR)){
                error(currToken.line, currToken.col, illegalLexcial);
            }else{
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }
            // 判断是否是都好
            if(currMatch(COMMA,0)){
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }else{
                break;
            }
        }
        if(currMatch(RSMALL)){
            root->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }else if(currMatch(LBIG)){
            error(currToken.col, currToken.col, shouldRsmall);
        }
        return;
    }

    void parser_sentence_compound(TreeNode* root) { // ＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
        // TODO 复合语句
        while(!currMatch(RBIG,0)) {
            if(currMatch(CONSTTK,0)){
                // 常量声明
                TreeNode* node1 = new TreeNode(DECLARE_CONST);
                root->children.push_back(node1);
                parser_const_declare(node1);  // return时, 已经读取到了下一个
            }else if(currMatch(INTTK,0) || currMatch(CHARTK,0)) {
                // 变量声明 | 函数声明
                parser_var_declare(root);   
            }else{
                // 语句列
                TreeNode* node1 = new TreeNode(SENTENCE_MULTI);
                root->children.push_back(node1);
                parser_sentences(node1);  // return时, 已经读取到了下一个
            }
        };
        return;
    }

    void parser_sentences(TreeNode* root) { // 语句列|条件语句==>多种语句==具体的语句
        TreeNode* st = new TreeNode(SENTENCE);
        switch (currToken.type)
        {
        case WHILETK: // while循环
            TreeNode* stWhile = new TreeNode(SENTENCE_LOOP);
            root->children.push_back(st);
            st->children.push_back(stWhile);
            parser_while(stWhile);
            break;
        case FORTK: // for循环
            TreeNode* stFor = new TreeNode(SENTENCE_LOOP);
            root->children.push_back(st);
            st->children.push_back(stFor);
            parser_for(stFor);
            break;
        case IFTK: // if条件
            TreeNode* stIf = new TreeNode(SENTENCE_IF);
            root->children.push_back(st);
            st->children.push_back(stIf);
            parser_if(stIf);
            break;
        
        default:
            delete st;
            break;
        }
    }

    void parser_for(TreeNode* root){ // <循环语句> ==> for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
        // for
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR,0)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // =
        if(!currMatch(ASSIGN,0)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* expression = new TreeNode(EXPRESSION);
        root->children.push_back(expression); 
        parser_expression(expression); // 解析表达式
        // 分号
        if(!currMatch(SEMICN,0)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 条件
        TreeNode* condition = new TreeNode(CONDITION);
        root->children.push_back(condition);
        parser_condition(condition); // 解析条件
        // 分号
        if(!currMatch(SEMICN,0)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 
        // 反括号
        if(!currMatch(RSMALL)) {
            error(currToken, shouldRsmall);
        }else{
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
    }
    void parser_while(TreeNode* root){ // <循环语句> ==> while '('＜条件＞')'＜语句＞
        // while
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 条件语句
        parser_condition_sentence(root);
    }
    
    void parser_if(TreeNode* root){  // ＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］
        // if
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 条件语句
        parser_condition_sentence(root);
        // 是否有else
        if(currMatch(ELSETK,0)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;    
            parser_sentences(root);
        }
    }

    void parser_condition_sentence(TreeNode* root){  // root ==> '('＜条件＞')'＜语句＞
        // 左括号
        if(!currMatch(LSMALL)) {
            error(currToken, illegalLexcial);
        }else{
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        // 条件
        TreeNode* condition = new TreeNode(CONDITION);
        root->children.push_back(condition);
        parser_condition(condition); // 解析条件
        // 反括号
        if(!currMatch(RSMALL)) {
            error(currToken, shouldRsmall);
        }else{
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        // 语句
        parser_sentences(root);
    }
    
    void parser_condition(TreeNode* root){ // 条件
        // TODO: 条件解析
        while(!currMatch(RSMALL,0)) NEXTTOKEN;
    }

    void parser_expression(TreeNode* root){ // ＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
        // TODO:表达式解析
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
        if(currMatch(LMID,0)){  
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
        if(!currMatch(LMID)) { return; }
        root->children.push_back(NEWLEAF);
        // 无符号整数
        NEXTTOKEN;
        arrayA = parse_int(root, true);
        // ]
        NEXTTOKEN;
        if(!currMatch(RMID)) { return; }
        root->children.push_back(NEWLEAF);
        NEXTTOKEN;

        if(currMatch(LMID,0)) {
            // [
            root->children.push_back(NEWLEAF);
            // 无符号整数
            NEXTTOKEN;
            arrayB = parse_int(root, true);
            // ]
            NEXTTOKEN;
            if(!currMatch(RMID)) { return; }
            root->children.push_back(NEWLEAF);
            NEXTTOKEN;
        }
    }

    void parse_array_init(TreeNode* root, TokenID defineType){
        if(arrayB==0){ // 只有一维度
            __array_init(root, arrayA, defineType);
            NEXTTOKEN;
        }else{
            // 开头的}
            if(!currMatch(LBIG)) {return;}
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
            if(!currMatch(RBIG)) {return;}
            root->children.push_back(NEWLEAF); 
            NEXTTOKEN;
        }
    }
    void __array_init(TreeNode* root, int d, TokenID defineType){
        if(!currMatch(LBIG)) {return;}
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
        if(!currMatch(RBIG)) {
            error(currToken.line, currToken.col, arrayCntError);
            return; // TODO: 如何跳过知道下一个可以
        }
        root->children.push_back(NEWLEAF); 
    }

    bool currMatch(TokenID tId, int log=1){
        if(log){
            if(currToken.type!=tId){
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