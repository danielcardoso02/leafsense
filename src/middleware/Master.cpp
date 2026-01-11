/**
 * @file Master.cpp
 * @brief Implementation of Main Control System Orchestrator
 */

#include "Master.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

Master::Master(MQueueHandler* queue) 
    : msgQueue(queue)
    , running(false)
    , sensorsCorrecting(false)
    , cameraCaptureCounter(0)
{
    // Initialize configuration
    idealConditions = new IdealConditions();
    
    // Initialize actuators (Mock GPIO pins)
    heater = new Heater(26);
    phuPump = new Pumps(6);
    phdPump = new Pumps(13);
    nPump = new Pumps(5);
    
    // Initialize sensors (Mock drivers)
    adc = new ADC(0x48);
    tempSensor = new Temp("mock_addr");
    phSensor = new PH(adc, 0);
    tdsSensor = new TDS(adc, 1);
    camera = new Cam();
    
    // Initialize ML engine with model path
    // Model located at: leafsense_model.onnx (in working directory)
    mlEngine = new ML(".", "leafsense_model.onnx");

    // Initialize synchronization primitives
    createMutexes();
    createConds();
}

Master::~Master() 
{
    stop();
    destroyMutexes();
    destroyConds();
    
    // Clean up allocated objects
    delete idealConditions;
    delete heater;
    delete phuPump;
    delete phdPump;
    delete nPump;
    delete adc;
    delete tempSensor;
    delete phSensor;
    delete tdsSensor;
    delete camera;
    delete mlEngine;
}

/* ============================================================================
 * Lifecycle Control
 * ============================================================================ */

void Master::start() 
{
    running = true;
    
    // Create all worker threads
    pthread_create(&tTime, NULL, tTimeFuncStatic, this);
    pthread_create(&tSig, NULL, tSigFuncStatic, this);
    pthread_create(&tReadSensors, NULL, tReadSensorsFuncStatic, this);
    pthread_create(&tWaterHeater, NULL, tWaterHeaterFuncStatic, this);
    pthread_create(&tPHU, NULL, tPHUFuncStatic, this);
    pthread_create(&tPHD, NULL, tPHDFuncStatic, this);
    pthread_create(&tNutrients, NULL, tNutrientsFuncStatic, this);
}

void Master::stop() 
{
    running = false;
    
    // Wake up all waiting threads
    triggerSignal(&condTime, &mutexTime);
    triggerSignal(&condRS, &mutexRS);
    triggerSignal(&condWH, &mutexWH);
    triggerSignal(&condPHU, &mutexPHU);
    triggerSignal(&condPHD, &mutexPHD);
    triggerSignal(&condN, &mutexN);
    
    // Wait for threads to finish
    pthread_join(tTime, NULL);
    pthread_join(tSig, NULL);
}

/* ============================================================================
 * Thread Functions - Timer
 * ============================================================================ */

void Master::tTimeFunc() 
{
    while (running) {
        sleep(2);  // 2 second heartbeat interval
        triggerSignal(&condTime, &mutexTime);
    }
}

void Master::tSigFunc() 
{
    while (running) {
        captureSignal(&condTime, &mutexTime);
        if (!running) break;
        triggerSignal(&condRS, &mutexRS);
    }
}

/* ============================================================================
 * Thread Functions - Sensor Reading & Control Logic
 * ============================================================================ */

