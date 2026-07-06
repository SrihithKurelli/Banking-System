#include "Utils.h"
#include <iostream>
#include <conio.h>
using namespace std;

string getHiddenPassword(){
    string password;
    char ch;
    
    while((ch = _getch())!='\r'){
        if(ch=='\b'){
            if(!password.empty()){
                password.pop_back();
                cout << "\b \b";
            }
        }
        else{
            password+=ch;
            cout << '*';
        }
    }
    cout << endl;
    return password;
}

int getValidInt(){
    int value;
    while(true){
        cin>>value;
        if(cin.fail()){
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid Input! Enter Again: ";
        }
        else{
            return value;
        }
    }
}

double getValidDouble(){
    double value;
    while(true){
        cin>>value;
        if(cin.fail()){
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid Input! Enter Again: ";
        }
        else{
            return value;
        }
    }
}
