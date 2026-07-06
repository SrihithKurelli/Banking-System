#include "Bank.h"
#include "Transaction.h"
#include "Utils.h"
#include<iostream>
#include<functional>
#include<stdexcept>
using namespace std;


//Constructor
Bank::Bank(){
    currentUser ="";
    if(database.open()){
        database.initializeTables();
    }
}

Bank::~Bank(){
    database.close();
}

//Password Hashing
string Bank::hashPassword(const string& password){
    hash<string>hasher;
    return to_string(hasher(password));
}

//Generate Account Number
string Bank::generateAccountNo(){
    sqlite3* db = database.getDB();

    const char* sql ="SELECT COUNT(*) FROM Accounts;";
    sqlite3_stmt* stmt;
    int count = 0;

    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    if(sqlite3_step(stmt) == SQLITE_ROW){
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return "BANK" + to_string(1001 + count);
}

//Current User Getter
string Bank::getCurrentUser() const{
    return currentUser;
}

//Logout
void Bank::logout(){
    currentUser = "";
    cout<< "Logged Out Successfully!" << endl;
}

//Create Account
void Bank::createAccount(){
    string password;
    string accountNo = generateAccountNo();
    cout<< "\nAccount Number: " << accountNo << endl;
    cout<< "Create Password: ";
    password = getHiddenPassword();
    if(password.empty()){
        cout << "Password cannot be empty!" << endl;
        return;
    }

    string hashedPassword = hashPassword(password);
    sqlite3* db = database.getDB();

    const char* sql =
        "INSERT INTO Accounts "
        "(accountNo,passwordHash,balance) "
        "VALUES(?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,hashedPassword.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt,3,0.0);

    if(sqlite3_step(stmt)== SQLITE_DONE){
        cout << "\nAccount Created Successfully!" << endl;
    }
    sqlite3_finalize(stmt);
}

//Login
bool Bank::login(const string& accountNo, const string& password){
    sqlite3* db = database.getDB();

    const char* sql =
        "SELECT passwordHash "
        "FROM Accounts "
        "WHERE accountNo=?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);

    bool success = false;

    if(sqlite3_step(stmt) == SQLITE_ROW){
        string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        if(storedHash == hashPassword(password)){
            currentUser = accountNo;
            success = true;
        }
    }
    sqlite3_finalize(stmt);
    return success;
    
}

//Deposit
void Bank::deposit(const string& accountNo, double amount){
    try{
        if(amount <= 0){
            throw runtime_error("Amount must be positive.");
        }

        sqlite3* db = database.getDB();

        const char* sql =
            "UPDATE Accounts "
            "SET balance = balance + ? "
            "WHERE accountNo = ?;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_double(stmt, 1, amount);
        sqlite3_bind_text(stmt,2,accountNo.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        Transaction::recordDeposit(accountNo, amount);

        cout << "Deposit Successful!" << endl;
    }
    catch(exception& e){
        cout << e.what() << endl;
    }
}

//Withdraw
void Bank::withdraw(const string& accountNo, double amount){
    try{
        if(amount <= 0){
            throw runtime_error("Amount must be positive.");
        }

        sqlite3* db = database.getDB();
        double balance = 0;

        const char* checkSql =
            "SELECT balance "
            "FROM Accounts "
            "WHERE accountNo=?;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,checkSql,-1,&stmt,nullptr);
        sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) == SQLITE_ROW){
            balance = sqlite3_column_double(stmt,0);
        }
        sqlite3_finalize(stmt);
        if(balance < amount){
            throw runtime_error("Insufficient Balance.");
        }

        const char* updateSql =
            "UPDATE Accounts "
            "SET balance = balance - ? "
            "WHERE accountNo=?;";

        sqlite3_prepare_v2(db,updateSql,-1,&stmt,nullptr);
        sqlite3_bind_double(stmt,1,amount);
        sqlite3_bind_text(stmt,2,accountNo.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        Transaction::recordWithdraw(accountNo,amount);
        cout << "Withdrawal Successful!" << endl;
    }
    catch(exception& e){
        cout << e.what() << endl;
    }
}

//Transfer
void Bank::transfer(const string& sender, const string&receiver, double amount){
     try{
        if(amount <= 0){
            throw runtime_error("Amount must be positive.");
        }

        if(sender == receiver){
            throw runtime_error("Cannot transfer to same account.");
        }

        sqlite3* db = database.getDB();
        char* errMsg = nullptr;

        sqlite3_exec(db,"BEGIN TRANSACTION;",nullptr,nullptr,&errMsg);
        double senderBalance = 0;
        sqlite3_stmt* stmt;

        const char* checkSql =
            "SELECT balance "
            "FROM Accounts "
            "WHERE accountNo=?;";

        //Check sender
        sqlite3_prepare_v2(db,checkSql,-1,&stmt,nullptr);
        sqlite3_bind_text(stmt,1,sender.c_str(),-1,SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) == SQLITE_ROW){
            senderBalance = sqlite3_column_double(stmt,0);
        }
        else{
            sqlite3_finalize(stmt);
            sqlite3_exec(db,"ROLLBACK;",nullptr,nullptr,&errMsg);
            throw runtime_error("Sender account not found.");
        }

        sqlite3_finalize(stmt);
        if(senderBalance < amount){
            sqlite3_exec(db,"ROLLBACK;",nullptr,nullptr,&errMsg);
            throw runtime_error("Insufficient Balance.");
        }

        //Check receiver
        sqlite3_prepare_v2(db,checkSql,-1,&stmt,nullptr);
        sqlite3_bind_text(stmt,1,receiver.c_str(),-1,SQLITE_TRANSIENT);

        if(sqlite3_step(stmt) != SQLITE_ROW){
            sqlite3_finalize(stmt);
            sqlite3_exec(db,"ROLLBACK;",nullptr,nullptr,&errMsg);
            throw runtime_error("Receiver account not found.");
        }

        sqlite3_finalize(stmt);

        //Deduct sender
        const char* deductSql =
            "UPDATE Accounts "
            "SET balance = balance - ? "
            "WHERE accountNo=?;";

        sqlite3_prepare_v2(db,deductSql,-1,&stmt,nullptr);
        sqlite3_bind_double(stmt,1,amount);
        sqlite3_bind_text(stmt,2,sender.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        //Credit receiver
        const char* addSql =
            "UPDATE Accounts "
            "SET balance = balance + ? "
            "WHERE accountNo=?;";

        sqlite3_prepare_v2(db,addSql,-1,&stmt,nullptr);
        sqlite3_bind_double(stmt,1,amount);
        sqlite3_bind_text(stmt,2,receiver.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        //Commit everything
        sqlite3_exec(db,"COMMIT;",nullptr,nullptr,&errMsg);
        Transaction::recordTransfer(sender,receiver,amount);
        cout << "Transfer Successful!" << endl;
    }
    catch(exception& e){
        cout << e.what() << endl;
    }
}

//Show Balance
void Bank::showBalance(const string& accountNo){
    sqlite3* db = database.getDB();
    const char* sql =
        "SELECT balance "
        "FROM Accounts "
        "WHERE accountNo=?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    if(sqlite3_step(stmt)== SQLITE_ROW){
        double balance =sqlite3_column_double(stmt,0);

        cout << "Current Balance: " << balance << endl;
    }
    else{
        cout << "Account not found" << endl;
    }
    sqlite3_finalize(stmt);
}

//Change Password
void Bank::changePassword(const string& accountNo){
    sqlite3* db = database.getDB();

    const char* sql =
        "SELECT passwordHash "
        "FROM Accounts "
        "WHERE accountNo=?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);

    string storedHash;

    if(sqlite3_step(stmt) == SQLITE_ROW){
        storedHash =reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
    }
    else{
        cout << "Account not found." << endl;
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    cout << "Enter Current Password: ";
    string currentPassword = getHiddenPassword();
    if(hashPassword(currentPassword)!= storedHash){
        cout << "Incorrect Password." << endl;
        return;
    }
    cout << "Enter New Password: ";
    string newPassword = getHiddenPassword();
    if(newPassword.empty()){
        cout << "Password cannot be empty." << endl;
        return;
    }

    string newHash = hashPassword(newPassword);
    if(newHash == storedHash){
        cout << "New password cannot be same as current password." << endl;
        return;
    }

    const char* updateSql =
        "UPDATE Accounts "
        "SET passwordHash=? "
        "WHERE accountNo=?;";

    sqlite3_prepare_v2(db,updateSql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,newHash.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    cout << "Password Changed Successfully!" << endl;
}

//Admin Login
bool Bank::adminLogin(const string& password){
    return password == "admin123";
}

//Show All Accounts
void Bank::showAllAccounts(){
    sqlite3* db = database.getDB();

    const char* sql =
        "SELECT accountNo,balance "
        "FROM Accounts;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    cout << "\n===== ALL ACCOUNTS =====\n";

    bool found = false;
    while(sqlite3_step(stmt)== SQLITE_ROW){
        found = true;
        string accountNo = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        double balance = sqlite3_column_double(stmt,1);
        cout << "Account No: " << accountNo << endl;
        cout << "Balance: " << balance << endl;
        cout << "-------------------" << endl;
    }

    if(!found){
        cout << "No accounts found." << endl;
    }
    sqlite3_finalize(stmt);
}

//Search Account
void Bank::searchAccount(const string& accountNo){
    sqlite3* db = database.getDB();
    const char* sql =
        "SELECT balance "
        "FROM Accounts "
        "WHERE accountNo=?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    sqlite3_bind_text(stmt,1,accountNo.c_str(),-1,SQLITE_TRANSIENT);
    if(sqlite3_step(stmt) == SQLITE_ROW){
        double balance = sqlite3_column_double(stmt,0);
        cout << "\nAccount Found\n";
        cout << "Account No: " << accountNo << endl;
        cout << "Balance: " << balance << endl;
    }
    else{
        cout << "Account not found." << endl;
    }

    sqlite3_finalize(stmt);
}

//Bank Summary
void Bank::showBankSummary(){
    sqlite3* db = database.getDB();

    const char* sql =
        "SELECT COUNT(*), "
        "COALESCE(SUM(balance),0) "
        "FROM Accounts;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db,sql,-1,&stmt,nullptr);
    int totalAccounts = 0;
    double totalBalance = 0;

    if(sqlite3_step(stmt)== SQLITE_ROW){
        totalAccounts = sqlite3_column_int(stmt,0);
        totalBalance = sqlite3_column_double(stmt,1);
    }
    sqlite3_finalize(stmt);

    cout << "\n===== BANK SUMMARY =====\n";
    cout << "Total Accounts: " << totalAccounts << endl;
    cout << "Total Money In Bank: " << totalBalance << endl;
}