void Master::tReadSensorsFunc() 
{
    float phRange[2], tempRange[2], tdsRange[2];
    
    while (running) {
        captureSignal(&condRS, &mutexRS);
        if (!running) break;

        // Read all sensors
        float t = tempSensor->readSensor();
        float p = phSensor->readSensor();
        float e = tdsSensor->readSensor();
        
        // Log to database via message queue
        std::stringstream ss;
        ss << "SENSOR|" << t << "|" << p << "|" << e;
        msgQueue->sendMessage(ss.str());

        /* --------------------------------------------------------------------
         * Periodic Camera Capture & ML Analysis (every 30 minutes)
         * First capture happens 4 seconds after startup for testing
         * -------------------------------------------------------------------- */
        cameraCaptureCounter++;
        if (cameraCaptureCounter >= 900 || cameraCaptureCounter == 2) {  // Trigger at 2 for initial test, then every 30 min
            if (cameraCaptureCounter >= 900) cameraCaptureCounter = 0;
            
            std::cout << "[Master] Capturing photo for ML analysis..." << std::endl;
            std::string photoPath = camera->takePhoto();
            
            if (!photoPath.empty()) {
                // Run ML inference on captured image
                MLResult mlResult = mlEngine->analyzeDetailed(photoPath);
                
                // Log ML prediction to database
                std::stringstream mlLog;
                mlLog << "LOG|ML Analysis|" << mlResult.class_name 
                      << "|Confidence: " << (mlResult.confidence * 100) << "%";
                msgQueue->sendMessage(mlLog.str());
                
                std::cout << "[Master] ML Result: " << mlResult.class_name 
                          << " (" << (mlResult.confidence * 100) << "%)" << std::endl;
            } else {
                std::cerr << "[Master] Failed to capture photo" << std::endl;
            }
        }

        // Get ideal ranges for control decisions
        idealConditions->getTemp(tempRange);
        idealConditions->getPH(phRange);
        idealConditions->getTDS(tdsRange);

        /* --------------------------------------------------------------------
         * Temperature Control (with hysteresis)
         * -------------------------------------------------------------------- */
        if (t < tempRange[0] && !heater->getState()) {
            triggerSignal(&condWH, &mutexWH);
        } else if (t > tempRange[1] && heater->getState()) {
            triggerSignal(&condWH, &mutexWH);
        }

        /* --------------------------------------------------------------------
         * pH Control
         * -------------------------------------------------------------------- */
        if (p < phRange[0]) {
            triggerSignal(&condPHU, &mutexPHU);
        } else if (p > phRange[1]) {
            triggerSignal(&condPHD, &mutexPHD);
        }

        /* --------------------------------------------------------------------
         * TDS/Nutrient Control
         * -------------------------------------------------------------------- */
        if (e < tdsRange[0]) {
            triggerSignal(&condN, &mutexN);
        }
        
        /* --------------------------------------------------------------------
         * Update Alert LED (kernel module integration)
         * -------------------------------------------------------------------- */
        updateAlertLED();
    }
}

/* ============================================================================
 * Thread Functions - Actuator Control
 * ============================================================================ */

void Master::tWaterHeaterFunc() 
{
    while (running) {
        captureSignal(&condWH, &mutexWH);
        if (!running) break;
        
        heater->setState(!heater->getState());
        msgQueue->sendMessage(heater->getState() 
            ? "LOG|Maintenance|Heater ON|Auto" 
            : "LOG|Maintenance|Heater OFF|Auto");
    }
}

void Master::tPHUFunc() 
{
    while (running) {
        captureSignal(&condPHU, &mutexPHU);
        if (!running) break;
        
        phuPump->pump(500);
        msgQueue->sendMessage("LOG|Maintenance|pH Up|Dosed 500ms");
    }
}

void Master::tPHDFunc() 
{
    while (running) {
        captureSignal(&condPHD, &mutexPHD);
        if (!running) break;
        
        phdPump->pump(500);
        msgQueue->sendMessage("LOG|Maintenance|pH Down|Dosed 500ms");
    }
}

void Master::tNutrientsFunc() 
{
    while (running) {
        captureSignal(&condN, &mutexN);
        if (!running) break;
        
        nPump->pump(1000);
        msgQueue->sendMessage("LOG|Maintenance|Nutrients|Dosed 1000ms");
    }
}

/* ============================================================================
 * Static Thread Entry Points
 * ============================================================================ */

void* Master::tTimeFuncStatic(void* arg) { 
    ((Master*)arg)->tTimeFunc(); 
    return NULL; 
}

void* Master::tSigFuncStatic(void* arg) { 
    ((Master*)arg)->tSigFunc(); 
    return NULL; 
}

