/**
 * @file Pumps.cpp
 * @brief Implementation of Peristaltic Pump Driver via libgpiod
 * 
 * Controls relay connected to GPIO pins for pump control.
 * Uses the libgpiod library for GPIO access.
 */

#include "Pumps.h"
#include <iostream>

/* ============================================================================
 * Construction / Destruction
 * ============================================================================ */

Pumps::Pumps(int pin) 
    : gpioPin(pin), state(false), initialized(false), chip(nullptr), line(nullptr) 
{
    // Open GPIO chip (gpiochip0 for Raspberry Pi)
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "[Pump GPIO" << gpioPin << "] WARNING: Cannot open gpiochip0, running in mock mode" << std::endl;
        return;
    }
    
    // Get the GPIO line
    line = gpiod_chip_get_line(chip, gpioPin);
    if (!line) {
        std::cerr << "[Pump GPIO" << gpioPin << "] WARNING: Cannot get GPIO line, running in mock mode" << std::endl;
        gpiod_chip_close(chip);
        chip = nullptr;
        return;
    }
    
    // Request line as output with initial value LOW (pump off)
    int ret = gpiod_line_request_output(line, "leafsense-pump", 0);
    if (ret < 0) {
        std::cerr << "[Pump GPIO" << gpioPin << "] WARNING: Cannot request GPIO as output, running in mock mode" << std::endl;
        gpiod_chip_close(chip);
        chip = nullptr;
        line = nullptr;
        return;
    }
    
    initialized = true;
    std::cout << "[Pump GPIO" << gpioPin << "] Initialized successfully (libgpiod)" << std::endl;
}

Pumps::~Pumps() 
{
    // Ensure pump is off when application closes
    pump(false);
    
    // Release resources
    if (line) {
        gpiod_line_release(line);
    }
    if (chip) {
        gpiod_chip_close(chip);
        std::cout << "[Pump GPIO" << gpioPin << "] Released" << std::endl;
    }
}

/* ============================================================================
 * Pump Control
 * ============================================================================ */

void Pumps::pump(bool on) 
{
    state = on;
    
    if (initialized && line) {
        // Real GPIO control via libgpiod
        int ret = gpiod_line_set_value(line, on ? 1 : 0);
        if (ret < 0) {
            std::cerr << "[Pump GPIO" << gpioPin << "] Error setting value" << std::endl;
        } else {
            std::cout << "[Pump GPIO" << gpioPin << "] -> " << (on ? "HIGH (ON)" : "LOW (OFF)") << std::endl;
        }
    } else {
        // Mock mode fallback
        std::cout << "[Pump GPIO" << gpioPin << "] (MOCK) " << (on ? "ON" : "OFF") << std::endl;
    }
}