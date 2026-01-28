/**
 * @file ADC.cpp
 * @brief Implementation of ADS1115 ADC Driver with I2C support
 * 
 * Uses Linux I2C interface (/dev/i2c-1) to communicate with ADS1115.
 * Falls back to mock mode if I2C is not available.
 */

#include "ADC.h"
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cerrno>

/* ============================================================================
 * Constructor
 * ============================================================================ */

ADC::ADC(int addr) 
    : i2cAddr(addr), i2cFd(-1), initialized(false)
{
    // Try to open I2C bus
    i2cFd = open("/dev/i2c-1", O_RDWR);
    if (i2cFd < 0) {
        std::cerr << "[ADC] Cannot open /dev/i2c-1: " << strerror(errno) << std::endl;
        std::cout << "[ADC] Running in MOCK mode" << std::endl;
        return;
    }
    
    // Set I2C slave address
    if (ioctl(i2cFd, I2C_SLAVE, i2cAddr) < 0) {
        std::cerr << "[ADC] Cannot set I2C address 0x" << std::hex << i2cAddr << ": " << strerror(errno) << std::endl;
        close(i2cFd);
        i2cFd = -1;
        std::cout << "[ADC] Running in MOCK mode" << std::endl;
        return;
    }
    
    initialized = true;
    std::cout << "[ADC] I2C initialized successfully at address 0x" << std::hex << i2cAddr << std::dec << std::endl;
}

ADC::~ADC()
{
    if (i2cFd >= 0) {
        close(i2cFd);
        std::cout << "[ADC] I2C connection closed" << std::endl;
    }
}

/* ============================================================================
 * Voltage Reading
 * ============================================================================ */

float ADC::readVoltage(int channel) 
{
    if (!initialized) {
        // Mock mode: Generate random voltage between 0.0V and 3.0V
        float noise = (float)(rand() % 300) / 100.0;
        return noise;
    }
    
    // Configure ADS1115 for single-ended reading on specified channel
    // Config register (0x01):
    // Bit 15: OS = 1 (start conversion)
    // Bits 14-12: MUX = channel (000=AIN0, 001=AIN1, 010=AIN2, 011=AIN3)
    // Bits 11-9: PGA = 001 (±4.096V)
    // Bit 8: MODE = 1 (single-shot)
    // Bits 7-5: DR = 100 (128 SPS)
    // Bits 4-0: comparator disabled
    
    uint16_t config = 0x8000;  // OS bit
    config |= ((channel & 0x03) + 4) << 12;  // MUX: AINx vs GND
    config |= 0x0200;  // PGA: ±4.096V
    config |= 0x0100;  // Single-shot mode
    config |= 0x0080;  // 128 SPS
    
    uint8_t configBytes[3];
    configBytes[0] = 0x01;  // Config register
    configBytes[1] = (config >> 8) & 0xFF;
    configBytes[2] = config & 0xFF;
    
    // Write config
    if (write(i2cFd, configBytes, 3) != 3) {
        std::cerr << "[ADC] I2C write error on channel " << channel << std::endl;
        return -1.0f;
    }
    
    // Poll conversion ready bit (bit 15 of config register)
    // This avoids using usleep() - proper hardware polling
    uint8_t pollReg = 0x01;  // Config register
    uint8_t pollBuffer[2];
    int pollAttempts = 0;
    const int maxPollAttempts = 100;  // Timeout after ~100 iterations
    
    do {
        if (write(i2cFd, &pollReg, 1) != 1) break;
        if (read(i2cFd, pollBuffer, 2) != 2) break;
        pollAttempts++;
    } while (((pollBuffer[0] & 0x80) == 0) && (pollAttempts < maxPollAttempts));
    
    if (pollAttempts >= maxPollAttempts) {
        std::cerr << "[ADC] Conversion timeout on channel " << channel << std::endl;
        return -1.0f;
    }
    
    // Read conversion register (0x00)
    uint8_t regAddr = 0x00;
    if (write(i2cFd, &regAddr, 1) != 1) {
        std::cerr << "[ADC] I2C register select error" << std::endl;
        return -1.0f;
    }
    
    uint8_t readBuffer[2];
    if (read(i2cFd, readBuffer, 2) != 2) {
        std::cerr << "[ADC] I2C read error" << std::endl;
        return -1.0f;
    }
    
    // Convert to voltage (16-bit signed, ±4.096V range)
    int16_t rawValue = (readBuffer[0] << 8) | readBuffer[1];
    float voltage = (rawValue / 32768.0f) * 4.096f;
    
    std::cout << "[ADC] Channel " << channel << ": Raw=" << rawValue 
              << ", Voltage=" << voltage << "V" << std::endl;
    
    return voltage;
}