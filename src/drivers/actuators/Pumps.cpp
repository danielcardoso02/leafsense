/**
 * @file Pumps.cpp
 * @brief Implementation of Peristaltic Pump Driver (Mock Mode)
 */

#include "Pumps.h"
#include <iostream>

/* ============================================================================
 * Pump Control (Mock Implementation)
 * ============================================================================ */

void Pumps::pump(unsigned int duration) 
{
    // Mock: Output pump activation to console
    std::cout << "[Pump " << gpioPin << "] Dosing for " << duration << "ms" << std::endl;
}