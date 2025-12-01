/**
 * @file IdealConditions.h
 * @brief Ideal Sensor Value Range Storage
 * @layer Middleware
 * 
 * Stores and provides access to the ideal ranges for all monitored
 * parameters. Used by the Master control loop to determine when
 * corrective action is needed.
 */

#ifndef IDEALCONDITIONS_H
#define IDEALCONDITIONS_H

/**
 * @class IdealConditions
 * @brief Stores ideal parameter ranges for plant growth
 * 
 * Default values are configured for Lettuce cultivation:
 * - TDS: 560-840 ppm
 * - pH: 5.5-6.5
 * - Temperature: 18-24°C
 */
class IdealConditions {
private:
    /* ------------------------------------------------------------------------
     * Parameter Ranges
     * ------------------------------------------------------------------------ */
    float tds_min, tds_max;    ///< TDS/EC range (ppm)
    float ph_min, ph_max;      ///< pH range
    float temp_min, temp_max;  ///< Temperature range (°C)

public:
    /* ------------------------------------------------------------------------
     * Constructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructor with default Lettuce values
     */
    IdealConditions();

    /* ------------------------------------------------------------------------
     * Getters (Array-based for efficiency)
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Gets TDS range
     * @param[out] r Array of 2 floats: [min, max]
     */
    void getTDS(float* r);
    
    /**
     * @brief Gets pH range
     * @param[out] r Array of 2 floats: [min, max]
     */
    void getPH(float* r);
    
    /**
     * @brief Gets temperature range
     * @param[out] r Array of 2 floats: [min, max]
     */
    void getTemp(float* r);

    /* ------------------------------------------------------------------------
     * Setters
     * ------------------------------------------------------------------------ */
    
    void setTDS(float min, float max);
    void setPH(float min, float max);
    void setTemp(float min, float max);
};

#endif // IDEALCONDITIONS_H