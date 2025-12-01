#include "ML.h"

// Mock: Returns 0 (Healthy) without loading heavy ONNX models
unsigned int ML::analyze(std::string imagePath) {
    return 0; 
}