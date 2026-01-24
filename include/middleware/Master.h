/**
 * @file Master.h
 * @brief Main Control System Orchestrator
 * @author Daniel Cardoso, Marco Costa
 * @layer Middleware
 * 
 * Implements the multi-threaded control system architecture.
 * Coordinates sensor reading, data logging, and actuator control
 * using POSIX threads with mutex/condition variable synchronization.
 * 
 * Thread Architecture:
 * - tTime: Heartbeat timer (2s interval)
 * - tSig: Signal dispatcher
 * - tReadSensors: Sensor polling and control logic
 * - tWaterHeater: Temperature control actuator
 * - tPHU/tPHD: pH Up/Down pump control
 * - tNutrients: Nutrient dosing pump control
 */

#ifndef MASTER_H
#define MASTER_H

/* ============================================================================
 * System Includes
 * ============================================================================ */
#include <pthread.h>
#include <unistd.h>

/* ============================================================================
 * Middleware Includes
 * ============================================================================ */
#include "MQueueHandler.h"
#include "IdealConditions.h"

/* ============================================================================
 * Driver Includes - Sensors
 * ============================================================================ */
#include "drivers/sensors/Temp.h"
#include "drivers/sensors/PH.h"
#include "drivers/sensors/TDS.h"
#include "drivers/sensors/ADC.h"
#include "drivers/sensors/Cam.h"

/* ============================================================================
 * Driver Includes - Actuators
 * ============================================================================ */
#include "drivers/actuators/Pumps.h"
#include "drivers/actuators/Heater.h"

/* ============================================================================
 * Application Includes
 * ============================================================================ */
#include "application/ml/ML.h"

/**
 * @class Master
 * @brief Central control system coordinator
 * 
 * Manages all backend threads and hardware interfaces.
 * Implements producer pattern for database logging via MQueue.
 */
class Master {
private:
    /* ------------------------------------------------------------------------
     * Communication
     * ------------------------------------------------------------------------ */
    MQueueHandler* msgQueue;     ///< Queue for database logging
    bool running;                ///< Thread run flag
    bool sensorsCorrecting;      ///< Flag: correction in progress
    int cameraCaptureCounter;    ///< Counter for periodic camera capture
    int cameraCaptureInterval; ///< Camera capture interval in tSig activations (900 = 30 min)
    int readSensorCD;            ///< Cooldown counter for sensor reading
    int readSensorInterval; ///< Sensor read interval in tSig activations

    /* ------------------------------------------------------------------------
     * Configuration
     * ------------------------------------------------------------------------ */
    IdealConditions* idealConditions;  ///< Ideal parameter ranges

    /* ------------------------------------------------------------------------
     * Actuators
     * ------------------------------------------------------------------------ */
    Heater* heater;              ///< Water heater control
    Pumps *phuPump;              ///< pH Up dosing pump
    Pumps *phdPump;              ///< pH Down dosing pump
    Pumps *nPump;                ///< Nutrient dosing pump

    /* ------------------------------------------------------------------------
     * Sensors
     * ------------------------------------------------------------------------ */
    ADC* adc;                    ///< Analog-to-Digital converter
    Temp* tempSensor;            ///< DS18B20 temperature sensor
    PH* phSensor;                ///< pH probe via ADC
    TDS* tdsSensor;              ///< TDS/EC probe via ADC
    Cam* camera;                 ///< USB camera for ML
    ML* mlEngine;                ///< Machine Learning inference

    /* ------------------------------------------------------------------------
     * Thread Handles
     * ------------------------------------------------------------------------ */
    pthread_t tTime;             ///< Heartbeat timer thread
    pthread_t tSig;              ///< Signal dispatcher thread
    pthread_t tReadSensors;      ///< Sensor polling thread
    pthread_t tCamera;           ///< Camera capture & ML thread
    pthread_t tWaterHeater;      ///< Heater control thread
    pthread_t tPHU;              ///< pH Up pump thread
    pthread_t tPHD;              ///< pH Down pump thread
    pthread_t tNutrients;        ///< Nutrient pump thread

