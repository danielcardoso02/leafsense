/**
 * @file AlertLed.cpp
 * @brief Implementation of Alert LED Driver (Kernel Module Wrapper)
 * 
 * Communicates with the custom Linux kernel module via the
 * /dev/led0 character device to control the physical alert LED.
 */

#include "drivers/actuators/AlertLed.h"
#include <iostream>
#include <fcntl.h>   // For open()
#include <unistd.h>  // For write(), close()
#include <cstring>   // For strlen

/* ============================================================================
 * Construction / Destruction
 * ============================================================================ */

AlertLed::AlertLed(std::string path) 
    : devicePath(path) 
{
    // Device file is opened on-demand to allow app startup
    // even if the kernel module isn't loaded yet
}

AlertLed::~AlertLed() 
{
    // Ensure LED is off when application closes
    turnOff();
}

/* ============================================================================
 * LED Control
 * ============================================================================ */

void AlertLed::turnOn() 
{
    writeToDriver("1");
}

void AlertLed::turnOff() 
{
    writeToDriver("0");
}

/* ============================================================================
 * Driver Communication
 * ============================================================================ */

/**
 * @brief Write a value to the kernel module device
 * 
 * Opens the device file, writes the command, and closes it.
 * Errors are logged to stderr if the driver is not available.
 * 
 * @param val Command string ("0" = OFF, "1" = ON)
 */
void AlertLed::writeToDriver(const char* val) 
{
    // Open the device driver file (write only)
    int fd = open(devicePath.c_str(), O_WRONLY);
    
    if (fd < 0) {
        std::cerr << "[AlertLed] Error: Cannot open driver at " << devicePath 
                  << ". Is the kernel module loaded?" << std::endl;
        return;
    }
    
    // Write the command (1 byte)
    ssize_t bytesWritten = write(fd, val, 1);
    
    if (bytesWritten < 0) {
        std::cerr << "[AlertLed] Error: Failed to write to driver." << std::endl;
    }
    
    // Close the device file
    close(fd);
}