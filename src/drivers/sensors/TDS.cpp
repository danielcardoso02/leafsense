/**
 * @file TDS.cpp
 * @brief Implementation of TDS/EC Sensor Driver with ADC I2C Support
 * 
 * Uses ADC channel to read analog TDS probe voltage.
 * Falls back to mock mode if ADC is not available.
 */

#include "TDS.h"
#include <cstdlib>
#include <iostream>

/* ============================================================================
 * Sensor Reading with ADC Support
 * ============================================================================ */

float TDS::readSensor() 
{
    if (adc) {
        // Read voltage from ADC channel
        float voltage = adc->readVoltage(channel);
        
        // Check if ADC read failed (returns -1.0 on error)
        if (voltage < 0.0f) {
            // Fallback to mock mode on ADC error
            realValue = 1200.0 + (rand() % 200);
            std::cout << "[TDS] ADC error, mock mode: " << realValue << "ppm" << std::endl;
            return realValue;
        }
        
        // Convert voltage to TDS/EC (ppm)
        // Typical TDS probe: 0V = 0ppm, 2.3V = ~1000ppm (linear approximation)
        // TDS = voltage * (1000 / 2.3) ≈ voltage * 435
        // Adjusted for typical hydroponics range (500-2000 ppm)
        realValue = voltage * 435.0;
        
        // Clamp to reasonable range
        if (realValue < 0.0) realValue = 0.0;
        if (realValue > 5000.0) realValue = 5000.0;
        
        std::cout << "[TDS] Channel " << channel 
                  << ": Voltage=" << voltage << "V, EC=" << realValue << "ppm" 
                  << std::endl;
        
        return realValue;
    }
    
    // Mock mode: Returns random EC/TDS around 1200-1400 ppm
    realValue = 1200.0 + (rand() % 200);
    std::cout << "[TDS] Mock mode: " << realValue << "ppm" << std::endl;
    return realValue;
}