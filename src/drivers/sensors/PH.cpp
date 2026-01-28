/**
 * @file PH.cpp
 * @brief Implementation of pH Sensor Driver with ADC I2C Support
 * 
 * Uses ADC channel to read analog pH probe voltage.
 * Calibration is based on standard pH 4.0 and pH 7.0 buffer solutions.
 * Falls back to mock mode if ADC is not available.
 * 
 * Calibration Procedure:
 * 1. Put probe in pH 7.0 buffer, record voltage (V7)
 * 2. Put probe in pH 4.0 buffer, record voltage (V4)
 * 3. Calculate: slope = (4.0 - 7.0) / (V4 - V7)
 * 4. Calculate: offset = 7.0 - (slope * V7)
 */

#include "PH.h"
#include <cstdlib>
#include <iostream>

/* ============================================================================
 * Calibration Constants (adjust after calibration)
 * ============================================================================ */

// Default calibration values for PH-4502C probe
// These should be updated after calibration with buffer solutions
static const float PH_CALIBRATION_OFFSET = 21.34f;  // Offset at pH 7.0
static const float PH_CALIBRATION_SLOPE = -5.70f;   // Slope (negative for typical probes)

// Alternative simple calibration (voltage at neutral pH 7.0)
// Calibrated for current sensor: ~1.03V measured at neutral pH
static const float PH_NEUTRAL_VOLTAGE = 1.03f;      // Voltage at pH 7.0 (measured)
static const float PH_VOLTAGE_PER_PH = 0.18f;       // ~0.18V per pH unit (Nernst equation)

/* ============================================================================
 * Sensor Reading with ADC Support
 * ============================================================================ */

float PH::readSensor() 
{
    if (adc) {
        // Read voltage from ADC channel
        float voltage = adc->readVoltage(channel);
        
        // Check if ADC read failed (returns -1.0 on error)
        if (voltage < 0.0f) {
            // Fallback to mock mode on ADC error
            float noise = (float)(rand() % 100) / 100.0f;
            realValue = 6.0f + noise;
            std::cout << "[pH] ADC error, mock mode: " << realValue << std::endl;
            return realValue;
        }
        
        // Convert voltage to pH using calibration
        // Method 1: Linear calibration (slope/offset from buffer solutions)
        // realValue = PH_CALIBRATION_SLOPE * voltage + PH_CALIBRATION_OFFSET;
        
        // Method 2: Nernst-based calculation
        // pH probes output ~2.5V at pH 7.0, with ~59mV change per pH unit at 25°C
        // Higher voltage = more acidic (lower pH)
        // Lower voltage = more alkaline (higher pH)
        realValue = 7.0f + ((PH_NEUTRAL_VOLTAGE - voltage) / PH_VOLTAGE_PER_PH);
        
        // Clamp to valid pH range
        if (realValue < 0.0f) realValue = 0.0f;
        if (realValue > 14.0f) realValue = 14.0f;
        
        std::cout << "[pH] Channel " << channel 
                  << ": Voltage=" << voltage << "V, pH=" << realValue 
                  << std::endl;
        
        return realValue;
    }
    
    // Mock mode: Returns random pH between 6.0 and 7.0
    float noise = (float)(rand() % 100) / 100.0f;
    realValue = 6.0f + noise;
    std::cout << "[pH] Mock mode: " << realValue << std::endl;
    return realValue;
}