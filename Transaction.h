#ifndef TRANSACTION_H
#define TRANSACTION_H

#include<string>
using namespace std;

class Transaction{
    public:
        static void recordDeposit(const string& accountNo, double amount);

        static void recordWithdraw(const string& accountNo, double amount);

        static void recordTransfer(const string& sender, const string& receiver, double amount);

        static void showHistory(const string& accountNo);

    private:
        static string getCurrentTime();
};

#endif