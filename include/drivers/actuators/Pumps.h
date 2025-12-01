/**
 * @file Pumps.h
 * @brief Peristaltic Pump Driver
 * @layer Drivers/Actuators
 * 
 * Controls peristaltic pumps for dosing solutions.
 * Used for pH Up, pH Down, and Nutrient delivery.
 */

#ifndef PUMPS_H
#define PUMPS_H

/**
 * @class Pumps
 * @brief GPIO-controlled peristaltic pump
 * 
 * Real mode: Activates GPIO pin for specified duration
 * Mock mode: Prints dosing action to console
 */
class Pumps {
private:
    int gpioPin;  ///< GPIO pin number for pump control

public:
    /**
     * @brief Constructs pump driver
     * @param pin GPIO pin number (BCM numbering)
     */
    Pumps(int pin) : gpioPin(pin) {}
    
    /**
     * @brief Activates pump for specified duration
     * @param duration Pump activation time in milliseconds
     */
    void pump(unsigned int duration);
};

#endif // PUMPS_H