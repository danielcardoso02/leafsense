/**
 * @file TDS.h
 * @brief TDS/EC Sensor Driver (Analog via ADC)
 * @layer Drivers/Sensors
 * 
 * Reads Total Dissolved Solids (TDS) or Electrical Conductivity (EC)
 * from an analog probe connected to an ADC channel.
 */

#ifndef TDS_H
#define TDS_H

/* ============================================================================
 * Includes
 * ============================================================================ */
#include "Sensor.h"
#include "ADC.h"

/**
 * @class TDS
 * @brief TDS/EC sensor driver (analog probe)
 * 
 * Real mode: Reads ADC voltage and converts to ppm
 * Mock mode: Returns random values around 1200-1400 ppm
 */
class TDS : public Sensor {
public:
    /**
     * @brief Constructs TDS sensor
     * @param a Pointer to ADC driver
     * @param c ADC channel number (0-3)
     */
    TDS(ADC* a, int c) : adc(a), channel(c) {}
    
    /**
     * @brief Reads current TDS/EC value
     * @return TDS in ppm (parts per million)
     */
    float readSensor() override;

private:
    ADC* adc;      ///< Pointer to ADC driver
    int channel;   ///< ADC channel (0-3)
};

#endif // TDS_H