#include<bits/stdc++.h>
#include "info.hpp"
#include "LexicalAnalyzer.hpp"
#include "GrammarAnalyzer.hpp"
#include "ErrorHandler.hpp"
using namespace std;

#define SUBMIT 1
#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
#else
    // const string inFile = "./data/1/testfile8.txt";
    string idx = "2"; 
    const string inFile = "./data/"+idx+"_gcase.txt";
    const string outFile = "./data/"+idx+"_goutputM.txt";
#endif

int main(){
    
    GrammarAnalyzer parser(inFile, outFile);
    parser.doParser();
    parser.print_res();

    return 0;
}