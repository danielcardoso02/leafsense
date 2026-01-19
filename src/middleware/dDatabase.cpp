/**
 * @file dDatabase.cpp
 * @brief Implementation of the Database Daemon Logic
 */

#include "../../include/middleware/dDatabase.h"
#include <iostream>

dDatabase::dDatabase(MQueueHandler* queue, std::string dbInfo) 
    : incomingQueue(queue), running(true) {
    // Initialize DB Manager
    db = new dbManager(dbInfo);
}

dDatabase::~dDatabase() {
    delete db;
}

void dDatabase::stop() {
    running = false;
    if (incomingQueue) {
        incomingQueue->sendMessage("EXIT");
    }
}

// Helper to split strings for parsing protocol
std::vector<std::string> dDatabase::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// The Translation Logic ("Convert MQueue to SQL command")
std::string dDatabase::translateToSQL(std::string rawMessage) {
    // Protocol Format: TAG|DATA1|DATA2...
    std::vector<std::string> parts = split(rawMessage, '|');
    
    if (parts.empty()) return "";

    std::string tag = parts[0];
    std::stringstream sql;

    if (tag == "SENSOR" && parts.size() >= 4) {
        // Format: SENSOR|TEMP|PH|EC
        // Schema: sensor_readings (temperature, ph, ec)
        sql << "INSERT INTO sensor_readings (temperature, ph, ec) VALUES ("
            << parts[1] << ", " << parts[2] << ", " << parts[3] << ");";
            
    } else if (tag == "LOG" && parts.size() >= 4) {
        // Format: LOG|TYPE|MESSAGE|DETAILS
        // Schema: logs (log_type, message, details)
        sql << "INSERT INTO logs (log_type, message, details) VALUES ('"
            << parts[1] << "', '" << parts[2] << "', '" << parts[3] << "');";
            
    } else if (tag == "ALERT" && parts.size() >= 3) {
        // Format: ALERT|TYPE|MESSAGE
        // Schema: alerts (type, message)
        sql << "INSERT INTO alerts (type, message) VALUES ('"
            << parts[1] << "', '" << parts[2] << "');";
            
    } else if (tag == "IMG" && parts.size() >= 3) {
        // Format: IMG|FILENAME|PATH
        // Schema: plant_images (filename, filepath)
        sql << "INSERT INTO plant_images (filename, filepath) VALUES ('"
            << parts[1] << "', '" << parts[2] << "');";
            
    } else if (tag == "PRED" && parts.size() >= 4) {
        // Format: PRED|FILENAME|LABEL|CONFIDENCE
        // Schema: ml_predictions (image_id, prediction_type, prediction_label, confidence)
        // Link to plant_images via subquery on filename
        sql << "INSERT INTO ml_predictions (image_id, prediction_type, prediction_label, confidence) "
            << "SELECT id, '" << parts[2] << "', '" << parts[2] << "', " << parts[3] 
            << " FROM plant_images WHERE filename = '" << parts[1] << "' "
            << "ORDER BY id DESC LIMIT 1;";
    } else {
        std::cerr << "[Daemon] Unknown message format: " << rawMessage << std::endl;
        return "";
    }

    return sql.str();
}

// The Event Loop
void dDatabase::run() {
    std::cout << "[Daemon] Database Service Started." << std::endl;

    while (running) {
        // 1. Wait for Message (Blocking Call)
        std::string msg = incomingQueue->receiveMessage();

        // FIX: Check for the Exit Signal immediately
        if (msg == "EXIT") break; 
        if (msg.empty()) continue;

        // 2. Translation Layer
        std::string sqlCommand = translateToSQL(msg);

        // 3. Execution Layer
        if (!sqlCommand.empty()) {
            bool success = db->insert(sqlCommand);
            if (success) {
                std::cout << "[Daemon] SUCCESS - Inserted: " << msg << std::endl;
            } else {
                std::cerr << "[Daemon] FAILED to insert: " << msg << std::endl;
                std::cerr << "[Daemon] SQL: " << sqlCommand << std::endl;
            }
        }
    }
    
    std::cout << "[Daemon] Database Service Stopped." << std::endl;
}