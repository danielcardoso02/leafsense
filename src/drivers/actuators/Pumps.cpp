#include "Pumps.h"
#include <iostream>

// Mock: Just prints to console
void Pumps::pump(unsigned int duration) {
    std::cout << "[Pump " << gpioPin << "] Dosing for " << duration << "ms" << std::endl;
}