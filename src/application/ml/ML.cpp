/**
 * @file ML.cpp
 * @brief Implementation of Machine Learning Inference Engine
 * 
 * Provides plant health classification based on camera images
 * using MobileNetV3-Small ONNX model with ONNX Runtime.
 */

#include "ML.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <numeric>

// OpenCV for image loading and preprocessing
#include <opencv2/opencv.hpp>

// ONNX Runtime
#include <onnxruntime_cxx_api.h>

/* ============================================================================
 * Static Members
 * ============================================================================ */

// Class names must match the order from training (alphabetical order)
// PyTorch ImageFolder sorts folders alphabetically:
//   0: deficiency -> Nutrient Deficiency
//   1: disease    -> Disease
//   2: healthy    -> Healthy
//   3: pest       -> Pest Damage
const std::vector<std::string> ML::CLASS_NAMES = {
    "Nutrient Deficiency",
    "Disease",
    "Healthy",
    "Pest Damage"
};

/* ============================================================================
 * Construction / Destruction
 * ============================================================================ */

ML::ML(std::string dir, std::string name)
    : initialized(false)
    , env(nullptr)
    , session(nullptr)
    , sessionOptions(nullptr)
{
    // Build full model path
    modelPath = dir + "/" + name;
    
    // Check if model file exists
    std::ifstream f(modelPath);
    if (!f.good()) {
        std::cerr << "[ML] Warning: Model file not found: " << modelPath << std::endl;
        std::cerr << "[ML] Running in mock mode (always returns Healthy)" << std::endl;
        return;
    }
    f.close();
    
    try {
        // Create ONNX Runtime environment
        Ort::Env* ortEnv = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "LeafSenseML");
        env = ortEnv;
        
        // Configure session options
        Ort::SessionOptions* ortOptions = new Ort::SessionOptions();
        ortOptions->SetIntraOpNumThreads(2);
        ortOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sessionOptions = ortOptions;
        
        // Load model
        Ort::Session* ortSession = new Ort::Session(*ortEnv, modelPath.c_str(), *ortOptions);
        session = ortSession;
        
        initialized = true;
        std::cout << "[ML] Model loaded successfully: " << modelPath << std::endl;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "[ML] Failed to load ONNX model: " << e.what() << std::endl;
        std::cerr << "[ML] Running in mock mode (always returns Healthy)" << std::endl;
        initialized = false;
    }
}

ML::~ML()
{
    if (session) {
        delete static_cast<Ort::Session*>(session);
    }
    if (sessionOptions) {
        delete static_cast<Ort::SessionOptions*>(sessionOptions);
    }
    if (env) {
        delete static_cast<Ort::Env*>(env);
    }
}

/* ============================================================================
 * Image Preprocessing
 * ============================================================================ */

std::vector<float> ML::preprocess(const std::string& imagePath)
{
    // Load image with OpenCV
    cv::Mat image = cv::imread(imagePath);
    
    if (image.empty()) {
        std::cerr << "[ML] Failed to load image: " << imagePath << std::endl;
        return std::vector<float>();
    }
    
    cv::Mat resized, rgb, normalized;
    
    // Resize to 224x224
    cv::resize(image, resized, cv::Size(IMAGE_SIZE, IMAGE_SIZE));
    
    // BGR to RGB
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    // Convert to float and normalize to [0, 1]
    rgb.convertTo(normalized, CV_32FC3, 1.0 / 255.0);
    
    // Split into channels
    std::vector<cv::Mat> channels(3);
    cv::split(normalized, channels);
    
    // ImageNet normalization: (pixel - mean) / std
    float mean[] = {0.485f, 0.456f, 0.406f};
    float std_dev[] = {0.229f, 0.224f, 0.225f};
    
    for (int i = 0; i < 3; i++) {
        channels[i] = (channels[i] - mean[i]) / std_dev[i];
    }
    
    // Convert to CHW format (Channels, Height, Width) for ONNX
    std::vector<float> tensor(3 * IMAGE_SIZE * IMAGE_SIZE);
    
    for (int c = 0; c < 3; c++) {
        for (int h = 0; h < IMAGE_SIZE; h++) {
            for (int w = 0; w < IMAGE_SIZE; w++) {
                tensor[c * IMAGE_SIZE * IMAGE_SIZE + h * IMAGE_SIZE + w] = 
                    channels[c].at<float>(h, w);
            }
        }
    }
    
    return tensor;
}

