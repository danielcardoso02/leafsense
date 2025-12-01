/**
 * @file Temp.h
 * @brief DS18B20 Temperature Sensor Driver
 * @layer Drivers/Sensors
 * 
 * Reads temperature from a DS18B20 1-Wire digital sensor.
 * In mock mode, returns simulated values for testing.
 */

#ifndef TEMP_H
#define TEMP_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include "Sensor.h"
#include <string>

/**
 * @class Temp
 * @brief Temperature sensor driver (DS18B20)
 * 
 * Real mode: Reads from /sys/bus/w1/devices/{addr}/temperature
 * Mock mode: Returns random values between 20.0-25.0°C
 */
class Temp : public Sensor {
public:
    /**
     * @brief Constructs temperature sensor
     * @param addr 1-Wire device address (e.g., "28-xxxx")
     */
    Temp(std::string addr) {}
    
    /**
     * @brief Reads current temperature
     * @return Temperature in degrees Celsius
     */
    float readSensor() override;
};

#endif // TEMP_H