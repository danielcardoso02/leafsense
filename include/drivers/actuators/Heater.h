/**
 * @file Heater.h
 * @brief Water Heater Control Driver
 * @layer Drivers/Actuators
 * 
 * Controls the water heater relay for temperature management.
 * Implements simple on/off control with state tracking.
 */

#ifndef HEATER_H
#define HEATER_H

/**
 * @class Heater
 * @brief GPIO-controlled water heater relay
 * 
 * Real mode: Controls relay via GPIO pin
 * Mock mode: Prints state changes to console
 */
class Heater {
private:
    bool state;  ///< Current heater state (true = ON)

public:
    /**
     * @brief Constructs heater driver
     * @param pin GPIO pin number (BCM numbering)
     */
    Heater(int pin) : state(false) {}
    
    /**
     * @brief Sets heater state
     * @param on true to turn on, false to turn off
     */
    void setState(bool on);
    
    /**
     * @brief Gets current heater state
     * @return true if heater is ON
     */
    bool getState() { return state; }
};

#endif // HEATER_H