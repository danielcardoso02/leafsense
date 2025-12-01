#ifndef TEMP_H
#define TEMP_H
#include "Sensor.h"
#include <string>
class Temp : public Sensor {
public:
    Temp(std::string addr) {}
    float readSensor() override;
};
#endif