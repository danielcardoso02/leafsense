/**
 * @file ADC.cpp
 * @brief Implementation of ADS1115 ADC Driver (Mock Mode)
 */

#include "ADC.h"
#include <cstdlib>
#include <iostream>

/* ============================================================================
 * Constructor
 * ============================================================================ */

ADC::ADC(int addr) 
{
    // Real mode: Initialize I2C connection at specified address
    // Mock mode: No initialization needed
}

/* ============================================================================
 * Voltage Reading (Mock Implementation)
 * ============================================================================ */

float ADC::readVoltage(int channel) 
{
    // Mock: Generate random voltage between 0.0V and 3.0V
    // This allows PH and TDS classes to calculate values from mock voltage
    float noise = (float)(rand() % 300) / 100.0;
    return noise;
}