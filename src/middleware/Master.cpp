/**
 * @file Master.cpp
 * @brief Implementation of Main Control System Orchestrator
 */

#include "Master.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <signal.h>

// Global pointer for signal handler access
static Master* g_masterInstance = nullptr;
static pthread_cond_t* g_condTime = nullptr;
static pthread_mutex_t* g_mutexTime = nullptr;

// SIGALRM handler - triggers tSig thread
static void sigalrmHandler(int sig) {
    (void)sig;
    if (g_condTime && g_mutexTime) {
        pthread_mutex_lock(g_mutexTime);
        pthread_cond_signal(g_condTime);
        pthread_mutex_unlock(g_mutexTime);
    }
}

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

Master::Master(MQueueHandler* queue) 
    : msgQueue(queue)
    , running(false)
    , sensorsCorrecting(false)
    , cameraCaptureCounter(0)
    , readSensorCD(0)
    , cameraCaptureInterval(900)  // 900 tSig activations
    , readSensorInterval(10)      // 10 tSig activations 
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
    // Model located at: /opt/leafsense/leafsense_model.onnx
    mlEngine = new ML("/opt/leafsense", "leafsense_model.onnx");

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
    pthread_create(&tCamera, NULL, tCameraFuncStatic, this);
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
    triggerSignal(&condCam, &mutexCam);
    triggerSignal(&condWH, &mutexWH);
    triggerSignal(&condPHU, &mutexPHU);
    triggerSignal(&condPHD, &mutexPHD);
    triggerSignal(&condN, &mutexN);
    
    // Wait for threads to finish
    pthread_join(tTime, NULL);
    pthread_join(tSig, NULL);
    pthread_join(tCamera, NULL);
}

/* ============================================================================
 * Thread Functions - Timer
 * ============================================================================ */

void Master::tTimeFunc() 
{
    std::cout << "[tTime] Timer thread started (5s interval)" << std::endl;
    
    // Use a dedicated mutex and condition for the timer wait
    pthread_mutex_t timerMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t timerCond = PTHREAD_COND_INITIALIZER;
    
    while (running) {
        // Use pthread_cond_timedwait instead of usleep for proper threading
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;  // Wait for 5 seconds
        
        pthread_mutex_lock(&timerMutex);
        pthread_cond_timedwait(&timerCond, &timerMutex, &ts);
        pthread_mutex_unlock(&timerMutex);
        
        if (!running) break;
        
        std::cout << "[tTime] Timer tick - signaling tSig thread" << std::endl;
        
        // Signal the tSig thread
        pthread_mutex_lock(&mutexTime);
        pthread_cond_signal(&condTime);
        pthread_mutex_unlock(&mutexTime);
    }
    
    pthread_mutex_destroy(&timerMutex);
    pthread_cond_destroy(&timerCond);
    
    std::cout << "[tTime] Timer thread stopped" << std::endl;
}

