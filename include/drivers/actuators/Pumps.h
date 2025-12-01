#ifndef PUMPS_H
#define PUMPS_H
class Pumps {
    int gpioPin;
public:
    Pumps(int pin) : gpioPin(pin) {}
    void pump(unsigned int duration);
};
#endif