/**
 * @file Cam.cpp
 * @brief Implementation of USB Camera Driver (Mock Mode)
 */

#include "Cam.h"

/* ============================================================================
 * Photo Capture (Mock Implementation)
 * ============================================================================ */

std::string Cam::takePhoto() 
{
    // Mock: Returns path to existing placeholder image
    return "resources/images/logo_leafsense.png";
}