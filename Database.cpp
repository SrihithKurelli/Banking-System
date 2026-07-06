#include "Database.h"
#include <iostream>
using namespace std;

Database::Database(){
    db = nullptr;
}

Database::~Database(){
    close();
}

bool Database::open(){
    int rc = sqlite3_open("bank.db", &db);
    if(rc != SQLITE_OK){
        cout << "Cannot open database!" <<endl;
        return false;
    }
    return true;
}

void Database::close(){
    if(db!=nullptr){
        sqlite3_close(db);
        db = nullptr;
    }
}

sqlite3* Database::getDB(){
    return db;
}

void Database::initializeTables(){

    const char* accountsTable = R"(
        CREATE TABLE IF NOT EXISTS Accounts(
            accountNo TEXT PRIMARY KEY,
            passwordHash TEXT NOT NULL,
            balance REAL NOT NULL
        );
    )";

    char* errMsg = nullptr;

    if(sqlite3_exec(db,accountsTable,nullptr,nullptr,&errMsg) != SQLITE_OK) {
        cout << "Accounts table error: " << errMsg <<endl;
        sqlite3_free(errMsg);
    }

    const char* transactionsTable = R"(
        CREATE TABLE IF NOT EXISTS Transactions(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            accountNo TEXT NOT NULL,
            type TEXT NOT NULL,
            otherAccount TEXT,
            amount REAL NOT NULL
        );
    )";

    if(sqlite3_exec(db,transactionsTable,nullptr,nullptr,&errMsg) != SQLITE_OK) {
        cout << "Transactions table error: " << errMsg <<endl;
        sqlite3_free(errMsg);
    }
}