#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
using namespace std;

class Database {
private:
    sqlite3* db;

public:
    Database();
    ~Database();

    bool open();
    void close();

    sqlite3* getDB();

    void initializeTables();
};

#endif