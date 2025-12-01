#ifndef ADC_H
#define ADC_H
class ADC {
public:
    ADC(int addr);
    float readVoltage(int ch);
};
#endif