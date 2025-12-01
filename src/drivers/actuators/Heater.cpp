/**
 * @file Heater.cpp
 * @brief Implementation of Heater Control Driver (Mock Mode)
 */

#include "Heater.h"
#include <iostream>

/* ============================================================================
 * Heater Control (Mock Implementation)
 * ============================================================================ */

void Heater::setState(bool on) 
{
    state = on;
    // Mock: Output heater state change to console
    std::cout << "[Heater] " << (on ? "ON" : "OFF") << std::endl;
}