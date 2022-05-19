#include<bits/stdc++.h>
// #include<io.h> // 验证访问一致性
#include "info.hpp"
#include "ErrorHandler.hpp"
#include "GrammarAnalyzer.hpp"
#include "TargetCodeGenerator.hpp"
using namespace std;

#define SUBMIT 1
#ifdef SUBMIT
    const string inFile = "testfile.txt";
    const string outFile = "output.txt";
    const string errFile = "error.txt";
    const string jsonFile = "test.json";
    const string codeFile = "mips.txt";
#else
    string idx = "1"; 
    // const string inFile = "./edata/" + idx + "/testfile" + idx + ".txt";
    // const string outFile = "./edata/" + idx + "/goutput" + idx + ".txt";
    // const string errFile = "./edata/" + idx + "/eoutput" + idx + ".txt";
    const string inFile = "./gdata/" + idx + "_gcase.txt";
    const string outFile = string("./gdata/") + "M_" + idx + "_goutput.txt";
    const string errFile = string("./edata/") + "M_" + idx + "_eoutput.txt";
    const string jsonFile = "tree.json";
#endif

void test(string inFile, string outFile="", string errFile="", string jsonFile="", string codeFile=""){
    ehandler.init();
    cout << inFile << " " << outFile << " " << errFile << endl;
    // GrammarAnalyzer parser(inFile);
    // parser.doParser();
    // if(outFile!="") parser.print_res(outFile);
    // if(errFile!="") ehandler.printError(errFile);
    // if(jsonFile!="") parser.print_json(jsonFile);
    TargetCodeGenerator tcg;
    tcg.doGenerate(inFile);
    tcg.printMIPS(codeFile);
}

void testAll(){
    string inFile, outFile, errFile;
    for(int i=2;i<=2;i++){
        for(int j=1;j<=10;j++){
            time_t s = clock();
            // inFile = "../data/"+to_string(i)+"/testfile"+to_string(j)+".txt";
            // outFile = "../data/"+to_string(i)+"/myOutput"+to_string(j)+".txt";
            inFile = "./edata/" + to_string(i) + "/testfile" + to_string(j) + ".txt";
            outFile = "./edata/" + to_string(i) + "/goutput" + to_string(j) + ".txt";
            errFile = "./edata/" + to_string(i) + "/eoutput" + to_string(j) + ".txt";
            // if(_access(inFile.c_str(),0)==-1) continue; // 验证能否访问
            // test(inFile, outFile, errFile);
            time_t t = clock();
            printf("testfile%d-%d,used %.f s and pass.\n",i,j,(t-s)*1.0/1000);
            printf("============\n");
        }
    }
}

void getSrcCode(string inFlie,string outFile){
    ifstream ifp;
    ofstream ofp;
    ifp.open(inFile, ios::binary);
    ofp.open(outFile);
    char ch;
    while(!ifp.eof()){
        ifp.read((char*)&ch,1);
        ofp << ch;
    }
    ifp.close();
    ofp.close();
}

int main(){ 
    // testAll();
    test(inFile, outFile, errFile, jsonFile,codeFile);
    // getSrcCode(inFile, codeFile);
    return 0;
}