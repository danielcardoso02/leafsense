/**
 * @file test_ml_dataset.cpp
 * @brief Test ML with dataset images
 * @author Daniel Cardoso, Marco Costa
 */

#include <iostream>
#include <filesystem>
#include <map>
#include <algorithm>
#include <cctype>
#include "../include/application/ml/ML.h"

namespace fs = std::filesystem;

int main()
{
    std::cout << "=== LeafSense ML Dataset Test ===" << std::endl;
    
    // Initialize ML engine
    ML ml(".", "leafsense_model.onnx");
    
    if (!ml.isInitialized()) {
        std::cerr << "Failed to initialize ML engine!" << std::endl;
        return 1;
    }
    
    std::string datasetPath = "../ml/dataset/val";
    
    if (!fs::exists(datasetPath)) {
        std::cerr << "Dataset not found at: " << datasetPath << std::endl;
        return 1;
    }
    
    std::map<std::string, int> correct;
    std::map<std::string, int> total;
    
    // Test each class
    for (const auto& classDir : fs::directory_iterator(datasetPath)) {
        if (!classDir.is_directory()) continue;
        
        std::string className = classDir.path().filename().string();
        std::cout << "\nTesting class: " << className << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        int classCorrect = 0;
        int classTotal = 0;
        
        for (const auto& imgFile : fs::directory_iterator(classDir.path())) {
            if (!imgFile.is_regular_file()) continue;
            
            std::string imgPath = imgFile.path().string();
            MLResult result = ml.analyzeDetailed(imgPath);
            
            // Convert prediction to lowercase for comparison
            std::string predLower = result.class_name;
            std::transform(predLower.begin(), predLower.end(), predLower.begin(), ::tolower);
            std::replace(predLower.begin(), predLower.end(), ' ', '_');
            
            bool isCorrect = (predLower == className);
            
            if (isCorrect) classCorrect++;
            classTotal++;
            
            // Only print first 3 per class
            if (classTotal <= 3) {
                std::cout << "  " << imgFile.path().filename().string() 
                          << " -> " << result.class_name 
                          << " (" << (result.confidence * 100) << "%)"
                          << (isCorrect ? " ✓" : " ✗")
                          << std::endl;
            }
        }
        
        correct[className] = classCorrect;
        total[className] = classTotal;
        
        if (classTotal > 3) {
            std::cout << "  ... (" << classTotal - 3 << " more images)" << std::endl;
        }
        
        std::cout << "  Accuracy: " << classCorrect << "/" << classTotal 
                  << " (" << (100.0 * classCorrect / classTotal) << "%)" << std::endl;
    }
    
    // Overall accuracy
    int totalCorrect = 0, totalImages = 0;
    for (const auto& [cls, cnt] : correct) {
        totalCorrect += cnt;
        totalImages += total[cls];
    }
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Overall Accuracy: " << totalCorrect << "/" << totalImages 
              << " (" << (100.0 * totalCorrect / totalImages) << "%)" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    return 0;
}
