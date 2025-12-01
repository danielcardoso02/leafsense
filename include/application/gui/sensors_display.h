/**
 * @file sensors_display.h
 * @brief Sensor Data Display Widget for LeafSense Dashboard
 * @layer Application/GUI
 * 
 * Displays real-time sensor readings:
 * - pH level (0-14 scale)
 * - Water temperature (°C)
 * - Electrical conductivity (EC in ppm)
 */

#ifndef SENSORS_DISPLAY_H
#define SENSORS_DISPLAY_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QWidget>
#include <QLabel>

/* ============================================================================
 * SensorsDisplay Class
 * ============================================================================ */

/**
 * @class SensorsDisplay
 * @brief Widget that displays current sensor readings on the dashboard
 * 
 * Shows pH, temperature, and EC values with consistent styling.
 * Values are updated via the update_values() method called from MainWindow.
 */
class SensorsDisplay : public QWidget
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor
     * ------------------------------------------------------------------------ */
    explicit SensorsDisplay(QWidget *parent = nullptr);

    /* ------------------------------------------------------------------------
     * Public Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Update displayed sensor values
     * @param ph pH reading (displayed with 2 decimal places)
     * @param temperature Temperature in Celsius (displayed with 2 decimal places)
     * @param ec Electrical conductivity in ppm (displayed as integer)
     */
    void update_values(double ph, double temperature, double ec);
    
    /**
     * @brief Apply current theme colors to widget
     */
    void apply_theme();

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    void setup_ui();

    /* ------------------------------------------------------------------------
     * UI Components
     * ------------------------------------------------------------------------ */
    QLabel *title;          ///< Section title ("Sensor Data")
    
    QLabel *ph_label;       ///< "pH:" label
    QLabel *ph_value;       ///< pH value display
    
    QLabel *temp_label;     ///< "Temp:" label
    QLabel *temp_value;     ///< Temperature value display
    
    QLabel *ec_label;       ///< "EC:" label
    QLabel *ec_value;       ///< EC value display
};

#endif // SENSORS_DISPLAY_H