void Master::tSigFunc() 
{
    std::cout << "[tSig] Thread started, waiting for timer signals..." << std::endl;
    
    while (running) {
        captureSignal(&condTime, &mutexTime);
        if (!running) break;

        std::cout << "[tSig] Tick! SensorCD=" << readSensorCD 
                  << ", CameraCD=" << cameraCaptureCounter << std::endl;

        if (nPump->getState()) {
            triggerSignal(&condN, &mutexN);
            msgQueue->sendMessage("LOG|Maintenance|Nutrients|Auto Off");
        }
        if (phuPump->getState()) {
            triggerSignal(&condPHU, &mutexPHU);
            msgQueue->sendMessage("LOG|Maintenance|pH Up|Auto Off");
        }
        if (phdPump->getState()) {
            triggerSignal(&condPHD, &mutexPHD);
            msgQueue->sendMessage("LOG|Maintenance|pH Down|Auto Off");
        }

        if (readSensorCD <= 0) {
            std::cout << "[tSig] Triggering sensor read" << std::endl;
            readSensorCD = readSensorInterval;
            triggerSignal(&condRS, &mutexRS);
        } else {
            if (sensorsCorrecting) {
                readSensorCD -= 2;
            } else {
                readSensorCD -= 1;
            }
        }
        
        if (cameraCaptureCounter <= 0) {
            std::cout << "[tSig] Triggering camera capture" << std::endl;
            cameraCaptureCounter = cameraCaptureInterval;
            triggerSignal(&condCam, &mutexCam);
        } else {
            cameraCaptureCounter -= 1;
        }
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
        sensorsCorrecting = false;
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
         * Triggers the dedicated camera thread
         * -------------------------------------------------------------------- */
        

        // Get ideal ranges for control decisions
        idealConditions->getTemp(tempRange);
        idealConditions->getPH(phRange);
        idealConditions->getTDS(tdsRange);

        /* --------------------------------------------------------------------
         * Temperature Control (with hysteresis)
         * -------------------------------------------------------------------- */
        std::cout << "[Master] Temp Control: Current=" << t << "°C, Range=[" 
                  << tempRange[0] << "-" << tempRange[1] << "], Heater=" 
                  << (heater->getState() ? "ON" : "OFF") << std::endl;
                  
        if (t < tempRange[0] && !heater->getState()) {
            std::cout << "[Master] Temperature LOW (" << t << " < " << tempRange[0] << ") -> Turning heater ON" << std::endl;
            sensorsCorrecting = true;
            triggerSignal(&condWH, &mutexWH);
        } else if (t > tempRange[1] && heater->getState()) {
            std::cout << "[Master] Temperature HIGH (" << t << " > " << tempRange[1] << ") -> Turning heater OFF" << std::endl;
            triggerSignal(&condWH, &mutexWH);
        }

        /* --------------------------------------------------------------------
         * pH Control
         * -------------------------------------------------------------------- */
        if (p < phRange[0]) {
            sensorsCorrecting = true;
            triggerSignal(&condPHU, &mutexPHU);
        } else if (p > phRange[1]) {
            sensorsCorrecting = true;
            triggerSignal(&condPHD, &mutexPHD);
        }

        /* --------------------------------------------------------------------
         * TDS/Nutrient Control
         * -------------------------------------------------------------------- */
        if (e < tdsRange[0]) {
            sensorsCorrecting = true;
            triggerSignal(&condN, &mutexN);
        }
        
        /* --------------------------------------------------------------------
         * Update Alert LED (kernel module integration)
         * -------------------------------------------------------------------- */
        updateAlertLED();
    }
}

/* ============================================================================
 * Thread Functions - Camera & ML Analysis
 * ============================================================================ */