/* ============================================================================
 * Softmax Function
 * ============================================================================ */

std::vector<float> ML::softmax(const std::vector<float>& logits)
{
    std::vector<float> probs(logits.size());
    
    // Find max for numerical stability
    float maxVal = *std::max_element(logits.begin(), logits.end());
    
    float sum = 0.0f;
    for (size_t i = 0; i < logits.size(); i++) {
        probs[i] = std::exp(logits[i] - maxVal);
        sum += probs[i];
    }
    
    for (size_t i = 0; i < probs.size(); i++) {
        probs[i] /= sum;
    }
    
    return probs;
}

/* ============================================================================
 * Out-of-Distribution Detection
 * ============================================================================ */

/**
 * @brief Check if image contains sufficient green/plant-like colors
 * @param imagePath Path to image file
 * @return Ratio of green pixels (0.0 to 1.0)
 * 
 * Plants typically have significant green content. This heuristic helps
 * reject obvious non-plant images (keyboards, walls, etc.) that the ML
 * model might incorrectly classify due to lack of OOD training data.
 */
float ML::checkGreenRatio(const std::string& imagePath)
{
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        return 0.0f;
    }
    
    // Convert to HSV for better green detection
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    
    // Green color range in HSV
    // Hue: 35-85 (green range), Saturation: 30+ (not gray), Value: 30+ (not too dark)
    cv::Mat greenMask;
    cv::inRange(hsv, cv::Scalar(35, 30, 30), cv::Scalar(85, 255, 255), greenMask);
    
    // Also check for yellow-green (sick plants) and brown-green (diseased)
    cv::Mat yellowGreenMask;
    cv::inRange(hsv, cv::Scalar(20, 30, 30), cv::Scalar(35, 255, 255), yellowGreenMask);
    
    // Combine masks
    cv::Mat combinedMask;
    cv::bitwise_or(greenMask, yellowGreenMask, combinedMask);
    
    // Calculate ratio of green-ish pixels
    int greenPixels = cv::countNonZero(combinedMask);
    int totalPixels = image.rows * image.cols;
    
    float greenRatio = static_cast<float>(greenPixels) / static_cast<float>(totalPixels);
    
    std::cout << "[ML] Green pixel ratio: " << (greenRatio * 100) << "%" << std::endl;
    
    return greenRatio;
}

float ML::calculateEntropy(const std::vector<float>& probs)
{
    // Shannon entropy: H = -sum(p * log2(p))
    // For 4 classes: H=0 means 100% certain, H=2.0 means uniform distribution
    float entropy = 0.0f;
    for (float p : probs) {
        if (p > 1e-7f) {  // Avoid log(0)
            entropy -= p * std::log2(p);
        }
    }
    return entropy;
}

bool ML::checkValidPlant(float entropy, float maxConfidence, float greenRatio)
{
    // A valid plant prediction should have:
    // 1. Sufficient green/plant-like colors in the image
    // 2. Low entropy (model is confident about one class)
    // 3. High maximum confidence
    //
    // The green ratio check is critical because neural networks trained only
    // on plant images will still produce confident predictions for non-plant
    // inputs (they haven't learned what "not a plant" looks like).
    
    // Check 1: Image must contain some green/plant-like colors
    if (greenRatio < MIN_GREEN_RATIO) {
        std::cout << "[ML] Insufficient green pixels (" << (greenRatio * 100) << "% < " 
                  << (MIN_GREEN_RATIO * 100) << "%) - likely non-plant image" << std::endl;
        return false;
    }
    
    // Check 2: Entropy should be low (model is confident)
    // For 4 classes, max entropy is log2(4) = 2.0 (uniform distribution)
    if (entropy > ENTROPY_THRESHOLD) {
        std::cout << "[ML] High entropy (" << entropy << " > " << ENTROPY_THRESHOLD 
                  << ") - possible non-plant image" << std::endl;
        return false;
    }
    
    // Check 3: Confidence should be above threshold
    if (maxConfidence < MIN_CONFIDENCE_THRESHOLD) {
        std::cout << "[ML] Low confidence (" << (maxConfidence * 100) << "% < " 
                  << (MIN_CONFIDENCE_THRESHOLD * 100) << "%) - possible non-plant image" << std::endl;
        return false;
    }
    
    return true;
}

