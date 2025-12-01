/**
 * @file ML.cpp
 * @brief Implementation of Machine Learning Analysis Module (Mock Mode)
 * 
 * Provides plant health classification based on camera images.
 * Uses MobileNetV3 Small ONNX model for inference in real mode.
 */

#include "ML.h"

/* ============================================================================
 * Image Analysis (Mock Implementation)
 * ============================================================================ */

/**
 * @brief Analyzes plant image for health classification
 * @param imagePath Path to the image file
 * @return Health status code (0 = Healthy)
 */
unsigned int ML::analyze(std::string imagePath) 
{
    // Mock: Returns 0 (Healthy) without loading heavy ONNX models
    return 0; 
}