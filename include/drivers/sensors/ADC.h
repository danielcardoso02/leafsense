/**
 * @file ADC.h
 * @brief ADS1115 Analog-to-Digital Converter Driver
 * @layer Drivers/Sensors
 * 
 * Provides I2C interface to ADS1115 16-bit ADC.
 * Used by pH and TDS sensors for analog readings.
 */

#ifndef ADC_H
#define ADC_H

/**
 * @class ADC
 * @brief ADS1115 ADC driver
 * 
 * Real mode: Communicates via I2C at specified address
 * Mock mode: Returns random voltage values (0.0-3.0V)
 */
class ADC {
public:
    /**
     * @brief Constructs ADC driver
     * @param addr I2C address (typically 0x48)
     */
    ADC(int addr);
    
    /**
     * @brief Reads voltage from specified channel
     * @param ch Channel number (0-3)
     * @return Voltage in volts (0.0 - 3.3V)
     */
    float readVoltage(int ch);
};

#endif // ADC_H