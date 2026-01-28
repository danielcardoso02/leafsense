/**
 * @file Heater.h
 * @brief Water Heater Control Driver
 * @layer Drivers/Actuators
 * 
 * Controls the water heater relay for temperature management.
 * Uses libgpiod for GPIO control.
 */

#ifndef HEATER_H
#define HEATER_H

#include <string>
#include <gpiod.h>

/**
 * @class Heater
 * @brief GPIO-controlled water heater relay using libgpiod
 * 
 * Controls a relay via libgpiod library.
 * Default GPIO pin: 21 (BCM numbering)
 */
class Heater {
private:
    bool state;                    ///< Current heater state (true = ON)
    int gpioPin;                   ///< GPIO pin number (BCM)
    bool initialized;              ///< Whether GPIO was successfully initialized
    struct gpiod_chip *chip;       ///< GPIO chip handle
    struct gpiod_line *line;       ///< GPIO line handle

public:
    /**
     * @brief Constructs heater driver
     * @param pin GPIO pin number (BCM numbering), default 26
     */
    Heater(int pin = 26);
    
    /**
     * @brief Destructor - turns off heater and releases GPIO
     */
    ~Heater();
    
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
    
    /**
     * @brief Checks if GPIO was initialized successfully
     * @return true if ready to use
     */
    bool isInitialized() { return initialized; }
};

#endif // HEATER_H