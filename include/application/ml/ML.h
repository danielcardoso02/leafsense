/**
 * @file ML.h
 * @brief Machine Learning Inference Engine
 * @author Daniel Cardoso, Marco Costa
 * @layer Application/ML
 * 
 * Performs plant disease detection using ONNX Runtime inference.
 * Analyzes plant images to detect diseases and deficiencies.
 */

#ifndef ML_H
#define ML_H

/* ============================================================================
 * Includes
 * ============================================================================ */

#include <string>
#include <vector>
#include <memory>

// Forward declarations to avoid including heavy headers here
namespace Ort {
    class Env;
    class Session;
    class SessionOptions;
}

/* ============================================================================
 * ML Result Structure
 * ============================================================================ */

/**
 * @struct MLResult
 * @brief Holds the result of ML inference
 */
struct MLResult {
    int class_id;               ///< Predicted class (0=Healthy, 1=Deficiency, etc.)
    std::string class_name;     ///< Human-readable class name
    float confidence;           ///< Confidence score (0.0 - 1.0)
    std::vector<float> probs;   ///< Probabilities for all classes
};

/* ============================================================================
 * ML Class
 * ============================================================================ */

/**
 * @class ML
 * @brief Plant disease detection via ONNX Runtime inference
 * 
 * Return codes (alphabetical order from PyTorch ImageFolder):
 * - 0: Nutrient Deficiency
 * - 1: Disease
 * - 2: Healthy
 * - 3: Pest Damage
 */
class ML {
private:
    std::string modelPath;      ///< Full path to ONNX model
    bool initialized;           ///< Whether model loaded successfully
    
    // ONNX Runtime objects (opaque pointers to avoid header pollution)
    void* env;
    void* session;
    void* sessionOptions;
    
    static const int IMAGE_SIZE = 224;
    static const std::vector<std::string> CLASS_NAMES;
    
    /**
     * @brief Preprocess image for inference
     * @param imagePath Path to image file
     * @return Preprocessed tensor data (CHW format, normalized)
     */
    std::vector<float> preprocess(const std::string& imagePath);
    
    /**
     * @brief Apply softmax to convert logits to probabilities
     * @param logits Raw model output
     * @return Probability distribution
     */
    std::vector<float> softmax(const std::vector<float>& logits);

public:
    /**
     * @brief Constructs ML engine and loads model
     * @param dir Model directory path
     * @param name Model filename
     */
    ML(std::string dir, std::string name);
    
    /**
     * @brief Destructor - releases ONNX resources
     */
    ~ML();
    
    /**
     * @brief Check if model loaded successfully
     * @return true if ready for inference
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Analyzes image for plant diseases (legacy interface)
     * @param imagePath Path to image file
     * @return Disease code (0 = Healthy)
     */
    unsigned int analyze(std::string imagePath);
    
    /**
     * @brief Analyzes image with detailed results
     * @param imagePath Path to image file
     * @return MLResult with class, confidence, and probabilities
     */
    MLResult analyzeDetailed(const std::string& imagePath);
};

#endif // ML_H