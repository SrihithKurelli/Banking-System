#include "Bank.h"
#include "Transaction.h"
#include "Utils.h"
#include<iostream>
using namespace std;


int main(){
    Bank bank;
    bool exitProgram = false;

    while(!exitProgram){
        cout << "\n============================\n";
        cout << "BANK MANAGEMENT SYSTEM\n";
        cout << "============================\n";

        cout << "1. Create Account\n";
        cout << "2. Login\n";
        cout << "3. Admin Login\n";
        cout << "4. Exit\n";

        cout << "\nEnter Choice: ";

        int choice = getValidInt();
        switch(choice){

            case 1:{
                bank.createAccount();
                break;
            }

            case 2:{
                string accountNo;
                string password;

                cout << "\nAccount Number: ";
                cin >> accountNo;
                cout << "Password: ";
                password = getHiddenPassword();

                if(bank.login(accountNo,password)){
                    
                    cout<< "\nLogin Successful!\n";

                    bool logout = false;

                    while(!logout){
                        cout << "\n============================\n";
                        cout << "USER DASHBOARD\n";
                        cout << "Logged In As: " << bank.getCurrentUser() << endl;
                        cout << "============================\n";

                        cout << "1. Deposit\n";
                        cout << "2. Withdraw\n";
                        cout << "3. Transfer\n";
                        cout << "4. Check Balance\n";
                        cout << "5. Transaction History\n";
                        cout << "6. Change Password\n";
                        cout << "7. Logout\n";
                        

                        cout << "\nEnter Choice: ";
                        int userChoice = getValidInt();
                        
                        switch(userChoice){
                            case 1:{
                                double amount;
                                cout << "Enter Amount: ";
                                amount = getValidDouble();
                                bank.deposit(bank.getCurrentUser(),amount);
                                break;
                            }
                            case 2:{
                                double amount;
                                cout << "Enter Amount: ";
                                amount = getValidDouble();
                                bank.withdraw(bank.getCurrentUser(),amount);
                                break;
                            }
                            case 3:{
                                string receiver;
                                double amount;
                                cout << "Receiver Account No: ";
                                cin >> receiver;
                                cout << "Enter Amount: ";
                                amount = getValidDouble();
                                bank.transfer(bank.getCurrentUser(),receiver,amount);
                                break;
                            }
                            case 4:{
                                bank.showBalance(bank.getCurrentUser());
                                break;
                            }
                            case 5:{
                                Transaction::showHistory(bank.getCurrentUser());
                                break;
                            }
                            case 6:{
                                bank.changePassword(bank.getCurrentUser());
                                break;
                            }
                            case 7:{
                                bank.logout();
                                logout = true;
                                break;
                            }
                            default:{
                                cout << "Invalid Choice!" << endl; 
                            }
                        }
                    }
                }
                else{
                    cout << "Invalid Account Number or Password!" << endl;
                }
                break;
            }
            case 3:{
                string adminPassword;

                cout << "\nAdmin Password: ";
                adminPassword =getHiddenPassword();

                if(bank.adminLogin(adminPassword)){
                    bool adminLogout = false;

                    while(!adminLogout){
                        cout << "\n============================\n";

                        cout << "ADMIN DASHBOARD\n";

                        cout << "============================\n";

                        cout << "1. View All Accounts\n";

                        cout << "2. Search Account\n";

                        cout << "3. Bank Summary\n";

                        cout << "4. Logout\n";

                        cout << "\nEnter Choice: ";

                        int adminChoice =getValidInt();

                        switch(adminChoice){
                            case 1:{
                                bank.showAllAccounts();
                                break;
                            }

                            case 2:{
                                string accountNo;
                                cout << "Enter Account No: ";
                                cin >> accountNo;
                                bank.searchAccount(accountNo);
                                break;
                            }

                            case 3:{
                                bank.showBankSummary();
                                break;
                            }

                            case 4:{
                                adminLogout = true;
                                break;
                            }

                            default:{
                                cout << "Invalid Choice!" << endl;
                            }
                        }
                    }
                }
                else{
                    cout << "Invalid Admin Password!" << endl;
                }

                break;
            }
            case 4:{
                exitProgram = true;
                cout << "Thank You!" << endl;
                break;
            }

            default:{
                cout << "Invalid Choice!" << endl;
            }

        }
    }
    return 0;
}