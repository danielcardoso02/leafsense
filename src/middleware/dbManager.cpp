/**
 * @file dbManager.cpp
 * @brief Implementation of Database Management Class
 */

#include "../../include/middleware/dbManager.h"

// Constructor
dbManager::dbManager(std::string dbPath) : info(dbPath), db(nullptr) {
    if (connect()) {
        // CRITICAL: Enable Foreign Key support immediately upon connection
        // This ensures cascading deletes for ML_PREDICTIONS and HEALTH_ASSESSMENTS work as designed.
        char* errMsg = 0;
        int rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "[DB Error] Failed to enable Foreign Keys: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
}

// Destructor
dbManager::~dbManager() {
    if (db) {
        sqlite3_close(db);
        std::cout << "[DB] Connection closed." << std::endl;
    }
}

// Connect
bool dbManager::connect() {
    int rc = sqlite3_open(info.c_str(), &db);
    if (rc) {
        std::cerr << "[DB Error] Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true; // Connection successful
}

// Callback for SELECT queries
int dbManager::callback(void* data, int argc, char** argv, char** azColName) {
    DBResult* result = static_cast<DBResult*>(data);
    
    // Populate headers only once (from the first row)
    if (result->headers.empty()) {
        for (int i = 0; i < argc; i++) {
            result->headers.push_back(azColName[i]);
        }
    }

    // Populate row data
    std::vector<std::string> row;
    for (int i = 0; i < argc; i++) {
        // Handle NULL values safely
        row.push_back(argv[i] ? argv[i] : "NULL");
    }
    result->rows.push_back(row);
    
    return 0;
}

// Insert (Wrapper for execute)
bool dbManager::insert(std::string sqlCommand) {
    return execute(sqlCommand);
}

// Remove (Wrapper for execute)
bool dbManager::remove(std::string sqlCommand) {
    return execute(sqlCommand);
}

// Read (Select)
DBResult dbManager::read(std::string sqlQuery) {
    DBResult result;
    char* zErrMsg = 0;
    
    int rc = sqlite3_exec(db, sqlQuery.c_str(), callback, &result, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "[DB Read Error] " << zErrMsg << "\nQuery: " << sqlQuery << std::endl;
        sqlite3_free(zErrMsg);
    }
    
    return result;
}

// Generic Execute (Create, Update, Drop, etc.)
bool dbManager::execute(std::string sqlCommand) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sqlCommand.c_str(), 0, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "[DB Exec Error] " << zErrMsg << "\nCommand: " << sqlCommand << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    return true;
}