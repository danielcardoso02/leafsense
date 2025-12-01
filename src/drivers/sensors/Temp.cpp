/**
 * @file Temp.cpp
 * @brief Implementation of DS18B20 Temperature Sensor Driver (Mock Mode)
 */

#include "Temp.h"
#include <cstdlib>

/* ============================================================================
 * Sensor Reading (Mock Implementation)
 * ============================================================================ */

float Temp::readSensor() 
{
    // Mock: Returns random temperature between 20.0 and 25.0°C
    float noise = (float)(rand() % 50) / 10.0;
    realValue = 20.0 + noise;
    return realValue;
}