void Master::tCameraFunc() 
{
    while (running) {
        captureSignal(&condCam, &mutexCam);
        if (!running) break;
        
        std::cout << "[Camera] Capturing photo for ML analysis..." << std::endl;
        std::string photoPath = camera->takePhoto();
        
        if (!photoPath.empty()) {
            // Extract filename from path
            std::string filename = photoPath.substr(photoPath.find_last_of("/") + 1);
            
            // Save image record to database
            std::stringstream imgMsg;
            imgMsg << "IMG|" << filename << "|" << photoPath;
            msgQueue->sendMessage(imgMsg.str());
            
            // Run ML inference on captured image
            MLResult mlResult = mlEngine->analyzeDetailed(photoPath);
            
            // Use do-while(false) pattern to allow early exit for OOD
            do {
                // ============================================================
                // Out-of-Distribution Detection (Non-plant image rejection)
                // ============================================================
                if (!mlResult.isValidPlant) {
                    std::cout << "[Camera] OOD Detection: Image does not appear to be a valid plant" << std::endl;
                    std::cout << "[Camera] Entropy: " << mlResult.entropy 
                              << ", Confidence: " << (mlResult.confidence * 100) << "%" << std::endl;
                    
                    // Save as "Unknown" prediction
                    std::stringstream predMsg;
                    predMsg << "PRED|" << filename << "|Unknown (Not a Plant)|" << mlResult.confidence;
                    msgQueue->sendMessage(predMsg.str());
                    
                    // Log the rejection
                    std::stringstream oodLog;
                    oodLog << "LOG|ML Analysis|Out-of-Distribution Detected"
                           << "|Image: " << filename 
                           << ", Entropy: " << mlResult.entropy 
                           << ", Confidence: " << (mlResult.confidence * 100) << "%";
                    msgQueue->sendMessage(oodLog.str());
                    
                    // Turn LED OFF for OOD (not a valid plant image)
                    setMLAlertLED(false);
                    
                    // Skip further ML processing for this image
                    break;
                }
                
                // Save ML prediction to database (linked to image)
                {
                    std::stringstream predMsg;
                    predMsg << "PRED|" << filename << "|" << mlResult.class_name 
                            << "|" << mlResult.confidence;
                    msgQueue->sendMessage(predMsg.str());
                }
                
                // Also log for history
                {
                    std::stringstream mlLog;
                    mlLog << "LOG|ML Analysis|" << mlResult.class_name 
                          << "|Confidence: " << (mlResult.confidence * 100) << "%";
                    msgQueue->sendMessage(mlLog.str());
                }
                
                std::cout << "[Camera] ML Result: " << mlResult.class_name 
                          << " (" << (mlResult.confidence * 100) << "%)" << std::endl;
            
                // ============================================================
                // LED Alert Control - ON for bad classes, OFF for Healthy
                // Class IDs: 0=Deficiency, 1=Disease, 2=Healthy, 3=Pest
                // ============================================================
                bool isBadClass = (mlResult.class_id != 2);  // Not Healthy
                setMLAlertLED(isBadClass);
                
                // ============================================================
                // Generate Recommendations based on ML prediction (TCDIS6, TCDEF5)
                // ============================================================
                generateMLRecommendation(mlResult, filename);
                
                // ============================================================
                // Multi-class confidence logging (TCDIS7, TCDEF7)
                // ============================================================
                if (mlResult.probs.size() >= 4) {
                    std::cout << "[Camera] All class probabilities:" << std::endl;
                    std::cout << "  - Nutrient Deficiency: " << (mlResult.probs[0] * 100) << "%" << std::endl;
                    std::cout << "  - Disease: " << (mlResult.probs[1] * 100) << "%" << std::endl;
                    std::cout << "  - Healthy: " << (mlResult.probs[2] * 100) << "%" << std::endl;
                    std::cout << "  - Pest Damage: " << (mlResult.probs[3] * 100) << "%" << std::endl;
                    
                    // Log secondary detections above 20% confidence
                    for (size_t i = 0; i < 4; i++) {
                        if ((int)i != mlResult.class_id && mlResult.probs[i] > 0.20f) {
                            std::string secondaryClass;
                            switch(i) {
                                case 0: secondaryClass = "Nutrient Deficiency"; break;
                                case 1: secondaryClass = "Disease"; break;
                                case 2: secondaryClass = "Healthy"; break;
                                case 3: secondaryClass = "Pest Damage"; break;
                            }
                            std::stringstream secLog;
                            secLog << "LOG|ML Analysis|Secondary: " << secondaryClass 
                                   << "|Confidence: " << (mlResult.probs[i] * 100) << "%";
                            msgQueue->sendMessage(secLog.str());
                        }
                    }
                }
                
                // ============================================================
                // Confidence threshold alerting (TCDIS8, TCDEF8)
                // ============================================================
                const float ALERT_THRESHOLD = 0.70f;  // 70% confidence threshold
                if (mlResult.class_id != 2 && mlResult.confidence >= ALERT_THRESHOLD) {  // Not Healthy
                    std::stringstream alertMsg;
                    alertMsg << "ALERT|Critical|" << mlResult.class_name 
                             << " detected with " << (mlResult.confidence * 100) << "% confidence";
                    msgQueue->sendMessage(alertMsg.str());
                    std::cout << "[Camera] ALERT: " << mlResult.class_name 
                              << " detected above threshold!" << std::endl;
                }
                
                // ============================================================
                // Specific Disease/Deficiency Logging (TCDIS9, TCDEF9)
                // ============================================================
                if (mlResult.class_id == 1) {  // Disease
                    std::stringstream diseaseLog;
                    diseaseLog << "LOG|Disease|" << mlResult.class_name 
                               << "|Image: " << filename 
                               << ", Confidence: " << (mlResult.confidence * 100) 
                               << "%, Timestamp: " << time(nullptr);
                    msgQueue->sendMessage(diseaseLog.str());
                } else if (mlResult.class_id == 0) {  // Deficiency
                    // Get current EC for correlation
                    float currentEC = tdsSensor->readSensor();
                    std::stringstream defLog;
                    defLog << "LOG|Deficiency|" << mlResult.class_name 
                           << "|Image: " << filename 
                           << ", Confidence: " << (mlResult.confidence * 100) 
                           << "%, Current EC: " << currentEC << " µS/cm";
                    msgQueue->sendMessage(defLog.str());
                } else if (mlResult.class_id == 3) {  // Pest
                    std::stringstream pestLog;
                    pestLog << "LOG|Disease|Pest Damage"
                            << "|Image: " << filename 
                            << ", Confidence: " << (mlResult.confidence * 100) << "%";
                    msgQueue->sendMessage(pestLog.str());
                }
                
            } while(false);  // End of ML processing block (allows break for OOD skip)
            
        } else {
            std::cerr << "[Camera] Failed to capture photo" << std::endl;
        }
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
        
        phuPump->pump(!phuPump->getState());
        msgQueue->sendMessage(phuPump->getState() 
            ? "LOG|Maintenance|pH Up|Auto" 
            : "LOG|Maintenance|pH Up|Auto");
    }
}

