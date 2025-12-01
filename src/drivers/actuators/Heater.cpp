#include "Heater.h"
#include <iostream>

// Mock: Just prints to console
void Heater::setState(bool on) {
    state = on;
    std::cout << "[Heater] " << (on ? "ON" : "OFF") << std::endl;
}