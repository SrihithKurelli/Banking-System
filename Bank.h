#ifndef BANK_H
#define BANK_H

#include "Database.h"
#include <string>
using namespace std;

class Bank{
    private:
        Database database;
        string currentUser;

        string hashPassword(const string& password);
        string generateAccountNo();

    public:

        //Constructor & Destructor
        Bank();
        ~Bank();

        //Authentication
        void createAccount();
        bool login(const string& accountNo, const string& password);
        string getCurrentUser() const;
        void logout();

        //Banking Operations
        void deposit(const string& accountNo, double amount);
        void withdraw(const string& accountNo, double amount);
        void transfer(const string& sender, const string& receiver, double amount);
        void showBalance(const string& accountNo);
        void changePassword(const string& accountNo);

        //Admin Operations
        bool adminLogin(const string& password);
        void showAllAccounts();
        void searchAccount(const string& accountNo);
        void showBankSummary();
};

#endif