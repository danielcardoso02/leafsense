#include "PH.h"
#include <cstdlib>

// Mock: Returns random pH between 6.0 and 7.0
float PH::readSensor() {
    float noise = (float)(rand() % 100) / 100.0;
    realValue = 6.0 + noise;
    return realValue;
}