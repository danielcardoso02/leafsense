#ifndef IDEALCONDITIONS_H
#define IDEALCONDITIONS_H

class IdealConditions {
private:
    float tds_min, tds_max, ph_min, ph_max, temp_min, temp_max;
public:
    IdealConditions();
    void getTDS(float* r);
    void getPH(float* r);
    void getTemp(float* r);
    void setTDS(float min, float max);
    void setPH(float min, float max);
    void setTemp(float min, float max);
};
#endif