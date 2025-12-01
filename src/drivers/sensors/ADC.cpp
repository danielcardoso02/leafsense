#include "ADC.h"
#include <cstdlib> // For rand()
#include <iostream>

ADC::ADC(int addr) {
    // In a real system: Initialize I2C connection here
    // For Simulation:
    // std::cout << "[ADC] Initialized at address " << std::hex << addr << std::endl;
}

float ADC::readVoltage(int channel) {
    // SIMULATION: Generate random voltage between 0.0V and 3.0V
    // This allows the PH and TDS classes to calculate "real" values from this mock voltage.
    
    float noise = (float)(rand() % 300) / 100.0; // 0.00 to 3.00
    return noise; 
}