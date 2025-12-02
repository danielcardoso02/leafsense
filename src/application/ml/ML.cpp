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
 * Inference
 * ============================================================================ */

MLResult ML::analyzeDetailed(const std::string& imagePath)
{
    MLResult result;
    result.class_id = 0;
    result.class_name = "Healthy";
    result.confidence = 1.0f;
    
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
        
        if (result.class_id < (int)CLASS_NAMES.size()) {
            result.class_name = CLASS_NAMES[result.class_id];
        } else {
            result.class_name = "Unknown";
        }
        
        std::cout << "[ML] Prediction: " << result.class_name 
                  << " (confidence: " << (result.confidence * 100) << "%)" << std::endl;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "[ML] Inference error: " << e.what() << std::endl;
        result.class_id = 0;
        result.class_name = "Healthy";
        result.confidence = 0.0f;
    }
    
    return result;
}

unsigned int ML::analyze(std::string imagePath)
{
    MLResult result = analyzeDetailed(imagePath);
    return static_cast<unsigned int>(result.class_id);
}