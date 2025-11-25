/**
 * @file dbManager.h
 * @brief Database Management Class for SQLite interactions
 * @layer Middleware
 */

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include <iostream>

// Structure to hold query results (Table representation)
struct DBResult {
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
};

class dbManager {
private:
    std::string info; // Database connection string (file path)
    sqlite3* db;      // SQLite connection handle

    /**
     * @brief Callback function used by sqlite3_exec to process query results
     */
    static int callback(void* data, int argc, char** argv, char** azColName);

public:
    /**
     * @brief Constructor: Initializes database connection
     * @param dbPath Path to the SQLite database file (e.g., "leafsense.db")
     */
    dbManager(std::string dbPath);

    /**
     * @brief Destructor: Closes database connection
     */
    ~dbManager();

    /**
     * @brief Establishes connection to the database
     * @return true if connected successfully, false otherwise
     */
    bool connect();

    /**
     * @brief Executes a Write operation (INSERT)
     * @param sqlCommand The SQL command string
     * @return true if successful
     */
    bool insert(std::string sqlCommand);

    /**
     * @brief Executes a Read operation (SELECT)
     * @param sqlQuery The SQL query string
     * @return DBResult structure containing headers and rows
     */
    DBResult read(std::string sqlQuery);

    /**
     * @brief Executes a Delete operation (DELETE)
     * @param sqlCommand The SQL command string
     * @return true if successful
     */
    bool remove(std::string sqlCommand);

    // Helper for executing non-query commands (Update, Create Table, etc.)
    bool execute(std::string sqlCommand);
};

#endif // DBMANAGER_H