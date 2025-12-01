#ifndef HEATER_H
#define HEATER_H
class Heater {
    bool state;
public:
    Heater(int pin) : state(false) {}
    void setState(bool on);
    bool getState() { return state; }
};
#endif