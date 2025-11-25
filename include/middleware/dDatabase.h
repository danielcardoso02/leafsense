/**
 * @file dDatabase.h
 * @brief Database Daemon Worker Thread
 * @layer Middleware / Daemon Package
 */

#ifndef DDATABASE_H
#define DDATABASE_H

#include "MQueueHandler.h"
#include "dbManager.h"
#include <string>
#include <vector>
#include <sstream>

class dDatabase {
private:
    MQueueHandler* incomingQueue; // From Sensor Threads (mqueueToDB)
    dbManager* db;                // Interface to SQLite
    bool running;

    /**
     * @brief Helper: Splits a string by a delimiter
     * used to parse "TAG|DATA" messages
     */
    std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief The "Translation Layer" (Section 4.5.11)
     * Converts raw message strings into SQL commands
     */
    std::string translateToSQL(std::string rawMessage);

public:
    /**
     * @brief Constructor
     * @param queue Pointer to the shared message queue
     * @param dbInfo Database file path
     */
    dDatabase(MQueueHandler* queue, std::string dbInfo);

    ~dDatabase();

    /**
     * @brief The Main Event Loop (Figure 63)
     * Continuous loop: Receive -> Translate -> Execute
     */
    void run();
    
    // Method to stop the loop gracefully during Shutdown (Section 4.5.16)
    void stop();
};

#endif // DDATABASE_H