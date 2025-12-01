/**
 * @file PH.cpp
 * @brief Implementation of pH Sensor Driver (Mock Mode)
 */

#include "PH.h"
#include <cstdlib>

/* ============================================================================
 * Sensor Reading (Mock Implementation)
 * ============================================================================ */

float PH::readSensor() 
{
    // Mock: Returns random pH between 6.0 and 7.0
    float noise = (float)(rand() % 100) / 100.0;
    realValue = 6.0 + noise;
    return realValue;
}