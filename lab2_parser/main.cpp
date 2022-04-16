#include<bits/stdc++.h>
#include<io.h>
#include "info.hpp"
#include "LexicalAnalyzer.hpp"
#include "GrammarAnalyzer.hpp"
#include "ErrorHandler.hpp"
using namespace std;

// #define SUBMIT 1
#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
#else
    // const string inFile = "./data/1/testfile8.txt";
    string idx = "1"; 
    const string inFile = "./data/"+idx+"_gcase.txt";
    const string outFile = "./data/M_"+idx+"_goutput.txt";
#endif

void test(){
    string inFile;
    string outFile;
    for(int i=2;i<=6;i++){
        for(int j=3;j<=10;j++){
            time_t s = clock();
            inFile = "../data/"+to_string(i)+"/testfile"+to_string(j)+".txt";
            if(_access(inFile.c_str(),0)==-1) continue;;
            outFile = "../data/"+to_string(i)+"/myOutput"+to_string(j)+".txt";
            printf("\n============\n");
            GrammarAnalyzer parser(inFile, outFile);
            parser.doParser();
            parser.print_res();        
            time_t t = clock();
            printf("testfile%d-%d,used %.f s and pass.\n",i,j,(t-s)*1.0/1000);
        }
    }
    
}

int main(){
    /* 逐步测试，有点小问题，不知道是不是空间申请的原因 */ 
    // test();
    /* OJ测试 */
    GrammarAnalyzer parser(inFile, outFile);
    parser.doParser();
    parser.print_res();
    return 0;
}