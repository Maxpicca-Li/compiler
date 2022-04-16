#pragma once
#include "info.hpp"
#include "util.hpp"
#include "LexicalAnalyzer.hpp"
#include "GrammarAnalyzer.hpp"
#include "ErrorHandler.hpp"
#include "SystemTableManager.hpp"
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
/* 有关宏定义，懒懒懒 */
#define NEXTTOKEN lastToken=currToken; currToken=lexer.nextToken()
#define NEWLEAF new TreeNode(currToken)

private:
    LexicalAnalyzer lexer;
    TreeNode* root = NULL;
    map<string, Variable>* varTableCurrP;
    Token currToken, lastToken;
    ofstream ofp;
    int arrayA = 0, arrayB = 0, arrayIdx = 0; // 记录数组范围

public:
    GrammarAnalyzer(string inFile, string outFile){
        this->lexer.init(inFile);
        this->lexer.doLexer();
        this->ofp.open(outFile);
        this->varTableCurrP = &varStaticTable;
        // this->lexer.printAllTokens();
    }

    ~GrammarAnalyzer(){
        deleteRoot(root);
        varTableCurrP=NULL;
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
        bool is_integral = false; // TODO: is_integral的分析
        NEXTTOKEN;
        while(!lexer.isEnd()){
            if(currMatch(CONSTTK,0)){
                // 常量声明
                TreeNode* node1 = new TreeNode(DECLARE_CONST);
                root->children.push_back(node1);
                parser_const_declare(node1);  // return时, 已经读取到了下一个
            }else if(currMatch(INTTK,0) || currMatch(CHARTK,0)) {
                // 变量声明|声明头部 // FIXME: 词法分析做符号表
                parser_unkown_declare(root);   
            }else if(currMatch(VOIDTK,0)){
                // 无返回值函数定义
                TreeNode* funcDefine = new TreeNode();
                root->children.push_back(funcDefine);
                Function f;
                f.returnType = VARVOID;
                funcDefine->children.push_back(NEWLEAF); NEXTTOKEN;
                if(currMatch(MAINTK,0)) {
                    is_integral = true;
                    funcDefine->stateId = DEFINE_FUNC_MAIN;
                }else if(currMatch(IDENFR,0)) { funcDefine->stateId = DEFINE_FUNC_NO; }
                else{ error(currToken, illegalLexcial); return; }
                f.name = currToken.valueStr;
                funcDefine->children.push_back(NEWLEAF); NEXTTOKEN;
                // 加入函数（应该声明的时候就加入）
                funcTable[f.name] = f;
                // 置位符号表
                varTableCurrP = &funcTable[f.name].varTable;
                parser_func_define(funcDefine,funcTable[f.name]);  // return时, 已经读取到了下一个
            }else{
                NEXTTOKEN;
            }
            // 归位符号表
            varTableCurrP = &varStaticTable;
        }
    }

    /* 变量说明|声明头部: 先建立tmp, 判断过程中先挂载在tmp节点上 */
    void parser_unkown_declare(TreeNode* root){ // root=程序
        // 可能的变量声明
        TreeNode* varDeclare = new TreeNode(DECLARE_VAR); 
        bool flag = true; // varDeclare的children是否为空
        while(true){
            TreeNode* tmp = new TreeNode(); // 终结符的父亲节点
            // defineType
            TokenID defineType = currToken.type;
            // f | v在parser_var_define中处理
            Function f;
            f.returnType = (defineType==INTTK)?VARINT:VARCHAR;
            tmp->children.push_back(NEWLEAF); NEXTTOKEN;
            // 标识符
            if(!currMatch(IDENFR)) { return; }
            f.name = currToken.valueStr;
            tmp->children.push_back(NEWLEAF); NEXTTOKEN;

            // ! ! ! 重要判断流程判断 ! ! !
            // root ==> 有返回值函数定义 ==> 头部声明tmp ==> 解析参数列表tmp.childern
            if(currMatch(LSMALL,0)){
                if(flag){ delete varDeclare; } // 如果中间节点varDeclare的子结点为空
                tmp->stateId = DECLARE_HEADER;
                TreeNode* funcDefine = new TreeNode(DEFINE_FUNC_RETURN); // 有|无返回值函数定义
                root->children.push_back(funcDefine);
                funcDefine->children.push_back(tmp);
                // 加入函数（应该声明的时候就加入）
                funcTable[f.name] = f;
                // 置位符号表
                varTableCurrP = &funcTable[f.name].varTable;
                parser_func_define(funcDefine, funcTable[f.name]);
                return;
            }
            // root ==> 变量声明 ==> 变量定义 ==> 有无初始化tmp ==> tmp.children
            if(flag){ 
                root->children.push_back(varDeclare);
                flag = false;
            }
            TreeNode* node2 = new TreeNode(DEFINE_VAR); // 变量定义
            varDeclare->children.push_back(node2);
            node2->children.push_back(tmp);
            parser_var_define(tmp,defineType); // {标识符}+{=值} 的判断, '{}'标识可选
            
            // 分号
            if(currMatch(SEMICN,0)){
                varDeclare->children.push_back(NEWLEAF); // 变量说明中的分号
                // defineType
                NEXTTOKEN;
                if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { return; } // 结束
            } 
        }
        return ;
    }

    void parser_func_define(TreeNode* root, Function& f){ // 有|无返回值函数定义
        // 匹配参数表
        parser_arguments(root,f); 
        // {
        if(!currMatch(LBIG)) { error(currToken,illegalLexcial); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        // 复合语句
        TreeNode* sentences =  new TreeNode(SENTENCE_COMPOUND);
        root->children.push_back(sentences);
        parser_sentence_compound(sentences);
        // }
        if(!currMatch(RBIG)) { error(currToken,illegalLexcial); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        return;
    }

    void parser_arguments(TreeNode* root, Function& f){ // 有|无返回值函数定义
        // (
        if(!currMatch(LSMALL)) return;
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        // 无参数
        if(currMatch(RSMALL,0)){ root->children.push_back(NEWLEAF); NEXTTOKEN; return;}
        // 参数表
        TreeNode* args = new TreeNode(LIST_ARGUMENT);
        root->children.push_back(args);
        while(!currMatch(RSMALL,0) && !currMatch(LBIG,0)){
            Variable v;
            // 标识符
            if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) {  error(currToken.line, currToken.col, illegalLexcial); }
            else{
                v.varType = (currToken.type==INTTK)?VARINT:VARCHAR;    
                f.argsType.push_back(v.varType);
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }
            if(!currMatch(IDENFR)){ error(currToken.line, currToken.col, illegalLexcial); }
            else{
                v.name = currToken.valueStr;
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }
            // 加入变量
            f.varTable[v.name]=v;
            // 判断是否是逗号
            if(currMatch(COMMA,0)){
                args->children.push_back(NEWLEAF); 
                NEXTTOKEN;
            }else{
                break;
            }
        }
        if(currMatch(RSMALL)){ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        else if(currMatch(LBIG)){ error(currToken.col, currToken.col, shouldRsmall);}
        return;
    }

    void parser_callfunc(TreeNode* root, Function& f){ // 函数调用 ==> *(值参数列表)
        root->stateId = (f.returnType==VARVOID)?SENTENCE_CALLFUNC:SENTENCE_CALLFUNC_RETURN;
        // 标识符
        if(!currMatch(IDENFR)) return;
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // (
        if(!currMatch(LSMALL)) return;
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        // 值参数表
        TreeNode* args = new TreeNode(LIST_ARGUMENT_VALUE);
        root->children.push_back(args);
        int idx=0; // 索引f.argsType;
        while(!currMatch(RSMALL,0) && !currMatch(LBIG,0)){
            // TODO: 值参数列表类型和数目的纠错
            TreeNode* exp = new TreeNode(EXPRESSION);
            args->children.push_back(exp);
            parser_expression(exp);
            // 逗号
            if(currMatch(COMMA,0)){
                args->children.push_back(NEWLEAF); NEXTTOKEN;
                idx++;
            }
        }
        // )
        if(currMatch(RSMALL)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }else if(currMatch(LBIG)){
            error(currToken.col, currToken.col, shouldRsmall);
        }
        return;
    }

    void parser_sentence_compound(TreeNode* root) { // ＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
        while(!currMatch(RBIG,0)) {
            if(currMatch(CONSTTK,0)){
                // 常量声明
                TreeNode* node1 = new TreeNode(DECLARE_CONST);
                root->children.push_back(node1);
                parser_const_declare(node1);  // return时, 已经读取到了下一个
            }else if(currMatch(INTTK,0) || currMatch(CHARTK,0)) {
                // 变量声明 | 函数声明
                parser_unkown_declare(root); 
            }else{
                // 语句列
                TreeNode* node1 = new TreeNode(SENTENCE_MULTI);
                root->children.push_back(node1);
                parser_sentences(node1);  // return时, 已经读取到了下一个
            }
        };
        return;
    }

    void parser_sentences(TreeNode* root) { // 语句列|条件语句|switch==>多种语句==具体的语句
        while(!currMatch(RBIG,0)){
            TreeNode* st = new TreeNode(SENTENCE);
            root->children.push_back(st);
            string name;
            switch (currToken.type)
            {
            case WHILETK:{ // while循环
                TreeNode* stWhile = new TreeNode(SENTENCE_LOOP);
                st->children.push_back(stWhile);
                parser_while(stWhile);
                }break;
            case FORTK:{ // for循环
                TreeNode* stFor = new TreeNode(SENTENCE_LOOP);
                st->children.push_back(stFor);
                parser_for(stFor);
                }break;
            case IFTK:{ // if条件
                TreeNode* stIf = new TreeNode(SENTENCE_IF);
                st->children.push_back(stIf);
                parser_if(stIf);
                }break;
            case SCANFTK:{ // 读语句
                TreeNode* stRead = new TreeNode(SENTENCE_READ);
                st->children.push_back(stRead);
                parser_read(stRead);
                if(!currMatch(SEMICN)) {error(currToken, illegalLexcial); return;}
                else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
                }break;
            case PRINTFTK:{ // 写语句
                TreeNode* stWrite = new TreeNode(SENTENCE_WRITE);
                st->children.push_back(stWrite);
                parser_write(stWrite);
                if(!currMatch(SEMICN)) {error(currToken, illegalLexcial); return;}
                else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
                }break;
            case IDENFR: {// 函数调用|赋值
                int varflag = 0;
                name = currToken.valueStr;
                if(funcTable.find(name)!=funcTable.end()){
                    // 函数调用语句
                    Function& f = funcTable[name];
                    TreeNode* stCallFunc = new TreeNode();
                    st->children.push_back(stCallFunc);
                    parser_callfunc(stCallFunc, f);
                }else{
                    // 赋值语句
                    Variable& v = getVariable(name);
                    TreeNode* stAssign = new TreeNode(SENTENCE_ASSIGN);
                    st->children.push_back(stAssign);
                    parser_assign(stAssign,v);
                }
                // 分号
                if(!currMatch(SEMICN)) {error(currToken, illegalLexcial); return;}
                else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
                }break;
            case RETURNTK:{ // 返回语句
                TreeNode* stR = new TreeNode(SENTENCE_RETURN);
                st->children.push_back(stR);
                parser_return(stR);
                if(!currMatch(SEMICN)) {error(currToken, illegalLexcial); return;}
                else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
                }break;
            case SEMICN:{ // 空语句
                /* 
                // 需要空语句
                TreeNode* stNULL = new TreeNode(SENTENCE_NULL);
                st->children.push_back(stNULL);
                // 分号
                if(!currMatch(SEMICN,0)) {error(currToken,illegalLexcial);return;}
                else { st->children.push_back(NEWLEAF); NEXTTOKEN;} 
                */
                // 不需要空标识
                st->children.push_back(NEWLEAF); NEXTTOKEN; 
                }break;
            case SWITCHTK: {// 情况语句
                TreeNode* stSwitch = new TreeNode(SENTENCE_SWITCH);
                st->children.push_back(stSwitch);
                parser_switch(stSwitch);
                }break;
            case LBIG: {// 语句列
                st->children.push_back(NEWLEAF); NEXTTOKEN;
                TreeNode* sts = new TreeNode(SENTENCE_MULTI);
                st->children.push_back(sts);
                parser_sentences(sts);
                // }
                if(!currMatch(RBIG,0)) {error(currToken,illegalLexcial);}
                else { st->children.push_back(NEWLEAF); NEXTTOKEN;}
                }break;
            default:
                root->children.pop_back();
                delete st;
                cout << "ERROR: !!!语句没有被匹配到!!!" << endl;
                NEXTTOKEN;
                break;
            }
        }
    }

    void parser_switch(TreeNode* root){ // 情况语句
        if(!currMatch(SWITCHTK,0)){ return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        parser_expression(exp);
        // 反括号
        if(!currMatch(RSMALL)) { error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 大括号
        if(!currMatch(LBIG)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 情况表
        TreeNode* node = new TreeNode(LIST_CASE);
        root->children.push_back(node);
        parser_list_case(node);
        // 缺省
        parser_switch_default(root);
        // 反括号
        if(!currMatch(RBIG)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_list_case(TreeNode* root){ // 情况表
        while(!currMatch(DEFAULTTK,0)){
            TreeNode* node = new TreeNode(SENTENCE_SWITCH_SUB);
            root->children.push_back(node);
            parser_case(node);
        }

    }

    void parser_case(TreeNode* root){ // 情况子语句
        // case
        if(!currMatch(CASETK)) {error(currToken, illegalLexcial); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 常量
        parser_const(root, currToken.type);
        // 冒号
        if(!currMatch(COLON)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 语句
        parser_sentences(root);
    }

    void parser_switch_default(TreeNode* root){ // 缺省
        // default
        if(!currMatch(DEFAULTTK)) {error(currToken, illegalLexcial); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 冒号
        if(!currMatch(COLON)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 语句
        parser_sentences(root);
    }
    
    void parser_return(TreeNode* root){ // 返回语句
        if(!currMatch(RETURNTK,0)){ return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL,0)) { return;} // 空返回
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        parser_expression(exp);
        // 反括号
        if(!currMatch(RSMALL)) { error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_write(TreeNode* root){  // 写语句
        if(!currMatch(PRINTFTK,0)){ error(currToken, illegalLexcial); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 字符串
        if(currMatch(STRCON,0)){
            TreeNode* str = new TreeNode(STRING);
            root->children.push_back(str);
            str->children.push_back(NEWLEAF); NEXTTOKEN;
            if(currMatch(COMMA,0)){
                root->children.push_back(NEWLEAF); NEXTTOKEN;
                // 表达式
                TreeNode* exp = new TreeNode(EXPRESSION);
                root->children.push_back(exp);
                parser_expression(exp); 
            }
        }else{
            // 表达式
            TreeNode* exp = new TreeNode(EXPRESSION);
            root->children.push_back(exp);
            parser_expression(exp);
        }
        // 反括号
        if(!currMatch(RSMALL)) { error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_read(TreeNode* root){  // 读语句
        if(!currMatch(SCANFTK,0)){ error(currToken, illegalLexcial); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 反括号
        if(!currMatch(RSMALL)) { error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    /* ＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞|＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞ */
    void parser_assign(TreeNode* root, Variable& v){ // 赋值语句
        // NOTE: 如果不能过样例，就只语法分析了，不根据v分析
        // 标识符
        if(!currMatch(IDENFR,0)) {return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 赋值解析
        switch (v.varType)
        {
        case VARINT:
        case VARCHAR:
            __assign_tail(root);
            break;
        case VARINT1D:
        case VARCHAR1D:
            __array_index(root);
            __assign_tail(root);
            break;
        case VARINT2D:
        case VARCHAR2D:
            __array_index(root);
            __array_index(root);
            __assign_tail(root);
            break;
        default:
            break;
        }
    }

    void __array_index(TreeNode* root){ // 赋值语句
        // [
        if(!currMatch(LMID)) {error(currToken,illegalLexcial); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        parser_expression(exp);
        // ]
        if(!currMatch(RMID)) {error(currToken,shouldRmid); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        return;
    }
    
    void __assign_tail(TreeNode* root){ // 赋值语句
        // =
        if(!currMatch(ASSIGN)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        parser_expression(exp);
        return;
    }

    void parser_for(TreeNode* root){ // <循环语句> ==> for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
        // for
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { error(currToken, illegalLexcial); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // =
        if(!currMatch(ASSIGN)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* expression = new TreeNode(EXPRESSION);
        root->children.push_back(expression); 
        parser_expression(expression); // 解析表达式
        // 分号
        if(!currMatch(SEMICN)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 条件
        TreeNode* condition = new TreeNode(CONDITION);
        root->children.push_back(condition);
        parser_condition(condition); // 解析条件
        // 分号
        if(!currMatch(SEMICN)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // =
        if(!currMatch(ASSIGN)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // +/-
        if(!currMatch(PLUS) && !currMatch(MINU)){ error(currToken, illegalLexcial); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // FIXME: 步长, 感觉也应该包括标识符. ＜步长＞::= ＜无符号整数＞ 
        TreeNode* step = new TreeNode(STEP);
        root->children.push_back(step); 
        parser_int(step);
        // 反括号
        if(!currMatch(RSMALL)) { error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN;}
        // 语句
        parser_sentences(root);
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
    
    void parser_condition(TreeNode* root){ // 条件::=＜表达式＞＜关系运算符＞＜表达式＞
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        parser_expression(exp);
        if(comp_op.find(currToken.type)!=comp_op.end()){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
        }else{
            error(currToken, illegalLexcial); // 报错
            return;
        }
        TreeNode* exp2 = new TreeNode(EXPRESSION);
        root->children.push_back(exp2);
        parser_expression(exp2);
    }

    void parser_expression(TreeNode* root){ // ＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
        root->stateId = EXPRESSION; // 避免之前初始化错误
        if(currMatch(PLUS,0) || currMatch(MINU,0)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        TreeNode* term = new TreeNode(TERM);
        root->children.push_back(term);
        parser_term(term);
        while(currMatch(PLUS,0) || currMatch(MINU,0)){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            TreeNode* term = new TreeNode(FACTOR);
            root->children.push_back(term);
            parser_term(term);
        }
    }

    void parser_term(TreeNode* root){  // ＜项＞::=＜因子＞{＜乘法运算符＞＜因子＞} 
        root->stateId = TERM; // 避免之前初始化错误
        TreeNode* factor = new TreeNode(FACTOR);
        root->children.push_back(factor);
        parser_factor(factor);
        while(currMatch(MULT,0) || currMatch(DIV,0)){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            TreeNode* factor = new TreeNode(FACTOR);
            root->children.push_back(factor);
            parser_factor(factor);
        }
    }

    void parser_factor(TreeNode* root){  // 因子
        root->stateId = FACTOR; // 避免之前初始化错误
        if(currMatch(LSMALL,0)){
            // (
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            // 表达式
            TreeNode* exp = new TreeNode(EXPRESSION);
            root->children.push_back(exp);
            parser_expression(exp);
            // )
            if(!currMatch(RSMALL)) {error(currToken, shouldRmid); return;}
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            return;
        }else if(currMatch(INTCON,0) || currMatch(PLUS,0) || currMatch(MINU,0)){ // 可能不是整数，还有+/-项
            parser_int(root);
        }else if(currMatch(CHARCON,0)){
            parser_char(root);
        }else if(currMatch(IDENFR,0)){
            string name = currToken.valueStr;
            if(funcTable.find(name)!=funcTable.end()){
                Function& f = funcTable[name];
                // 函数调用
                TreeNode* node = new TreeNode();
                root->children.push_back(node);
                parser_callfunc(node,f);
            }else{
                // NOTE: 如果不能过样例，就只语法分析了，不根据v分析
                Variable& v = getVariable(name);
                // 标识符
                root->children.push_back(NEWLEAF); NEXTTOKEN;
                // 赋值解析
                switch (v.varType)
                {
                case VARINT:
                case VARCHAR:
                    break;
                case VARINT1D:
                case VARCHAR1D:
                    __array_index(root);
                    break;
                case VARINT2D:
                case VARCHAR2D:
                    __array_index(root);
                    __array_index(root);
                    break;
                default:
                    break;
                }
            }
        }
    }


    void parser_var_define(TreeNode* root, TokenID defineType){ // root=变量定义有无初始化
        // defineSub: 
        // 类标识符解决了; 标识符第一次判断解决了, ','后的子句判断没解决
        Variable v;
        v.varType = (defineType==INTTK)?VARINT:VARCHAR;
        int perSize = (defineType==INTTK)?sizeof(int):sizeof(char);
        if(currMatch(COMMA,0)){
            // 子句中的变量定义有无初始化, 需要解决标识符问题
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            if(!currMatch(IDENFR)) { return; }
            v.name = currToken.valueStr;
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }else if(lastToken.type==IDENFR){
            v.name = lastToken.valueStr;
        }
        if(currMatch(LMID,0)){  
            // 数组
            arrayA = 0;arrayB = 0;
            parser_brack(root); // 解析维度
            v.sizeA = arrayA; 
            v.sizeB = arrayB; 
            if(arrayB!=0){
                v.varType = (defineType==INTTK)?VARINT2D:VARCHAR2D;
                v.size = perSize*arrayB*arrayA;
                v.valueP = new char(v.size);
            }else{
                v.varType = (defineType==INTTK)?VARINT1D:VARCHAR1D;
                v.size = perSize*arrayA;
                v.valueP = new char(v.size);
            }
            if(currMatch(ASSIGN)) { 
                // 有初始化
                root->stateId = DEFINE_VAR_INIT;    
                root->children.push_back(NEWLEAF); NEXTTOKEN;
                parser_array_init(root,defineType,v); // 初始化解析
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
                // 常量
                parser_const(root,defineType);
            }else{
                // 无初始化
                root->stateId = DEFINE_VAR_NO;
            }
        }
        // v加入符号表
        varTableCurrP->insert({v.name, v});
        // FIXME: DEBUGout
        cout << v.name << "已经加入符号表" << varTableCurrP << endl;
        // 已经到下一个了
        if(currMatch(COMMA,0)){
            parser_var_define(root,defineType);
        }
    }

    int parser_const(TreeNode* root, TokenID caseType){ // root==> 常量
        TreeNode* cst = new TreeNode(CONST);
        root->children.push_back(cst);
        // 整数|字符
        if(caseType==INTTK || caseType==INTCON || caseType==PLUS || caseType==MINU) return parser_int(cst);
        else if(caseType==CHARTK || caseType==CHARCON) return parser_char(cst);
        else{ cout << "ERROR: !!! 未知常量类型!!!" << endl; return 0;}
    }
    void parser_const_declare(TreeNode* root){ // root==>常量说明
        // 判断
        if (!currMatch(CONSTTK)) return;
        root->children.push_back(NEWLEAF);
        
        // 常量定义
        TreeNode* node = new TreeNode(DEFINE_CONST);
        root->children.push_back(node);

        NEXTTOKEN;
        if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { return; }
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
        Variable v;
        v.isConst = true;
        if(defineType==INTTK) {v.varType = VARINT;}
        else if(defineType==CHARTK)  {v.varType = VARCHAR;}
        NEXTTOKEN;
        if(!currMatch(IDENFR)) { return; }
        v.name = currToken.valueStr;
        root->children.push_back(NEWLEAF); NEXTTOKEN;

        // =
        if(!currMatch(ASSIGN)) { return; }
        root->children.push_back(NEWLEAF); 
        
        NEXTTOKEN;
        if(defineType==INTTK){ // 数字
            int value = parser_int(root);
            v.size = sizeof(value);
            v.valueP = new char(v.size);
            memcpy(v.valueP,&value,v.size);
        }else{ // 字符
            char value = parser_char(root);
            v.size = sizeof(value);
            v.valueP = new char(v.size);
            memcpy(v.valueP,&value,v.size);
        }
        // v加入符号表
        varTableCurrP->insert({v.name, v});
        // 逗号和分号的区别
        if(currMatch(COMMA,0)){
            root->children.push_back(NEWLEAF);
            parser_const_define(root,defineType); // 继续常量定义
        } 
    }

    int parser_int(TreeNode* root, bool unsign=false){ // 有+|-的是整数，没有则为无符号整数
        TreeNode* node2 = new TreeNode(UNSIGNED_INT);
        if(!unsign){
            TreeNode* node1 = new TreeNode(INT);
            root->children.push_back(node1);
            node1->children.push_back(node2);
        }else{
            root->children.push_back(node2);
        }
        // +，-
        bool minus = false;
        if(currMatch(PLUS,0) || currMatch(MINU,0)){
            minus = currMatch(MINU,0);
            // 判断数字是否为正
            if(unsign && minus) {error(currToken.line, currToken.col, arrayDimError);}
            node2->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        // 数字
        if(!currMatch(INTCON)){return -1;}
        node2->children.push_back(NEWLEAF); 
        // 结果
        int res = str2int(currToken.valueStr);
        NEXTTOKEN;
        return minus ? -res : res;
    }

    char parser_char(TreeNode* root){
        // char
        if(!currMatch(CHARCON)) { cout<<"ERROR:误入char"; error(currToken,illegalLexcial);return 0; }
        root->children.push_back(NEWLEAF); 
        char res = currToken.valueStr[0];
        NEXTTOKEN;
        return res;
    }

    void parser_brack(TreeNode* root){ // 叶结点的上方
        // [
        if(!currMatch(LMID)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 无符号整数
        arrayA = parser_int(root, true);
        if(!currMatch(RMID)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        if(currMatch(LMID,0)) {
            // [
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            // 无符号整数
            arrayB = parser_int(root, true);
            if(!currMatch(RMID)) { return; }
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
    }

    void parser_array_init(TreeNode* root, TokenID defineType, Variable& v){
        arrayIdx = 0;
        if(arrayB==0){ // 只有一维度
            __array_init(root, arrayA, defineType, v);
        }else{
            // 开头的}
            if(!currMatch(LBIG)) {return;}
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            __array_init(root, arrayB, defineType, v);
            // 中间的，
            if(!currMatch(COMMA)) {return;}
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            __array_init(root, arrayB, defineType, v);
            // 结尾的}
            if(!currMatch(RBIG)) {return;}
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
    }
    
    void __array_init(TreeNode* root, int d, TokenID defineType, Variable& v){
        if(!currMatch(LBIG)) {return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // TODO: 数组类型和维度的比较
        for(int i=0;i<d;i++){
            if(i){
                if(!currMatch(COMMA)){ break; }
                else{ root->children.push_back(NEWLEAF); NEXTTOKEN;}
            }
            if(defineType==INTTK) {
                int res = parser_const(root,defineType); 
                memcpy(v.valueP+arrayIdx*sizeof(int), &res, sizeof(int));
            }else {
                char res = parser_const(root,defineType); 
                memcpy(v.valueP+arrayIdx*sizeof(char), &res, sizeof(char));
            }
            ++arrayIdx;
        }
        if(!currMatch(RBIG)) {
            error(currToken.line, currToken.col, arrayCntError);
            return; // TODO: 如何跳过知道下一个可以
        }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
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

    Variable& getVariable(string name){
        // 两张表
        int varflag = 0;
        if(varStaticTable.find(name)!=varStaticTable.end()) varflag = 1;
        else if(varTableCurrP->find(name)!=varTableCurrP->end()) varflag = 2; 
        // NOTE: 查找变量的限制，可能找不到对应的变量导致返回
        if(varflag==0) {
            error(currToken,undefined); 
            cout << "ERROR!!! 没有找到变量" << name << "，应该写错了，或者覆盖了，查找范围：" << &varStaticTable << "," << varTableCurrP<< endl;
            // 就新建立一个 Variable
            Variable tmp;
            tmp.name = name; // 其他值保持默认
            varTableCurrP->insert({name, tmp});
            Variable& v = varTableCurrP->find(name)->second;
            return v;
        }else{
            Variable& v = (varflag==1)?varStaticTable[name]:varTableCurrP->find(name)->second;
            return v;
        }
    }

};