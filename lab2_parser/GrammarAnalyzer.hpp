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
    VartypeID returnVid;
    vector<VartypeID> returnVec;

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
        NEXTTOKEN;
        while(!lexer.isEnd()){
            if(currMatch(CONSTTK,0)){
                // 常量声明
                TreeNode* node1 = new TreeNode(DECLARE_CONST);
                root->children.push_back(node1);
                parser_const_declare(node1);  // return时, 已经读取到了下一个
            }else if(currMatch(INTTK,0) || currMatch(CHARTK,0)) {
                // 变量声明|声明头部
                parser_unkown_declare(root);   
            }else if(currMatch(VOIDTK,0)){
                // 无返回值函数定义
                TreeNode* funcDefine = new TreeNode();
                root->children.push_back(funcDefine);
                Function f;
                f.returnType = VARVOID;
                funcDefine->children.push_back(NEWLEAF); NEXTTOKEN;
                if(currMatch(MAINTK,0)) {
                    funcDefine->stateId = DEFINE_FUNC_MAIN;
                }else if(currMatch(IDENFR,0)) { funcDefine->stateId = DEFINE_FUNC_NO; }
                else{ 
                    ehandler.errorUnkown(currToken,"无法判断为main或函数");
                    return; 
                }
                f.name = currToken.valueStr;
                funcDefine->children.push_back(NEWLEAF); NEXTTOKEN;
                addFunction(f);
                parser_func_define(funcDefine,funcTable[f.name]);  // return时, 已经读取到了下一个
            }else{
                NEXTTOKEN;
            }
            // 归位符号表
            varTableCurrP = &varStaticTable;
            // 重置为返回值类型
            returnVid = VARVOID;
            // 清空返回值列表
            returnVec.clear();
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
                addFunction(f);
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
            if(currMatch(SEMICN,0)){ varDeclare->children.push_back(NEWLEAF); NEXTTOKEN; }
            else{ ehandler.error(lastToken, shouldSEMICN); } // 避免换行
            if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { return; } // 结束
        }
        return ;
    }

    void parser_func_define(TreeNode* root, Function& f){ // 有|无返回值函数定义
        // 匹配参数表
        parser_arguments(root,f); 
        // {
        if(!currMatch(LBIG)) { ehandler.errorUnkown(currToken,"应该为{"); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        // 复合语句
        TreeNode* sentences =  new TreeNode(SENTENCE_COMPOUND);
        root->children.push_back(sentences);
        parser_sentence_compound(sentences);
        if(returnVid!=VARVOID && returnVec.size()==0){ // 有返回值的函数无任何返回语句
            ehandler.error(currToken,funcWithReturn);
        }
        // }
        if(!currMatch(RBIG)) { ehandler.errorUnkown(currToken,"应该为}"); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        return;
    }

    void parser_arguments(TreeNode* root, Function& f){ // 有|无返回值函数定义
        // (
        if(!currMatch(LSMALL,0)) return;
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        // main函数没有参数表
        if(f.name=="main"){
            if(currMatch(RSMALL,0)){ root->children.push_back(NEWLEAF); NEXTTOKEN;}
            else { ehandler.error(currToken.col, currToken.col, shouldRsmall); }
            return;
        }
        // 其他函数要记录参数表
        TreeNode* args = new TreeNode(LIST_ARGUMENT);
        root->children.push_back(args);
        int save_currLineNumber = currToken.line; // 应该在同一行解决
        while(!currMatch(RSMALL,0) && !currMatch(SEMICN,0) && currToken.line==save_currLineNumber){
            Variable v;
            // 标识符
            if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) {  ehandler.errorUnkown(currToken, "缺少类型标识符"); }
            else{
                v.varType = (currToken.type==INTTK)?VARINT:VARCHAR;    
                f.argsType.push_back(v.varType);
                args->children.push_back(NEWLEAF); NEXTTOKEN;
            }
            if(!currMatch(IDENFR)){ ehandler.errorUnkown(currToken, "缺少命名标识符"); }
            else{
                v.name = currToken.valueStr;
                args->children.push_back(NEWLEAF); NEXTTOKEN;
            }
            // 加入变量
            f.varTable[v.name]=v;
            // 判断是否是逗号
            if(currMatch(COMMA,0)){ args->children.push_back(NEWLEAF); NEXTTOKEN; }
            else{ break; }
        }
        if(currMatch(RSMALL)){ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        else { ehandler.error(currToken.col, currToken.col, shouldRsmall);}
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
        int idx=0, argsNumber = f.argsType.size(); // 索引
        int save_currLineNumber = currToken.line;
        // 这里应该根据token调用完，再去检查是否符号类型和数量
        while(!currMatch(RSMALL,0) && !currMatch(SEMICN,0) && currToken.line==save_currLineNumber){
            TreeNode* exp = new TreeNode(EXPRESSION);
            args->children.push_back(exp);
            VartypeID vid = parser_expression(exp);
            if(idx<argsNumber){
                if(vid != f.argsType[idx]) ehandler.error(currToken, funcParamsType);
            }
            idx++;
            // 逗号
            if(currMatch(COMMA,0)){ args->children.push_back(NEWLEAF); NEXTTOKEN; }
            else{ break; }
        }
        if(argsNumber && idx!=argsNumber) { ehandler.error(currToken, funcParamsNumber); }
        // )
        if(currMatch(RSMALL)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }else{
            ehandler.error(currToken, shouldRsmall);
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
                parser_sentence_multi(root);  // return时, 已经读取到了下一个
            }
        };
        return;
    }

    void parser_sentence_multi(TreeNode* root){ // 复合语句==>语句列
        TreeNode* node1 = new TreeNode(SENTENCE_MULTI);
        root->children.push_back(node1);
        while(!currMatch(RBIG,0)){
            parser_sentences(node1);
        }
    }

    void parser_sentences(TreeNode* root) { // 语句列==>具体的语句
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
            if(!currMatch(SEMICN)) {ehandler.error(lastToken, shouldSEMICN);}
            else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
            }break;
        case PRINTFTK:{ // 写语句
            TreeNode* stWrite = new TreeNode(SENTENCE_WRITE);
            st->children.push_back(stWrite);
            parser_write(stWrite);
            if(!currMatch(SEMICN)) {ehandler.error(lastToken, shouldSEMICN);}
            else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
            }break;
        case IDENFR: {// 函数调用|赋值
            name = currToken.valueStr;
            // 可能是函数，也可能是变量
            if(judge_is_assign_or_call(name)){
                // 赋值语句
                Variable& v = getVariable(name);
                if(v.isConst) ehandler.error(currToken,constChange);
                TreeNode* stAssign = new TreeNode(SENTENCE_ASSIGN);
                st->children.push_back(stAssign);
                parser_assign(stAssign,v);
            }else{
                // 函数调用语句
                Function& f = funcTable[name];
                TreeNode* stCallFunc = new TreeNode();
                st->children.push_back(stCallFunc);
                parser_callfunc(stCallFunc, f);
            }
            // 分号
            if(!currMatch(SEMICN)) {ehandler.error(lastToken, shouldSEMICN);}
            else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
            }break;
        case RETURNTK:{ // 返回语句
            TreeNode* stR = new TreeNode(SENTENCE_RETURN);
            st->children.push_back(stR);
            parser_return(stR);
            if(!currMatch(SEMICN)) {ehandler.error(lastToken, shouldSEMICN);}
            else {st->children.push_back(NEWLEAF); NEXTTOKEN;}
            }break;
        case SEMICN:{ // 空语句
            st->children.push_back(NEWLEAF); NEXTTOKEN; 
            }break;
        case SWITCHTK: {// 情况语句
            TreeNode* stSwitch = new TreeNode(SENTENCE_SWITCH);
            st->children.push_back(stSwitch);
            parser_switch(stSwitch);
            }break;
        case LBIG: {// 语句列
            st->children.push_back(NEWLEAF); NEXTTOKEN;
            parser_sentence_multi(st);
            // }
            if(!currMatch(RBIG,0)) { ehandler.errorUnkown(currToken, "没有匹配到}"); }
            else { st->children.push_back(NEWLEAF); NEXTTOKEN;}
            // 注意，语句列要返回
            return;
            }break;
        default:
            root->children.pop_back();
            delete st;
            NEXTTOKEN;
            ehandler.errorUnkown(currToken, "未知的sentence");
            break;
        }
    }

    bool judge_is_assign_or_call(string name){ // 函数调用|赋值
        bool varFlag = false;
        if(varTableCurrP->find(name)!=varTableCurrP->end() || varStaticTable.find(name)!=varStaticTable.end()){
            // 先检查变量
            varFlag = true;
        }else if(funcTable.find(name)!=funcTable.end()){
            // 再检查函数
        }else{
            // 都没有，则新建变量
            varFlag = true;
        }
        return varFlag;
    }

    void parser_switch(TreeNode* root){ // 情况语句
        if(!currMatch(SWITCHTK,0)){ return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { ehandler.errorUnkown(currToken, "没有匹配到("); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        VartypeID vid = parser_expression(exp);
        // 反括号
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 大括号
        if(!currMatch(LBIG)) { ehandler.errorUnkown(currToken, "没有匹配到{"); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 情况表
        TreeNode* node = new TreeNode(LIST_CASE);
        root->children.push_back(node);
        parser_list_case(node,vid);
        // 缺省
        parser_switch_default(root);
        // 反括号
        if(!currMatch(RBIG)) { ehandler.errorUnkown(currToken, "没有匹配到}"); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_list_case(TreeNode* root, VartypeID vid){ // 情况表
        while(!currMatch(DEFAULTTK,0) && !currMatch(RBIG,0)){
            TreeNode* node = new TreeNode(SENTENCE_SWITCH_SUB);
            root->children.push_back(node);
            parser_case(node,vid);
        }

    }

    void parser_case(TreeNode* root, VartypeID vid){ // 情况子语句
        // case
        if(!currMatch(CASETK)) { ehandler.errorUnkown(currToken, "不合法parser_case"); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 常量
        parser_const(root, vid);
        // 冒号
        if(!currMatch(COLON)) { ehandler.errorUnkown(currToken, "不合法parser_case中的冒号"); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 语句
        parser_sentences(root);
    }

    void parser_switch_default(TreeNode* root){ // 缺省
        if(!currMatch(DEFAULTTK)) {ehandler.error(currToken, lackDeafult); return;}
        // 缺省
        TreeNode* node = new TreeNode(SENTENCE_SWITCH_DEFAULT);
        root->children.push_back(node);
        // default
        node->children.push_back(NEWLEAF); NEXTTOKEN;
        // 冒号
        if(!currMatch(COLON)) { ehandler.errorUnkown(currToken, "不合法parser_case中的冒号"); }
        else{ node->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 语句
        parser_sentences(node);
    }
    
    void parser_return(TreeNode* root){ // 返回语句
        // TODO: lab3_testfile9_funcWithReturn
        if(!currMatch(RETURNTK,0)){ return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL,0)) {  // 形如return;的语句
            if(returnVid != VARVOID) ehandler.error(currToken,funcWithReturn);
            returnVec.push_back(returnVid); // 标志进入过return语句
            return; 
        }else{ 
            // 判返回值
            if(returnVid == VARVOID){ ehandler.error(currToken,funcNoReturn); }
            root->children.push_back(NEWLEAF); NEXTTOKEN; 
        }
        if(currMatch(RSMALL)) {  // 形如return();的语句
            if(returnVid != VARVOID) ehandler.error(currToken,funcWithReturn);
            returnVec.push_back(returnVid); // 标志进入过return语句
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            return;
        }
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        VartypeID vid = parser_expression(exp);
        if(returnVid!=VARVOID && vid!=returnVid){ // return语句中表达式类型与返回值类型不一致
            ehandler.error(currToken,funcWithReturn);
        }
        returnVec.push_back(vid);
        // 反括号
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_write(TreeNode* root){  // 写语句
        if(!currMatch(PRINTFTK,0)){ ehandler.errorUnkown(currToken, "不合法parser_write"); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { ehandler.errorUnkown(currToken, "没有匹配到("); }
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
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    void parser_read(TreeNode* root){  // 读语句
        if(!currMatch(SCANFTK,0)){ ehandler.errorUnkown(currToken, "不合法parser_read"); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 左括号
        if(!currMatch(LSMALL)) { ehandler.errorUnkown(currToken, "没有匹配到("); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符 ==> 需要判断变量
        if(!currMatch(IDENFR)) { ehandler.errorUnkown(currToken, "缺少命名标识符"); }
        else{ 
            Variable& v = getVariable(currToken.valueStr);
            if(v.isConst) ehandler.error(currToken, constChange);
            root->children.push_back(NEWLEAF); NEXTTOKEN; 
        }
        // 反括号
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        return;
    }

    /* ＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞|＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞ */
    void parser_assign(TreeNode* root, Variable& v){ // 赋值语句
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
            __assign_tail(root);
            break;
        }
    }

    void __array_index(TreeNode* root){ // 赋值语句
        // [
        if(!currMatch(LMID)) { ehandler.errorUnkown(currToken, "没有匹配到["); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 表达式
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        VartypeID vid= parser_expression(exp);
        if(vid!=VARINT) ehandler.error(currToken,arrayIdxError);
        // ]
        if(!currMatch(RMID)) {ehandler.error(currToken,shouldRmid); return;}
        root->children.push_back(NEWLEAF);NEXTTOKEN;
        return;
    }
    
    void __assign_tail(TreeNode* root){ // 赋值语句
        // =
        if(!currMatch(ASSIGN)) { ehandler.errorUnkown(currToken, "没有匹配到="); }
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
        if(!currMatch(LSMALL)) { ehandler.errorUnkown(currToken, "没有匹配到("); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ ehandler.errorUnkown(currToken, "缺少命名标识符"); }
        else { 
            getVariable(currToken.valueStr); // 注意识别变量
            root->children.push_back(NEWLEAF); NEXTTOKEN; 
        }
        // =
        if(!currMatch(ASSIGN)){ ehandler.errorUnkown(currToken, "没有匹配到="); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 表达式
        TreeNode* expression = new TreeNode(EXPRESSION);
        root->children.push_back(expression); 
        parser_expression(expression); // 解析表达式
        // 分号
        if(!currMatch(SEMICN)){ ehandler.error(lastToken, shouldSEMICN); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 条件
        TreeNode* condition = new TreeNode(CONDITION);
        root->children.push_back(condition);
        parser_condition(condition); // 解析条件
        // 分号
        if(!currMatch(SEMICN)){ ehandler.error(lastToken, shouldSEMICN); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ { ehandler.errorUnkown(currToken, "缺少命名标识符"); } }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // =
        if(!currMatch(ASSIGN)){ { ehandler.errorUnkown(currToken, "没有匹配到="); } }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 标识符
        if(!currMatch(IDENFR)){ { ehandler.errorUnkown(currToken, "缺少命名标识符"); } }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // +/-
        if(!currMatch(PLUS,0) && !currMatch(MINU,0)){ ehandler.errorUnkown(currToken, "没有匹配到+/-"); }
        else { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        TreeNode* step = new TreeNode(STEP);
        root->children.push_back(step); 
        parser_int(step,true);
        // 反括号
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
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
        if(!currMatch(LSMALL)) { ehandler.errorUnkown(currToken, "没有匹配到(");
        }else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 条件
        TreeNode* condition = new TreeNode(CONDITION);
        root->children.push_back(condition);
        parser_condition(condition); // 解析条件
        // 反括号
        if(!currMatch(RSMALL)) { ehandler.error(currToken, shouldRsmall); }
        else{ root->children.push_back(NEWLEAF); NEXTTOKEN; }
        // 语句
        parser_sentences(root);
    }
    
    void parser_condition(TreeNode* root){ // 条件::=＜表达式＞＜关系运算符＞＜表达式＞
        TreeNode* exp = new TreeNode(EXPRESSION);
        root->children.push_back(exp);
        VartypeID vid1 = parser_expression(exp);
        if(comp_op.find(currToken.type)!=comp_op.end()){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
        }else{
            ehandler.errorUnkown(currToken, "不合法比较符");
            return;
        }
        TreeNode* exp2 = new TreeNode(EXPRESSION);
        root->children.push_back(exp2);
        VartypeID vid2 = parser_expression(exp2);
        if(vid1!=VARINT || vid2!=VARINT) ehandler.error(currToken,illegalCondition);
    }

    VartypeID parser_expression(TreeNode* root){ // ＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
        root->stateId = EXPRESSION; // 避免之前初始化错误
        VartypeID vid=VARVOID;
        if(currMatch(PLUS,0) || currMatch(MINU,0)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        TreeNode* term = new TreeNode(TERM);
        root->children.push_back(term);
        vid = parser_term(term);
        while(currMatch(PLUS,0) || currMatch(MINU,0)){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            TreeNode* term = new TreeNode(FACTOR);
            root->children.push_back(term);
            parser_term(term);
        }
        return vid;
    }

    VartypeID parser_term(TreeNode* root){  // ＜项＞::=＜因子＞{＜乘法运算符＞＜因子＞} 
        root->stateId = TERM; // 避免之前初始化错误
        VartypeID vid;
        TreeNode* factor = new TreeNode(FACTOR);
        root->children.push_back(factor);
        vid = parser_factor(factor);
        while(currMatch(MULT,0) || currMatch(DIV,0)){
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            TreeNode* factor = new TreeNode(FACTOR);
            root->children.push_back(factor);
            parser_factor(factor);
        }
        return vid;
    }

    VartypeID parser_factor(TreeNode* root){  // 因子
        root->stateId = FACTOR; // 避免之前初始化错误
        VartypeID vid = VARVOID;
        if(currMatch(LSMALL,0)){
            // (
            root->children.push_back(NEWLEAF);NEXTTOKEN;
            // 表达式
            TreeNode* exp = new TreeNode(EXPRESSION);
            root->children.push_back(exp);
            vid = parser_expression(exp);
            // )
            if(!currMatch(RSMALL)) {ehandler.error(currToken, shouldRsmall);}
            else {root->children.push_back(NEWLEAF);NEXTTOKEN;}
            return vid;
        }else if(currMatch(INTCON,0) || currMatch(PLUS,0) || currMatch(MINU,0)){ // 可能不是整数，还有+/-项
            parser_int(root);
            return VARINT;
        }else if(currMatch(CHARCON,0)){
            parser_char(root);
            return VARCHAR;
        }else if(currMatch(IDENFR,0)){
            string name = currToken.valueStr;
            if(judge_is_assign_or_call(name)){
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
                if(v.varType==VARINT || v.varType==VARINT1D || v.varType==VARINT2D) return VARINT;
                else return VARCHAR;
            }else{
                Function& f = funcTable[name];
                // 函数调用
                TreeNode* node = new TreeNode();
                root->children.push_back(node);
                parser_callfunc(node,f);
                return f.returnType;
            }
        }
        return vid;
    }

    void parser_var_define(TreeNode* root, TokenID defineType){ // root=变量定义有无初始化
        // 类标识符解决了; 标识符第一次判断解决了, ','后的子句判断没解决
        Variable v;
        v.varType = (defineType==INTTK)?VARINT:VARCHAR;
        int perSize = (defineType==INTTK)?sizeof(int):sizeof(char);
        if(lastToken.type==IDENFR){ // 上一个，即defineType对应的lastToken
            v.name = lastToken.valueStr;
        }else if(currMatch(IDENFR,0)){ //     
            v.name = currToken.valueStr;
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }else{
            ehandler.errorUnkown(currToken,"ERROR:!!!无法匹配标识符!!!");
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
            }else{
                v.varType = (defineType==INTTK)?VARINT1D:VARCHAR1D;
                v.size = perSize*arrayA;    
            }
            v.valueP = new char[v.size];
            if(currMatch(ASSIGN,0)) { 
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
                parser_const(root,v.varType);
            }else{
                // 无初始化
                root->stateId = DEFINE_VAR_NO;
            }
        }
        // v加入符号表
        addVariable(v);
        // 已经到下一个了
        if(currMatch(COMMA,0)){
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            parser_var_define(root,defineType);
        }
    }

    int parser_const(TreeNode* root, VartypeID vid){ // root==> 常量
        TreeNode* cst = new TreeNode(CONST);
        root->children.push_back(cst);
        // 整数|字符
        TokenID caseType = currToken.type;
        if(caseType==INTTK || caseType==INTCON || caseType==PLUS || caseType==MINU) {
            if(vid!=VARINT) { ehandler.error(currToken, constType); }
            return parser_int(cst);
        }
        else if(caseType==CHARTK || caseType==CHARCON) {
            if(vid!=VARCHAR) { ehandler.error(currToken, constType); }
            return parser_char(cst);
        }
        else {
            ehandler.errorUnkown(currToken, "未知常量类型");
            return 0;
        }
    }
    void parser_const_declare(TreeNode* root){ // root==>常量说明
        // 判断
        if (!currMatch(CONSTTK)) return;
        root->children.push_back(NEWLEAF);
        // 常量定义
        TreeNode* node = new TreeNode(DEFINE_CONST);
        root->children.push_back(node); NEXTTOKEN;
        if(!currMatch(INTTK,0) && !currMatch(CHARTK,0)) { return; }
        parser_const_define(node, currToken.type); 
        // 当前已经处理到了分号
        if(!currMatch(SEMICN)){ ehandler.error(lastToken, shouldSEMICN); return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 判断下一个是不是const
        if(currMatch(CONSTTK,0)) parser_const_declare(root);
    }

    void parser_const_define(TreeNode* root, TokenID defineType){ // root ==> 常量定义
        // 标识符
        Variable v;
        v.isConst = true;
        if(defineType==INTTK) {
            v.varType = VARINT; v.size = sizeof(int);
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        else if(defineType==CHARTK)  {
            v.varType = VARCHAR; v.size = sizeof(char);
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        else ehandler.errorUnkown(currToken,"不合法常量类型");
        if(!currMatch(IDENFR)) { return; }
        v.name = currToken.valueStr;
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // =
        if(!currMatch(ASSIGN)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        v.valueP = new char[v.size];
        if(defineType==INTTK){ // 数字
            int value = parser_int(root);
            memcpy(v.valueP,&value,v.size);
        }else{ // 字符
            char value = parser_char(root);
            memcpy(v.valueP,&value,v.size);
        }
        // v加入符号表
        addVariable(v);
        // 逗号和分号的区别
        if(currMatch(COMMA,0)){
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
            if(unsign && minus) {ehandler.error(currToken.line, currToken.col, arrayIdxError);}
            node2->children.push_back(NEWLEAF); NEXTTOKEN;
        }
        // 数字
        if(!currMatch(INTCON)){return -1;}
        // 结果
        int res = str2int(currToken.valueStr);
        node2->children.push_back(NEWLEAF); NEXTTOKEN;
        return minus ? -res : res;
    }

    char parser_char(TreeNode* root){
        // char
        if(!currMatch(CHARCON)) { ehandler.errorUnkown(currToken, "不合法parser_char"); return 0; }
        char res = currToken.valueStr[0];
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        return res;
    }

    void parser_brack(TreeNode* root){ // 叶结点的上方
        // [
        if(!currMatch(LMID)) { return; }
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        // 无符号整数
        arrayA = parser_int(root, true);
        if(!currMatch(RMID)) { ehandler.error(currToken, shouldRmid); }
        else {root->children.push_back(NEWLEAF); NEXTTOKEN;}
        if(currMatch(LMID,0)) {
            // [
            root->children.push_back(NEWLEAF); NEXTTOKEN;
            // 无符号整数
            arrayB = parser_int(root, true);
            if(!currMatch(RMID)) { ehandler.error(currToken, shouldRmid); }
            else {root->children.push_back(NEWLEAF); NEXTTOKEN;}
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
            int i=0;
            while(1){
                __array_init(root, arrayB, defineType, v);
                ++i;
                // 中间的，
                if(!currMatch(COMMA,0)) {break;}
                root->children.push_back(NEWLEAF); NEXTTOKEN;
            }
            if(i!=arrayA) {ehandler.error(currToken, arrayCntError);}
            // 结尾的}
            if(!currMatch(RBIG)) {return;}
            root->children.push_back(NEWLEAF); NEXTTOKEN;
        }
    }
    
    void __array_init(TreeNode* root, int d, TokenID defineType, Variable& v){
        if(!currMatch(LBIG)) {return;}
        root->children.push_back(NEWLEAF); NEXTTOKEN;
        int i=0;
        while(1){
            if(i){
                if(!currMatch(COMMA)){ break; }
                root->children.push_back(NEWLEAF); NEXTTOKEN;
            }
            if(defineType==INTTK) {
                int res = parser_const(root,VARINT); 
                memcpy(v.valueP+arrayIdx*sizeof(int), &res, sizeof(int));
            }else if(defineType==CHARTK){
                char res = parser_const(root,VARCHAR); 
                memcpy(v.valueP+arrayIdx*sizeof(char), &res, sizeof(char));
            }else{
                ehandler.errorUnkown(currToken,"未知常量类型");
            }
            ++arrayIdx;
            ++i;
        }
        if(i!=d) {ehandler.error(currToken, arrayCntError);}
        if(currMatch(RBIG)) { root->children.push_back(NEWLEAF); NEXTTOKEN; }
        else { ehandler.errorUnkown(currToken,"没有匹配到}"); }
    }

    bool currMatch(TokenID tId, int log=1){
        if(log){
            /* if(currToken.type!=tId){
                cout << "预期出现<" << tokenId_str[tId] << ">, 实际出现:" << currToken;
            } */
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

    void addFunction(Function f){
        // funcTable & varStaticTable
        if(funcTable.find(f.name)!=funcTable.end() || varStaticTable.find(f.name)!=varStaticTable.end()){ // 名字重定义
            ehandler.error(currToken, redefined);
        }
        funcTable[f.name] = f;
        // 置位符号表
        varTableCurrP = &funcTable[f.name].varTable;
        // 置位返回值类型
        returnVid = f.returnType; 
    }

    void addVariable(Variable v){
        // varTableCurrP 只看当前表 (无需比较函数表)
        if(varTableCurrP->find(v.name)!=varTableCurrP->end()){ // 名字重定义
            ehandler.error(currToken, redefined);
            varTableCurrP->find(v.name)->second = v;
        } else {
            varTableCurrP->insert({v.name, v});
        }
    }

    Variable& getVariable(string name){
        // 这里由先后顺序
        if(varTableCurrP->find(name)!=varTableCurrP->end()) return varTableCurrP->find(name)->second; 
        else if(varStaticTable.find(name)!=varStaticTable.end()) return varStaticTable[name];
        else {
            ehandler.error(currToken,undefined); 
            // 就新建立一个 Variable
            Variable tmp;
            tmp.name = name; // 其他值保持默认
            varTableCurrP->insert({name, tmp});
            return varTableCurrP->find(name)->second;
        }
    }

};