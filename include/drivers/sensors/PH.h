#ifndef PH_H
#define PH_H
#include "Sensor.h"
#include "ADC.h"
class PH : public Sensor {
public:
    PH(ADC* a, int c) {}
    float readSensor() override;
};
#endif