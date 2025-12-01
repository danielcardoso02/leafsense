#ifndef MASTER_H
#define MASTER_H

#include <pthread.h>
#include <unistd.h>
#include "MQueueHandler.h"
#include "IdealConditions.h"

// Drivers
#include "drivers/sensors/Temp.h"
#include "drivers/sensors/PH.h"
#include "drivers/sensors/TDS.h"
#include "drivers/sensors/ADC.h"
#include "drivers/sensors/Cam.h"
#include "drivers/actuators/Pumps.h"
#include "drivers/actuators/Heater.h"
#include "application/ml/ML.h"

class Master {
private:
    MQueueHandler* msgQueue;
    bool running;
    bool sensorsCorrecting;

    IdealConditions* idealConditions;
    Heater* heater;
    Pumps *phuPump, *phdPump, *nPump;
    ADC* adc;
    Temp* tempSensor;
    PH* phSensor;
    TDS* tdsSensor;
    Cam* camera;
    ML* mlEngine;

    pthread_t tTime, tSig, tReadSensors, tWaterHeater, tPHU, tPHD, tNutrients;
    pthread_mutex_t mutexRS, mutexTime, mutexN, mutexPHU, mutexPHD, mutexWH;
    pthread_cond_t condRS, condTime, condN, condPHU, condPHD, condWH;

    void createMutexes(); void createConds();
    void destroyMutexes(); void destroyConds();
    void captureSignal(pthread_cond_t* c, pthread_mutex_t* m);
    void triggerSignal(pthread_cond_t* c, pthread_mutex_t* m);

public:
    Master(MQueueHandler* queue);
    ~Master();
    void start();
    void stop();

    static void* tTimeFuncStatic(void* arg);
    static void* tSigFuncStatic(void* arg);
    static void* tReadSensorsFuncStatic(void* arg);
    static void* tWaterHeaterFuncStatic(void* arg);
    static void* tPHUFuncStatic(void* arg);
    static void* tPHDFuncStatic(void* arg);
    static void* tNutrientsFuncStatic(void* arg);

    void tTimeFunc(); void tSigFunc(); void tReadSensorsFunc();
    void tWaterHeaterFunc(); void tPHUFunc(); void tPHDFunc(); void tNutrientsFunc();
};
#endif