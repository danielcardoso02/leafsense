#ifndef TDS_H
#define TDS_H
#include "Sensor.h"
#include "ADC.h"
class TDS : public Sensor {
public:
    TDS(ADC* a, int c) {}
    float readSensor() override;
};
#endif