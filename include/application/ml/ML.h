/**
 * @file ML.h
 * @brief Machine Learning Inference Engine
 * @layer Application/ML
 * 
 * Performs plant disease detection using ONNX model inference.
 * Analyzes plant images to detect diseases and deficiencies.
 */

#ifndef ML_H
#define ML_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include <string>

/**
 * @class ML
 * @brief Plant disease detection via ML inference
 * 
 * Real mode: Loads ONNX model and performs inference
 * Mock mode: Returns 0 (Healthy) for all images
 * 
 * Return codes:
 * - 0: Healthy
 * - 1: Powdery Mildew
 * - 2: Downy Mildew
 * - 3: Nutrient Deficiency
 */
class ML {
public:
    /**
     * @brief Constructs ML engine
     * @param d ONNX model directory path
     * @param n Model filename
     */
    ML(std::string d, std::string n) {}
    
    /**
     * @brief Analyzes image for plant diseases
     * @param imgPath Path to image file
     * @return Disease code (0 = Healthy)
     */
    unsigned int analyze(std::string imgPath);
};

#endif // ML_H