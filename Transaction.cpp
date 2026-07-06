#include "Transaction.h"
#include "sqlite3.h"
#include<iostream>
#include<ctime>
using namespace std;

//Timestamp Generator
string Transaction::getCurrentTime(){
    time_t now = time(0);
    char*dt = ctime(&now);
    string timestamp(dt);
    if(!timestamp.empty()&&timestamp[timestamp.size()-1]=='\n'){
        timestamp.pop_back();
    }
    return timestamp;
}

//Record Deposit
void Transaction::recordDeposit(const string& accountNo, double amount){
     sqlite3* db;

    if(sqlite3_open("bank.db", &db)!= SQLITE_OK){
        cout << "Database Error!" <<endl;
        return;
    }

    const char* sql =
        "INSERT INTO Transactions "
        "(timestamp,accountNo,type,otherAccount,amount) "
        "VALUES(?,?,?,?,?);";

    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,getCurrentTime().c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,"DEPOSIT",-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,"",-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,5,amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//Record Withdrawal
void Transaction::recordWithdraw(const string& accountNo, double amount){
    sqlite3* db;

    if(sqlite3_open("bank.db", &db)!= SQLITE_OK){
        cout << "Database Error!" <<endl;
        return;
    }

    const char* sql =
        "INSERT INTO Transactions "
        "(timestamp,accountNo,type,otherAccount,amount) "
        "VALUES(?,?,?,?,?);";

    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,getCurrentTime().c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,"WITHDRAW",-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,"",-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,5,amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//Record Transfer
void Transaction::recordTransfer(const string& sender, const string& receiver, double amount){
    sqlite3* db;

    if(sqlite3_open("bank.db", &db)!= SQLITE_OK){
        cout << "Database Error!" <<endl;
        return;
    }

    const char* sql =
        "INSERT INTO Transactions "
        "(timestamp,accountNo,type,otherAccount,amount) "
        "VALUES(?,?,?,?,?);";

    sqlite3_stmt* stmt;

    //Sender Entry
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,getCurrentTime().c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,sender.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,"TRANSFER_OUT",-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,receiver.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,5,amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    //Receiver Entry
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,getCurrentTime().c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,receiver.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,"TRANSFER_IN",-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,sender.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,5,amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//Show Transaction History
void Transaction::showHistory(const string& accountNo){
    sqlite3* db;

    if(sqlite3_open("bank.db", &db)!= SQLITE_OK){
        cout << "Database Error!" << endl;
        return;
    }

    const char* sql =
        "SELECT timestamp,type,otherAccount,amount "
        "FROM Transactions "
        "WHERE accountNo=? "
        "ORDER BY id;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);

    bool found = false;
    while(sqlite3_step(stmt)== SQLITE_ROW){
        found = true;
        string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        string otherAccount = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        double amount = sqlite3_column_double(stmt,3);
        cout << timestamp << " | " << type << " | ";
        if(!otherAccount.empty()){
            cout << otherAccount << " | ";
        }
        cout << amount <<endl;
    }

    if(!found){
        cout << "No Transactions Found!" <<endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


