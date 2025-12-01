/**
 * @file TDS.cpp
 * @brief Implementation of TDS/EC Sensor Driver (Mock Mode)
 */

#include "TDS.h"
#include <cstdlib>

/* ============================================================================
 * Sensor Reading (Mock Implementation)
 * ============================================================================ */

float TDS::readSensor() 
{
    // Mock: Returns random EC/TDS around 1200-1400 ppm
    realValue = 1200.0 + (rand() % 200);
    return realValue;
}