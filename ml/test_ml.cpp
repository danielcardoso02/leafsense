/**
 * @file test_ml.cpp
 * @brief Simple test for ML inference
 */

#include <iostream>
#include "../include/application/ml/ML.h"

int main()
{
    std::cout << "=== LeafSense ML Test ===" << std::endl;
    
    // Initialize ML engine
    ML ml(".", "leafsense_model.onnx");
    
    if (!ml.isInitialized()) {
        std::cerr << "Failed to initialize ML engine!" << std::endl;
        return 1;
    }
    
    std::cout << "\nTesting with logo image..." << std::endl;
    
    // Test with available image
    std::string testImage = "../resources/images/logo_leafsense.png";
    
    // Test legacy interface
    unsigned int result = ml.analyze(testImage);
    std::cout << "Legacy analyze() returned: " << result << std::endl;
    
    // Test detailed interface
    MLResult detailed = ml.analyzeDetailed(testImage);
    std::cout << "\nDetailed result:" << std::endl;
    std::cout << "  Class ID: " << detailed.class_id << std::endl;
    std::cout << "  Class Name: " << detailed.class_name << std::endl;
    std::cout << "  Confidence: " << (detailed.confidence * 100) << "%" << std::endl;
    
    std::cout << "\nAll class probabilities:" << std::endl;
    const char* classNames[] = {"Healthy", "Nutrient Deficiency", "Disease", "Pest Damage"};
    for (size_t i = 0; i < detailed.probs.size() && i < 4; i++) {
        std::cout << "  " << i << ": " << classNames[i] << ": " 
                  << (detailed.probs[i] * 100) << "%" << std::endl;
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}
