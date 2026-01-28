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
    int class_id;               ///< Predicted class (0=Deficiency, 1=Disease, 2=Healthy, 3=Pest)
    std::string class_name;     ///< Human-readable class name
    float confidence;           ///< Confidence score (0.0 - 1.0)
    std::vector<float> probs;   ///< Probabilities for all classes
    bool isValidPlant;          ///< Whether image appears to be a valid plant (OOD detection)
    float entropy;              ///< Shannon entropy of probability distribution (lower = more confident)
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
    
    // Out-of-distribution detection thresholds
    static constexpr float ENTROPY_THRESHOLD = 1.8f;      ///< Max entropy for valid plant (log2(4)=2.0 is uniform)
    static constexpr float MIN_CONFIDENCE_THRESHOLD = 0.3f; ///< Minimum confidence to be valid
    static constexpr float MIN_GREEN_RATIO = 0.10f;       ///< Minimum green pixel ratio (10% for lettuce)
    
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
    
    /**
     * @brief Check if image contains sufficient green/plant-like colors
     * @param imagePath Path to image file
     * @return Ratio of green pixels (0.0 to 1.0)
     */
    float checkGreenRatio(const std::string& imagePath);
    
    /**
     * @brief Calculate Shannon entropy of probability distribution
     * @param probs Probability distribution
     * @return Entropy value (0 = certain, log2(N) = uniform/uncertain)
     */
    float calculateEntropy(const std::vector<float>& probs);
    
    /**
     * @brief Check if prediction indicates a valid plant image
     * @param entropy Calculated entropy
     * @param maxConfidence Maximum class probability
     * @param greenRatio Ratio of green pixels in image
     * @return true if likely a valid plant image
     */
    bool checkValidPlant(float entropy, float maxConfidence, float greenRatio);

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