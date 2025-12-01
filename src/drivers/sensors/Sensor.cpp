#include "../include/drivers/sensors/Sensor.h"

// Constructor
Sensor::Sensor() : realValue(0.0), correcting(false) {
}

Sensor::~Sensor() {
}

// Helper to get the last read value
float Sensor::getValue() {
    return realValue;
}

// Sets the 'correcting' flag
// Used by Master to tell the sensor "We are fixing the water, read faster!"
void Sensor::setTime(bool isCorrecting) {
    correcting = isCorrecting;
}