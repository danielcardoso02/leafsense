/**
 * @file AlertLed.h
 * @brief Alert LED Driver Interface (Kernel Module Wrapper)
 * 
 * Provides a C++ interface to the custom Linux kernel module
 * that controls the alert LED via /dev/led0 character device.
 */

#ifndef ALERT_LED_H
#define ALERT_LED_H

#include <string>

/* ============================================================================
 * AlertLed Class
 * ============================================================================ */

/**
 * @brief Wrapper class for the Custom Linux Kernel Module
 * 
 * Interacts with the character device /dev/led0 to control
 * the physical alert LED on the LeafSense system.
 */
class AlertLed 
{
private:
    std::string devicePath;  ///< Path to device node

    /**
     * @brief Internal helper to write data to the driver
     * @param val "0" for OFF, "1" for ON
     */
    void writeToDriver(const char* val);

public:
    /* ------------------------------------------------------------------------
     * Construction / Destruction
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructor
     * @param path Path to the device node (default: "/dev/led0")
     */
    AlertLed(std::string path = "/dev/led0");
    
    /**
     * @brief Destructor - ensures LED is turned off
     */
    ~AlertLed();

    /* ------------------------------------------------------------------------
     * LED Control
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Turn the alert LED ON
     */
    void turnOn();
    
    /**
     * @brief Turn the alert LED OFF
     */
    void turnOff();
};

#endif // ALERT_LED_H