/**
 * @file Cam.h
 * @brief Camera Driver for Plant Imaging (Raspberry Pi Camera Module)
 * @layer Drivers/Sensors
 * 
 * Captures images from Raspberry Pi Camera Module (OV5647) for ML disease detection.
 * Uses OpenCV VideoCapture for image acquisition.
 * Images are saved to /opt/leafsense/gallery/ with timestamp.
 */

#ifndef CAM_H
#define CAM_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include <string>

/**
 * @class Cam
 * @brief Raspberry Pi camera driver
 * 
 * Captures images from /dev/video0 (OV5647 camera module) using OpenCV.
 * Photos are automatically saved with timestamp to gallery directory.
 */
class Cam {
public:
    /**
     * @brief Constructs camera driver
     */
    Cam() {}
    
    /**
     * @brief Captures a photo from the camera
     * @return Filepath to the captured image, or empty string on failure
     * 
     * The image is saved to /opt/leafsense/gallery/plant_YYYYMMDD_HHMMSS.jpg
     * Returns empty string if camera cannot be opened or capture fails.
     */
    std::string takePhoto();
};

#endif // CAM_H