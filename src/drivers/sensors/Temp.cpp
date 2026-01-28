/**
 * @file Temp.cpp
 * @brief Implementation of DS18B20 Temperature Sensor Driver with 1-Wire Support
 * 
 * Reads temperature from DS18B20 sensor via Linux 1-Wire interface.
 * Device path: /sys/bus/w1/devices/28-XXXX/w1_slave
 * Falls back to mock mode if sensor not available.
 * 
 * Hardware Setup:
 * - Data pin: GPIO 4 (with 4.7kOhm pull-up to 3.3V)
 * - Enable: dtoverlay=w1-gpio,gpiopin=4 in /boot/config.txt
 */

#include "Temp.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>

// 1-Wire device path
static const char* W1_DEVICES_PATH = "/sys/bus/w1/devices/";
static const char* W1_SLAVE_FILE = "/w1_slave";

/* ============================================================================
 * Helper: Find DS18B20 device path
 * ============================================================================ */

static std::string findDS18B20Device() 
{
    DIR* dir = opendir(W1_DEVICES_PATH);
    if (!dir) {
        return "";
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // DS18B20 devices start with "28-"
        if (strncmp(entry->d_name, "28-", 3) == 0) {
            std::string path = std::string(W1_DEVICES_PATH) + entry->d_name + W1_SLAVE_FILE;
            closedir(dir);
            return path;
        }
    }
    
    closedir(dir);
    return "";
}

/* ============================================================================
 * Sensor Reading with 1-Wire Support
 * ============================================================================ */

float Temp::readSensor() 
{
    // Try to find DS18B20 device
    std::string devicePath = findDS18B20Device();
    
    if (!devicePath.empty()) {
        std::ifstream file(devicePath);
        if (file.is_open()) {
            std::string line;
            
            // Read first line - check CRC
            std::getline(file, line);
            if (line.find("YES") == std::string::npos) {
                std::cerr << "[Temp] CRC check failed" << std::endl;
                file.close();
                goto mock_mode;
            }
            
            // Read second line - contains temperature
            std::getline(file, line);
            file.close();
            
            // Find "t=" and extract temperature
            size_t pos = line.find("t=");
            if (pos != std::string::npos) {
                int rawTemp = std::stoi(line.substr(pos + 2));
                realValue = rawTemp / 1000.0f;  // Convert from millidegrees
                
                std::cout << "[Temp] DS18B20: " << realValue << "°C" 
                          << std::endl;
                
                return realValue;
            }
        }
    }

mock_mode:
    // Mock: Returns random temperature between 15.0 and 25.0°C
    float noise = (float)(rand() % 100) / 10.0f;  // 0.0 to 10.0
    realValue = 15.0f + noise;  // 15.0 to 25.0°C
    
    std::cout << "[Temp] Mock mode: " << realValue << "°C" 
              << std::endl;
    
    return realValue;
}
