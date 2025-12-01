#ifndef SENSOR_H
#define SENSOR_H
class Sensor {
protected:
    float realValue;
    bool correcting;
public:
    Sensor() : realValue(0), correcting(false) {}
    virtual ~Sensor() {}
    virtual float readSensor() = 0;
    void setTime(bool c) { correcting = c; }
};
#endif