void Master::tPHDFunc() 
{
    while (running) {
        captureSignal(&condPHD, &mutexPHD);
        if (!running) break;
        
        phdPump->pump(!phdPump->getState());
        msgQueue->sendMessage(phdPump->getState() 
            ? "LOG|Maintenance|pH Down|Auto" 
            : "LOG|Maintenance|pH Down|Auto");
    }
}

void Master::tNutrientsFunc() 
{
    while (running) {
        captureSignal(&condN, &mutexN);
        if (!running) break;
        
        nPump->pump(!nPump->getState());
        msgQueue->sendMessage(nPump->getState() 
            ? "LOG|Maintenance|Nutrients|Auto" 
            : "LOG|Maintenance|Nutrients|Auto");
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

void* Master::tCameraFuncStatic(void* arg) { 
    ((Master*)arg)->tCameraFunc(); 
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
    pthread_mutex_init(&mutexCam, NULL);
}

void Master::createConds() 
{
    pthread_cond_init(&condRS, NULL);
    pthread_cond_init(&condTime, NULL);
    pthread_cond_init(&condN, NULL);
    pthread_cond_init(&condPHU, NULL);
    pthread_cond_init(&condPHD, NULL);
    pthread_cond_init(&condWH, NULL);
    pthread_cond_init(&condCam, NULL);
}

void Master::destroyMutexes() 
{
    pthread_mutex_destroy(&mutexRS);
    pthread_mutex_destroy(&mutexTime);
    pthread_mutex_destroy(&mutexN);
    pthread_mutex_destroy(&mutexPHU);
    pthread_mutex_destroy(&mutexPHD);
    pthread_mutex_destroy(&mutexWH);
    pthread_mutex_destroy(&mutexCam);
}

void Master::destroyConds() 
{
    pthread_cond_destroy(&condRS);
    pthread_cond_destroy(&condTime);
    pthread_cond_destroy(&condN);
    pthread_cond_destroy(&condPHU);
    pthread_cond_destroy(&condPHD);
    pthread_cond_destroy(&condWH);
    pthread_cond_destroy(&condCam);
}

/* ============================================================================
 * LED Alert Control - Based on ML Classification
 * ============================================================================ */

void Master::setMLAlertLED(bool alertActive) 
{
    // Control LED via libgpiod (gpioset command) - GPIO 20
    // This is more reliable than the kernel module approach
    const char* cmd = alertActive 
        ? "gpioset gpiochip0 20=1" 
        : "gpioset gpiochip0 20=0";
    
    int result = system(cmd);
    
    if (result != 0) {
        std::cerr << "[LED] Failed to control LED via gpioset" << std::endl;
    } else {
        std::cout << "[LED] Alert LED -> " << (alertActive ? "ON (Bad class detected)" : "OFF") << std::endl;
    }
}

void Master::updateAlertLED() 
{
    // Legacy function - now LED is controlled by ML classification
    // This function can be removed or kept for future sensor-based alerts
}

/* ============================================================================
 * ML Recommendation Generation (TCDIS6, TCDEF5, TCDEF6, TCDEF10)
 * ============================================================================ */

void Master::generateMLRecommendation(const MLResult& mlResult, const std::string& filename)
{
    // Skip recommendation if image is out-of-distribution (not a valid plant)
    if (!mlResult.isValidPlant) {
        std::cout << "[Master] Skipping recommendation - not a valid plant image" << std::endl;
        return;
    }
    
    std::string recType;
    std::string recText;
    
    // Get current sensor values for correlation (TCDEF10)
    float currentEC = tdsSensor->readSensor();
    float currentPH = phSensor->readSensor();
    float currentTemp = tempSensor->readSensor();
    
    // Get ideal ranges for comparison
    float tempRange[2], phRange[2], tdsRange[2];
    idealConditions->getTemp(tempRange);
    idealConditions->getPH(phRange);
    idealConditions->getTDS(tdsRange);
    
    switch (mlResult.class_id) {
        case 0:  // Nutrient Deficiency
            recType = "Deficiency";
            
            // TCDEF6: Specific Nutrient Recommendation based on EC correlation
            if (currentEC < tdsRange[0]) {
                // Low EC indicates general nutrient deficiency
                float deficit = tdsRange[0] - currentEC;
                if (deficit > 300) {
                    recText = "CRITICAL: Severe nutrient deficiency detected. EC is " + 
                              std::to_string((int)currentEC) + " µS/cm (target: " + 
                              std::to_string((int)tdsRange[0]) + "-" + std::to_string((int)tdsRange[1]) + 
                              "). Add complete NPK nutrient solution immediately. Recommend 2-3 doses.";
                } else if (deficit > 150) {
                    recText = "Moderate nutrient deficiency. EC is " + 
                              std::to_string((int)currentEC) + " µS/cm. Add balanced nutrient solution. Recommend 1-2 doses.";
                } else {
                    recText = "Mild nutrient deficiency. EC is " + 
                              std::to_string((int)currentEC) + " µS/cm. Add light nutrient supplement.";
                }
            } else if (currentEC > tdsRange[1]) {
                // High EC but still deficiency - likely specific nutrient missing
                recText = "Possible specific nutrient deficiency despite adequate EC (" + 
                          std::to_string((int)currentEC) + " µS/cm). Check for: "
                          "Iron (Fe) if yellowing between veins, "
                          "Calcium (Ca) if tip burn, "
                          "Magnesium (Mg) if older leaf yellowing. "
                          "Consider foliar spray treatment.";
            } else {
                // EC in range - pH might be locking out nutrients
                if (currentPH < phRange[0] || currentPH > phRange[1]) {
                    recText = "Nutrient lockout suspected due to pH imbalance (current: " + 
                              std::to_string(currentPH).substr(0, 4) + ", target: " + 
                              std::to_string(phRange[0]).substr(0, 3) + "-" + 
                              std::to_string(phRange[1]).substr(0, 3) + "). "
                              "Adjust pH before adding nutrients.";
                } else {
                    recText = "Visual nutrient deficiency detected but EC/pH are normal. "
                              "Monitor for 24h. If symptoms persist, flush system and replenish nutrients.";
                }
            }
            break;
            
        case 1:  // Disease
            recType = "Disease";
            recText = "Disease detected. IMMEDIATE ACTIONS: "
                      "1) Isolate affected plant if possible. "
                      "2) Remove visibly infected leaves. "
                      "3) Apply appropriate fungicide/bactericide. "
                      "4) Improve air circulation. "
                      "5) Reduce humidity if above 70%. "
                      "Current conditions - Temp: " + std::to_string((int)currentTemp) + 
                      "°C, pH: " + std::to_string(currentPH).substr(0, 4) + 
                      ". Monitor closely for 48 hours.";
            break;
            
        case 2:  // Healthy
            recType = "Healthy";
            recText = "Plant appears healthy. Continue current care routine. "
                      "Conditions: Temp " + std::to_string((int)currentTemp) + 
                      "°C, pH " + std::to_string(currentPH).substr(0, 4) + 
                      ", EC " + std::to_string((int)currentEC) + " µS/cm.";
            break;
            
        case 3:  // Pest Damage
            recType = "Pest";
            recText = "Pest damage detected. RECOMMENDED ACTIONS: "
                      "1) Inspect undersides of all leaves for insects. "
                      "2) Look for common pests: aphids, spider mites, thrips, whiteflies. "
                      "3) Apply neem oil or insecticidal soap. "
                      "4) Consider introducing beneficial insects (ladybugs, lacewings). "
                      "5) Yellow sticky traps for monitoring. "
                      "Check again in 3-5 days.";
            break;
            
        default:
            recType = "Unknown";
            recText = "Unknown classification. Manual inspection recommended.";
            break;
    }
    
    // Send recommendation to database
    std::stringstream recMsg;
    recMsg << "REC|" << filename << "|" << recType << "|" << recText 
           << "|" << mlResult.confidence;
    msgQueue->sendMessage(recMsg.str());
    
    // Log recommendation
    std::cout << "[Master] Recommendation (" << recType << "): " 
              << recText.substr(0, 80) << "..." << std::endl;
}