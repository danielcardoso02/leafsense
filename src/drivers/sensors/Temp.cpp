#include "Temp.h"
#include <cstdlib> // rand

// Mock: Returns random temperature between 20.0 and 25.0
float Temp::readSensor() {
    float noise = (float)(rand() % 50) / 10.0; 
    realValue = 20.0 + noise;
    return realValue;
}
