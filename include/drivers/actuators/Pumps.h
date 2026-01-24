/**
 * @file Pumps.h
 * @brief Peristaltic Pump Driver
 * @layer Drivers/Actuators
 * 
 * Controls peristaltic pumps for dosing solutions.
 * Used for pH Up, pH Down, and Nutrient delivery.
 * Uses libgpiod for GPIO control.
 */

#ifndef PUMPS_H
#define PUMPS_H

#include <gpiod.h>

/**
 * @class Pumps
 * @brief GPIO-controlled peristaltic pump using libgpiod
 * 
 * Real mode: Activates GPIO pin via libgpiod
 * Mock mode: Prints dosing action to console
 */
class Pumps {
private:
    int gpioPin;                   ///< GPIO pin number for pump control
    bool state;                    ///< Current pump state (true = ON)
    bool initialized;              ///< Whether GPIO was successfully initialized
    struct gpiod_chip *chip;       ///< GPIO chip handle
    struct gpiod_line *line;       ///< GPIO line handle

public:
    /**
     * @brief Constructs pump driver
     * @param pin GPIO pin number (BCM numbering)
     */
    Pumps(int pin);
    
    /**
     * @brief Destructor - turns off pump and releases GPIO
     */
    ~Pumps();
    
    /**
     * @brief Sets pump state
     * @param on true to turn on, false to turn off
     */
    void pump(bool on);

    /**
     * @brief Gets current pump state
     * @return true if pump is ON
     */
    bool getState() { return state; }
};

#endif // PUMPS_H