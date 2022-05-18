#pragma once
#include<iostream>
#include<string>
using std::string;
using std::cout;
using std::endl;

int str2int(string s){
    int res = 0, n = s.size();
    for(int i=0;i<n;i++){
        res = res*10+s[i]-'0';
    }
    return res;
}

float str2float(string s){
    float res;
    int a = 0, b = 0;
    int n = s.size();
    int i = 0, flag = 0;
    for(;i<n;i++){
        if(s[i]=='.') {
            flag = 1;
            continue;
        }
        if(!flag) a = a*10+s[i]-'0';
        else {
            b = b*10+s[i]-'0';
            flag *= 10;
        }
    }
    res = 1.0*a + 1.0*b/flag;
    return res;
}

void testStrTransfer(){
    cout << str2int("012340") << endl;
    float a = str2float("012340.1234");
    // cout << a << endl;
    printf("%.6f",a);
}