/* ============================================================================
 * Inference
 * ============================================================================ */

MLResult ML::analyzeDetailed(const std::string& imagePath)
{
    MLResult result;
    result.class_id = 2;  // Default to Healthy
    result.class_name = "Healthy";
    result.confidence = 1.0f;
    result.isValidPlant = true;
    result.entropy = 0.0f;
    
    // Mock mode if not initialized
    if (!initialized) {
        std::cout << "[ML] Mock mode: returning Healthy" << std::endl;
        return result;
    }
    
    try {
        // Preprocess image
        std::vector<float> inputTensor = preprocess(imagePath);
        
        if (inputTensor.empty()) {
            std::cerr << "[ML] Preprocessing failed, returning default" << std::endl;
            return result;
        }
        
        // Get session
        Ort::Session* ortSession = static_cast<Ort::Session*>(session);
        
        // Create input tensor
        std::vector<int64_t> inputShape = {1, 3, IMAGE_SIZE, IMAGE_SIZE};
        
        auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        
        Ort::Value inputOrt = Ort::Value::CreateTensor<float>(
            memoryInfo,
            inputTensor.data(),
            inputTensor.size(),
            inputShape.data(),
            inputShape.size()
        );
        
        // Get input/output names
        Ort::AllocatorWithDefaultOptions allocator;
        
        auto inputNamePtr = ortSession->GetInputNameAllocated(0, allocator);
        auto outputNamePtr = ortSession->GetOutputNameAllocated(0, allocator);
        
        const char* inputNames[] = {inputNamePtr.get()};
        const char* outputNames[] = {outputNamePtr.get()};
        
        // Run inference
        auto outputTensors = ortSession->Run(
            Ort::RunOptions{nullptr},
            inputNames,
            &inputOrt,
            1,
            outputNames,
            1
        );
        
        // Get output
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
        
        int numClasses = static_cast<int>(outputShape[1]);
        std::vector<float> logits(outputData, outputData + numClasses);
        
        // Apply softmax
        result.probs = softmax(logits);
        
        // Find max probability
        auto maxIt = std::max_element(result.probs.begin(), result.probs.end());
        result.class_id = std::distance(result.probs.begin(), maxIt);
        result.confidence = *maxIt;
        
        // Calculate entropy for out-of-distribution detection
        result.entropy = calculateEntropy(result.probs);
        
        // Check green ratio for color-based OOD detection
        float greenRatio = checkGreenRatio(imagePath);
        
        // Combined OOD check: entropy + confidence + green ratio
        result.isValidPlant = checkValidPlant(result.entropy, result.confidence, greenRatio);
        
        if (result.class_id < (int)CLASS_NAMES.size()) {
            result.class_name = CLASS_NAMES[result.class_id];
        } else {
            result.class_name = "Unknown";
        }
        
        // Modify output if not a valid plant
        if (!result.isValidPlant) {
            std::cout << "[ML] Out-of-distribution detected: " << imagePath << std::endl;
            std::cout << "[ML] Entropy: " << result.entropy << ", Max confidence: " 
                      << (result.confidence * 100) << "%, Green ratio: " 
                      << (greenRatio * 100) << "%" << std::endl;
            result.class_name = "Unknown (Not a Plant)";
            result.class_id = -1;  // Indicate invalid
        }
        
        std::cout << "[ML] Prediction: " << result.class_name 
                  << " (confidence: " << (result.confidence * 100) << "%"
                  << ", entropy: " << result.entropy 
                  << ", valid: " << (result.isValidPlant ? "yes" : "no") << ")" << std::endl;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "[ML] Inference error: " << e.what() << std::endl;
        result.class_id = 2;
        result.class_name = "Healthy";
        result.confidence = 0.0f;
        result.isValidPlant = false;
        result.entropy = 2.0f;
    }
    
    return result;
}

unsigned int ML::analyze(std::string imagePath)
{
    MLResult result = analyzeDetailed(imagePath);
    return static_cast<unsigned int>(result.class_id);
}