void* Master::tReadSensorsFuncStatic(void* arg) { 
    ((Master*)arg)->tReadSensorsFunc(); 
    return NULL; 
}

void* Master::tWaterHeaterFuncStatic(void* arg) { 
    ((Master*)arg)->tWaterHeaterFunc(); 
    return NULL; 
}

void* Master::tPHUFuncStatic(void* arg) { 
    ((Master*)arg)->tPHUFunc(); 
    return NULL; 
}

void* Master::tPHDFuncStatic(void* arg) { 
    ((Master*)arg)->tPHDFunc(); 
    return NULL; 
}

void* Master::tNutrientsFuncStatic(void* arg) { 
    ((Master*)arg)->tNutrientsFunc(); 
    return NULL; 
}

/* ============================================================================
 * Synchronization Helpers
 * ============================================================================ */

void Master::captureSignal(pthread_cond_t* c, pthread_mutex_t* m) 
{
    pthread_mutex_lock(m);
    pthread_cond_wait(c, m);
    pthread_mutex_unlock(m);
}

void Master::triggerSignal(pthread_cond_t* c, pthread_mutex_t* m) 
{
    pthread_mutex_lock(m);
    pthread_cond_signal(c);
    pthread_mutex_unlock(m);
}

/* ============================================================================
 * Mutex/Condition Variable Initialization
 * ============================================================================ */

void Master::createMutexes() 
{
    pthread_mutex_init(&mutexRS, NULL);
    pthread_mutex_init(&mutexTime, NULL);
    pthread_mutex_init(&mutexN, NULL);
    pthread_mutex_init(&mutexPHU, NULL);
    pthread_mutex_init(&mutexPHD, NULL);
    pthread_mutex_init(&mutexWH, NULL);
}

void Master::createConds() 
{
    pthread_cond_init(&condRS, NULL);
    pthread_cond_init(&condTime, NULL);
    pthread_cond_init(&condN, NULL);
    pthread_cond_init(&condPHU, NULL);
    pthread_cond_init(&condPHD, NULL);
    pthread_cond_init(&condWH, NULL);
}

void Master::destroyMutexes() 
{
    pthread_mutex_destroy(&mutexRS);
    pthread_mutex_destroy(&mutexTime);
    pthread_mutex_destroy(&mutexN);
    pthread_mutex_destroy(&mutexPHU);
    pthread_mutex_destroy(&mutexPHD);
    pthread_mutex_destroy(&mutexWH);
}

void Master::destroyConds() 
{
    pthread_cond_destroy(&condRS);
    pthread_cond_destroy(&condTime);
    pthread_cond_destroy(&condN);
    pthread_cond_destroy(&condPHU);
    pthread_cond_destroy(&condPHD);
    pthread_cond_destroy(&condWH);
}

/* ============================================================================
 * LED Alert Control
 * ============================================================================ */

void Master::updateAlertLED() 
{
    // Get ideal ranges
    float tempRange[2], phRange[2], tdsRange[2];
    idealConditions->getTemp(tempRange);
    idealConditions->getPH(phRange);
    idealConditions->getTDS(tdsRange);
    
    // Get current sensor values
    float t = tempSensor->readSensor();
    float p = phSensor->readSensor();
    float e = tdsSensor->readSensor();
    
    // Check if any parameter is out of range
    bool alertActive = (t < tempRange[0] || t > tempRange[1] ||
                        p < phRange[0] || p > phRange[1] ||
                        e < tdsRange[0] || e > tdsRange[1]);
    
    // Control LED via kernel module
    static int ledFd = -1;
    
    // Open LED device on first call
    if (ledFd == -1) {
        ledFd = open("/dev/leddev", O_WRONLY);
        if (ledFd < 0) {
            // LED module not loaded, skip LED control
            return;
        }
    }
    
    // Write '1' or '0' to LED device
    const char* cmd = alertActive ? "1" : "0";
    ssize_t written = write(ledFd, cmd, 1);
    
    if (written != 1) {
        std::cerr << "[Master] Failed to write to LED device" << std::endl;
    }
}