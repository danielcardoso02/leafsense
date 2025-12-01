/**
 * @file Sensor.cpp
 * @brief Implementation of the base Sensor class
 * 
 * Provides common functionality for all sensor types including value
 * storage and correction timing control.
 */

#include "../include/drivers/sensors/Sensor.h"

/* ============================================================================
 * Construction / Destruction
 * ============================================================================ */

Sensor::Sensor() 
    : realValue(0.0)
    , correcting(false) 
{
}

Sensor::~Sensor() 
{
}

/* ============================================================================
 * Value Access
 * ============================================================================ */

/**
 * @brief Gets the last read sensor value
 */
float Sensor::getValue() 
{
    return realValue;
}

/* ============================================================================
 * Timing Control
 * ============================================================================ */

/**
 * @brief Sets the correction timing flag
 * 
 * Used by Master to indicate when corrections are in progress,
 * allowing sensors to adjust their reading frequency.
 */
void Sensor::setTime(bool isCorrecting) 
{
    correcting = isCorrecting;
}