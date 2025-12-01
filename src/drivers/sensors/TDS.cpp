#include "TDS.h"
#include <cstdlib>

// Mock: Returns random EC/TDS
float TDS::readSensor() {
    realValue = 1200.0 + (rand() % 200);
    return realValue;
}