    /* ------------------------------------------------------------------------
     * Synchronization Primitives
     * ------------------------------------------------------------------------ */
    pthread_mutex_t mutexRS, mutexTime, mutexN, mutexPHU, mutexPHD, mutexWH, mutexCam;
    pthread_cond_t condRS, condTime, condN, condPHU, condPHD, condWH, condCam;

    /* ------------------------------------------------------------------------
     * Private Methods - Synchronization
     * ------------------------------------------------------------------------ */
    void createMutexes();        ///< Initialize all mutexes
    void createConds();          ///< Initialize all condition variables
    void destroyMutexes();       ///< Destroy all mutexes
    void destroyConds();         ///< Destroy all condition variables
    
    /**
     * @brief Blocks until signal received
     * @param c Condition variable to wait on
     * @param m Associated mutex
     */
    void captureSignal(pthread_cond_t* c, pthread_mutex_t* m);
    
    /**
     * @brief Sends signal to waiting thread
     * @param c Condition variable to signal
     * @param m Associated mutex
     */
    void triggerSignal(pthread_cond_t* c, pthread_mutex_t* m);
    
    /**
     * @brief Updates alert LED based on sensor readings (legacy)
     * 
     * Checks if any parameter is out of ideal range and controls
     * the LED via kernel module (/dev/led0).
     * LED ON = Alert active, LED OFF = All parameters normal
     */
    void updateAlertLED();
    
    /**
     * @brief Controls alert LED based on ML classification
     * 
     * Turns LED ON when bad class detected (Disease, Deficiency, Pest)
     * Turns LED OFF when Healthy or OOD
     * @param alertActive true to turn LED ON, false for OFF
     */
    void setMLAlertLED(bool alertActive);
    
    /**
     * @brief Generates recommendations based on ML prediction results
     * 
     * Creates specific treatment recommendations based on detected issues.
     * Correlates ML predictions with current sensor readings for
     * more accurate nutrient deficiency recommendations.
     * 
     * @param mlResult The ML inference result with class and confidence
     * @param filename The image filename for database linking
     */
    void generateMLRecommendation(const MLResult& mlResult, const std::string& filename);

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructs Master with message queue
     * @param queue Shared queue for database logging
     */
    Master(MQueueHandler* queue);
    ~Master();

    /* ------------------------------------------------------------------------
     * Lifecycle Control
     * ------------------------------------------------------------------------ */
    
    void start();  ///< Creates and starts all threads
    void stop();   ///< Signals all threads to stop and joins them

    /* ------------------------------------------------------------------------
     * Static Thread Entry Points (pthread requires static)
     * ------------------------------------------------------------------------ */
    static void* tTimeFuncStatic(void* arg);
    static void* tSigFuncStatic(void* arg);
    static void* tReadSensorsFuncStatic(void* arg);
    static void* tCameraFuncStatic(void* arg);
    static void* tWaterHeaterFuncStatic(void* arg);
    static void* tPHUFuncStatic(void* arg);
    static void* tPHDFuncStatic(void* arg);
    static void* tNutrientsFuncStatic(void* arg);

    /* ------------------------------------------------------------------------
     * Thread Functions (Instance Methods)
     * ------------------------------------------------------------------------ */
    void tTimeFunc();         ///< Heartbeat: triggers every 2s
    void tSigFunc();          ///< Dispatcher: forwards time signal
    void tReadSensorsFunc();  ///< Reads sensors, triggers actuators
    void tCameraFunc();       ///< Camera capture & ML analysis
    void tWaterHeaterFunc();  ///< Toggles heater state
    void tPHUFunc();          ///< Doses pH Up solution
    void tPHDFunc();          ///< Doses pH Down solution
    void tNutrientsFunc();    ///< Doses nutrient solution
};

#endif // MASTER_H