/**
 * @file IdealConditions.cpp
 * @brief Implementation of Ideal Sensor Value Range Storage
 */

#include "IdealConditions.h"

/* ============================================================================
 * Constructor
 * ============================================================================ */

IdealConditions::IdealConditions() 
{
    // Default values for Lettuce (Design Section 2.1.6)
    tds_min = 560.0;
    tds_max = 840.0;
    ph_min = 5.5;
    ph_max = 6.5;
    temp_min = 18.0;
    temp_max = 24.0;
}

/* ============================================================================
 * Getters (Array-based for efficiency)
 * ============================================================================ */

void IdealConditions::getTDS(float* range) 
{
    range[0] = tds_min;
    range[1] = tds_max;
}

void IdealConditions::getPH(float* range) 
{
    range[0] = ph_min;
    range[1] = ph_max;
}

void IdealConditions::getTemp(float* range) 
{
    range[0] = temp_min;
    range[1] = temp_max;
}

/* ============================================================================
 * Setters
 * ============================================================================ */

void IdealConditions::setTDS(float min, float max) 
{
    tds_min = min;
    tds_max = max;
}

void IdealConditions::setPH(float min, float max) 
{
    ph_min = min;
    ph_max = max;
}

void IdealConditions::setTemp(float min, float max) 
{
    temp_min = min;
    temp_max = max;
}