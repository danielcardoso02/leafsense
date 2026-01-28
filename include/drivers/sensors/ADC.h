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
 * @brief ADS1115 ADC driver with I2C support
 * 
 * Real mode: Communicates via I2C at specified address (/dev/i2c-1)
 * Mock mode: Returns random voltage values (0.0-3.0V) if I2C unavailable
 */
class ADC {
private:
    int i2cAddr;       ///< I2C slave address (typically 0x48)
    int i2cFd;         ///< I2C file descriptor
    bool initialized;  ///< True if I2C connection is active

public:
    /**
     * @brief Constructs ADC driver and initializes I2C
     * @param addr I2C address (typically 0x48)
     */
    ADC(int addr);
    
    /**
     * @brief Destructor - closes I2C connection
     */
    ~ADC();
    
    /**
     * @brief Reads voltage from specified channel
     * @param ch Channel number (0-3)
     * @return Voltage in volts (0.0 - 4.096V range)
     */
    float readVoltage(int ch);
    
    /**
     * @brief Checks if I2C is initialized
     * @return true if I2C connection is active
     */
    bool isInitialized() { return initialized; }
};

#endif // ADC_H