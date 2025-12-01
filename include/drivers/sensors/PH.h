/**
 * @file PH.h
 * @brief pH Sensor Driver (Analog via ADC)
 * @layer Drivers/Sensors
 * 
 * Reads pH from an analog probe connected to an ADC channel.
 * Converts voltage to pH using calibration formula.
 */

#ifndef PH_H
#define PH_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include "Sensor.h"
#include "ADC.h"

/**
 * @class PH
 * @brief pH sensor driver (analog probe)
 * 
 * Real mode: Reads ADC voltage and converts to pH (0-14 scale)
 * Mock mode: Returns random values between 6.0-7.0
 */
class PH : public Sensor {
public:
    /**
     * @brief Constructs pH sensor
     * @param a Pointer to ADC driver
     * @param c ADC channel number (0-3)
     */
    PH(ADC* a, int c) {}
    
    /**
     * @brief Reads current pH value
     * @return pH level (0.0 - 14.0)
     */
    float readSensor() override;
};

#endif // PH_H