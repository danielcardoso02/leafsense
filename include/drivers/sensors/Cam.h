/**
 * @file Cam.h
 * @brief USB Camera Driver for Plant Imaging
 * @layer Drivers/Sensors
 * 
 * Captures images from USB camera for ML disease detection.
 * Uses OpenCV for image capture in real mode.
 */

#ifndef CAM_H
#define CAM_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include <string>

/**
 * @class Cam
 * @brief USB camera driver
 * 
 * Real mode: Captures image using OpenCV VideoCapture
 * Mock mode: Returns path to a placeholder image
 */
class Cam {
public:
    /**
     * @brief Constructs camera driver
     */
    Cam() {}
    
    /**
     * @brief Captures a photo
     * @return Filepath to the captured image
     */
    std::string takePhoto();
};

#endif // CAM_H