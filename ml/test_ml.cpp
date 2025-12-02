/**
 * @file test_ml.cpp
 * @brief Simple test for ML inference
 */

#include <iostream>
#include "../include/application/ml/ML.h"

int main(int argc, char* argv[])
{
    std::cout << "=== LeafSense ML Test ===" << std::endl;
    
    // Initialize ML engine
    ML ml(".", "leafsense_model.onnx");
    
    if (!ml.isInitialized()) {
        std::cerr << "Failed to initialize ML engine!" << std::endl;
        return 1;
    }
    
    // Get test image from argument or use default
    std::string testImage;
    if (argc > 1) {
        testImage = argv[1];
        std::cout << "\nTesting with: " << testImage << std::endl;
    } else {
        testImage = "../resources/images/logo_leafsense.png";
        std::cout << "\nTesting with default image..." << std::endl;
    }
    
    // Test detailed interface
    MLResult detailed = ml.analyzeDetailed(testImage);
    
    std::cout << "\nResult:" << std::endl;
    std::cout << "  Class: " << detailed.class_name << std::endl;
    std::cout << "  Confidence: " << (detailed.confidence * 100) << "%" << std::endl;
    
    std::cout << "\nAll class probabilities:" << std::endl;
    const char* classNames[] = {"Nutrient Deficiency", "Disease", "Healthy", "Pest Damage"};
    for (size_t i = 0; i < detailed.probs.size() && i < 4; i++) {
        std::cout << "  " << classNames[i] << ": " 
                  << (detailed.probs[i] * 100) << "%" << std::endl;
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}
