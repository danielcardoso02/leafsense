/**
 * @file Sensor.h
 * @brief Abstract Base Class for All Sensors
 * @layer Drivers/Sensors
 * 
 * Defines the common interface for all sensor implementations.
 * Concrete sensors (Temp, PH, TDS) inherit from this class.
 */

#ifndef SENSOR_H
#define SENSOR_H

/**
 * @class Sensor
 * @brief Abstract sensor interface
 * 
 * Provides:
 * - Pure virtual readSensor() method
 * - Correction mode flag for faster polling during adjustments
 */
class Sensor {
protected:
    float realValue;   ///< Last read sensor value
    bool correcting;   ///< Fast-poll mode during corrections

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    Sensor() : realValue(0), correcting(false) {}
    virtual ~Sensor() {}

    /* ------------------------------------------------------------------------
     * Abstract Interface
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Reads the current sensor value
     * @return Sensor reading in appropriate units
     */
    virtual float readSensor() = 0;

    /* ------------------------------------------------------------------------
     * Correction Mode
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Sets correction mode for faster polling
     * @param c true to enable fast polling
     */
    void setTime(bool c) { correcting = c; }
};

#endif // SENSOR_H