/**
 * @file Heater.cpp
 * @brief Implementation of Heater Control Driver via libgpiod
 * 
 * Controls a relay connected to GPIO pin 21 (default) for heater control.
 * Uses the libgpiod library for GPIO access.
 */

#include "Heater.h"
#include <iostream>

/* ============================================================================
 * Construction / Destruction
 * ============================================================================ */

Heater::Heater(int pin) 
    : state(false), gpioPin(pin), initialized(false), chip(nullptr), line(nullptr) 
{
    // Open GPIO chip (gpiochip0 for Raspberry Pi)
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "[Heater] WARNING: Cannot open gpiochip0, running in mock mode" << std::endl;
        return;
    }
    
    // Get the GPIO line
    line = gpiod_chip_get_line(chip, gpioPin);
    if (!line) {
        std::cerr << "[Heater] WARNING: Cannot get GPIO line " << gpioPin << ", running in mock mode" << std::endl;
        gpiod_chip_close(chip);
        chip = nullptr;
        return;
    }
    
    // Request line as output with initial value HIGH (heater off - inverted logic)
    int ret = gpiod_line_request_output(line, "leafsense-heater", 1);
    if (ret < 0) {
        std::cerr << "[Heater] WARNING: Cannot request GPIO " << gpioPin << " as output, running in mock mode" << std::endl;
        gpiod_chip_close(chip);
        chip = nullptr;
        line = nullptr;
        return;
    }
    
    initialized = true;
    std::cout << "[Heater] GPIO " << gpioPin << " initialized successfully (libgpiod)" << std::endl;
}

Heater::~Heater() 
{
    // Ensure heater is off when application closes
    setState(false);
    
    // Release resources
    if (line) {
        gpiod_line_release(line);
    }
    if (chip) {
        gpiod_chip_close(chip);
        std::cout << "[Heater] GPIO " << gpioPin << " released" << std::endl;
    }
}

/* ============================================================================
 * Heater State Control
 * ============================================================================ */

void Heater::setState(bool on) 
{
    state = on;
    
    if (initialized && line) {
        // Real GPIO control via libgpiod
        // Note: Inverted logic - GPIO LOW = Heater ON, GPIO HIGH = Heater OFF
        int ret = gpiod_line_set_value(line, on ? 0 : 1);
        if (ret < 0) {
            std::cerr << "[Heater] Error setting GPIO " << gpioPin << " value" << std::endl;
        } else {
            std::cout << "[Heater] GPIO " << gpioPin << " -> " << (on ? "LOW (ON)" : "HIGH (OFF)") << std::endl;
        }
    } else {
        // Mock mode fallback
        std::cout << "[Heater] (MOCK) " << (on ? "ON" : "OFF") << std::endl;
    }
}