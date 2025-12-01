#include "Master.h"
#include <iostream>
#include <sstream>

Master::Master(MQueueHandler* queue) : msgQueue(queue), running(false), sensorsCorrecting(false) {
    idealConditions = new IdealConditions();
    
    // Initialize Mock Drivers
    heater = new Heater(26); 
    phuPump = new Pumps(6);  phdPump = new Pumps(13); nPump = new Pumps(5);
    adc = new ADC(0x48);
    tempSensor = new Temp("mock_addr");
    phSensor = new PH(adc, 0);    
    tdsSensor = new TDS(adc, 1);
    camera = new Cam();
    mlEngine = new ML("mock", "mock");

    createMutexes(); createConds();
}

Master::~Master() {
    stop();
    destroyMutexes(); destroyConds();
    delete idealConditions; delete heater; delete phuPump; delete phdPump; delete nPump;
    delete adc; delete tempSensor; delete phSensor; delete tdsSensor; delete camera; delete mlEngine;
}

void Master::start() {
    running = true;
    pthread_create(&tTime, NULL, tTimeFuncStatic, this);
    pthread_create(&tSig, NULL, tSigFuncStatic, this);
    pthread_create(&tReadSensors, NULL, tReadSensorsFuncStatic, this);
    pthread_create(&tWaterHeater, NULL, tWaterHeaterFuncStatic, this);
    pthread_create(&tPHU, NULL, tPHUFuncStatic, this);
    pthread_create(&tPHD, NULL, tPHDFuncStatic, this);
    pthread_create(&tNutrients, NULL, tNutrientsFuncStatic, this);
}

void Master::stop() {
    running = false;
    triggerSignal(&condTime, &mutexTime);
    triggerSignal(&condRS, &mutexRS);
    triggerSignal(&condWH, &mutexWH);
    triggerSignal(&condPHU, &mutexPHU);
    triggerSignal(&condPHD, &mutexPHD);
    triggerSignal(&condN, &mutexN);
    pthread_join(tTime, NULL);
    pthread_join(tSig, NULL);
}

// --- Logic ---
void Master::tTimeFunc() {
    while(running) {
        sleep(2); // Heartbeat
        triggerSignal(&condTime, &mutexTime);
    }
}

void Master::tSigFunc() {
    while(running) {
        captureSignal(&condTime, &mutexTime);
        if (!running) break;
        triggerSignal(&condRS, &mutexRS);
    }
}

void Master::tReadSensorsFunc() {
    float phRange[2], tempRange[2], tdsRange[2];
    while(running) {
        captureSignal(&condRS, &mutexRS);
        if (!running) break;

        float t = tempSensor->readSensor();
        float p = phSensor->readSensor();
        float e = tdsSensor->readSensor();
        
        // Log to Database
        std::stringstream ss;
        ss << "SENSOR|" << t << "|" << p << "|" << e;
        msgQueue->sendMessage(ss.str());

        // Control Logic
        idealConditions->getTemp(tempRange);
        idealConditions->getPH(phRange);
        idealConditions->getTDS(tdsRange);

        // Temperature control (with hysteresis)
        if (t < tempRange[0] && !heater->getState()) {
            triggerSignal(&condWH, &mutexWH);
        } else if (t > tempRange[1] && heater->getState()) {
            triggerSignal(&condWH, &mutexWH);
        }

        // pH control
        if (p < phRange[0]) triggerSignal(&condPHU, &mutexPHU);
        else if (p > phRange[1]) triggerSignal(&condPHD, &mutexPHD);

        // TDS/Nutrient control
        if (e < tdsRange[0]) triggerSignal(&condN, &mutexN);
    }
}

// Actuators
void Master::tWaterHeaterFunc() {
    while(running) {
        captureSignal(&condWH, &mutexWH);
        if (!running) break;
        heater->setState(!heater->getState());
        msgQueue->sendMessage(heater->getState() ? "LOG|Maintenance|Heater ON|Auto" : "LOG|Maintenance|Heater OFF|Auto");
    }
}
void Master::tPHUFunc() {
    while(running) {
        captureSignal(&condPHU, &mutexPHU);
        if (!running) break;
        phuPump->pump(500);
        msgQueue->sendMessage("LOG|Maintenance|pH Up|Dosed 500ms");
    }
}
void* Master::tPHDFuncStatic(void* arg) { ((Master*)arg)->tPHDFunc(); return NULL; }
void Master::tPHDFunc() {
    while(running) {
        captureSignal(&condPHD, &mutexPHD);
        if (!running) break;
        phdPump->pump(500);
        msgQueue->sendMessage("LOG|Maintenance|pH Down|Dosed 500ms");
    }
}
void* Master::tNutrientsFuncStatic(void* arg) { ((Master*)arg)->tNutrientsFunc(); return NULL; }
void Master::tNutrientsFunc() {
    while(running) {
        captureSignal(&condN, &mutexN);
        if (!running) break;
        nPump->pump(1000);
        msgQueue->sendMessage("LOG|Maintenance|Nutrients|Dosed 1000ms");
    }
}

void* Master::tTimeFuncStatic(void* arg) { ((Master*)arg)->tTimeFunc(); return NULL; }
void* Master::tSigFuncStatic(void* arg) { ((Master*)arg)->tSigFunc(); return NULL; }
void* Master::tReadSensorsFuncStatic(void* arg) { ((Master*)arg)->tReadSensorsFunc(); return NULL; }
void* Master::tWaterHeaterFuncStatic(void* arg) { ((Master*)arg)->tWaterHeaterFunc(); return NULL; }
void* Master::tPHUFuncStatic(void* arg) { ((Master*)arg)->tPHUFunc(); return NULL; }

void Master::captureSignal(pthread_cond_t* c, pthread_mutex_t* m) {
    pthread_mutex_lock(m); pthread_cond_wait(c, m); pthread_mutex_unlock(m);
}
void Master::triggerSignal(pthread_cond_t* c, pthread_mutex_t* m) {
    pthread_mutex_lock(m); pthread_cond_signal(c); pthread_mutex_unlock(m);
}

void Master::createMutexes() {
    pthread_mutex_init(&mutexRS, NULL); pthread_mutex_init(&mutexTime, NULL);
    pthread_mutex_init(&mutexN, NULL); pthread_mutex_init(&mutexPHU, NULL);
    pthread_mutex_init(&mutexPHD, NULL); pthread_mutex_init(&mutexWH, NULL);
}
void Master::createConds() {
    pthread_cond_init(&condRS, NULL); pthread_cond_init(&condTime, NULL);
    pthread_cond_init(&condN, NULL); pthread_cond_init(&condPHU, NULL);
    pthread_cond_init(&condPHD, NULL); pthread_cond_init(&condWH, NULL);
}
void Master::destroyMutexes() {
    pthread_mutex_destroy(&mutexRS); pthread_mutex_destroy(&mutexTime);
    pthread_mutex_destroy(&mutexN); pthread_mutex_destroy(&mutexPHU);
    pthread_mutex_destroy(&mutexPHD); pthread_mutex_destroy(&mutexWH);
}
void Master::destroyConds() {
    pthread_cond_destroy(&condRS); pthread_cond_destroy(&condTime);
    pthread_cond_destroy(&condN); pthread_cond_destroy(&condPHU);
    pthread_cond_destroy(&condPHD); pthread_cond_destroy(&condWH);
}