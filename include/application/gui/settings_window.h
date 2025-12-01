/**
 * @file settings_window.h
 * @brief Application Settings Dialog
 * @layer Application/GUI
 * 
 * Modal dialog for configuring sensor parameters, display options, and theme.
 */

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>

/* ============================================================================
 * Application Includes
 * ============================================================================ */
#include "theme/theme_manager.h"

/**
 * @class SettingsWindow
 * @brief Dialog for configuring application settings
 * 
 * Configurable options:
 * - Sensor ideal ranges (pH, Temperature, EC)
 * - Theme selection (Light/Dark)
 * - Notification preferences
 */
class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    /* ------------------------------------------------------------------------
     * Public Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Gets the current sensor parameter settings
     * @return SensorParameters struct with all values
     */
    SensorParameters get_sensor_parameters() const;
    
    /**
     * @brief Sets the sensor parameter UI values
     * @param params Values to populate in the UI
     */
    void set_sensor_parameters(const SensorParameters &params);

private slots:
    /* ------------------------------------------------------------------------
     * Button Handlers
     * ------------------------------------------------------------------------ */
    void on_save_button_clicked();    ///< Saves settings and closes
    void on_cancel_button_clicked();  ///< Discards changes and closes

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    void setup_ui();      ///< Creates UI components
    void apply_theme();   ///< Applies current theme
    void load_settings(); ///< Loads current settings into UI

    /* ------------------------------------------------------------------------
     * UI Components - Sensor Parameters
     * ------------------------------------------------------------------------ */
    QSpinBox *sensor_poll_interval;    ///< Sensor reading interval (seconds)
    QSpinBox *control_interval;        ///< Control loop interval (seconds)
    
    QDoubleSpinBox *ideal_ph_min;      ///< pH minimum threshold
    QDoubleSpinBox *ideal_ph_max;      ///< pH maximum threshold
    QDoubleSpinBox *ideal_temp_min;    ///< Temperature minimum (°C)
    QDoubleSpinBox *ideal_temp_max;    ///< Temperature maximum (°C)
    QDoubleSpinBox *ideal_ec_min;      ///< EC minimum (µS/cm)
    QDoubleSpinBox *ideal_ec_max;      ///< EC maximum (µS/cm)

    /* ------------------------------------------------------------------------
     * UI Components - Display Options
     * ------------------------------------------------------------------------ */
    QComboBox *theme_selector;         ///< Light/Dark theme dropdown
    QCheckBox *notifications_enabled;  ///< Enable/disable notifications

    /* ------------------------------------------------------------------------
     * UI Components - Buttons
     * ------------------------------------------------------------------------ */
    QPushButton *save_btn;    ///< Save settings button
    QPushButton *cancel_btn;  ///< Cancel button

    /* ------------------------------------------------------------------------
     * State
     * ------------------------------------------------------------------------ */
    SensorParameters current_params;   ///< Current working values
};

#endif // SETTINGS_WINDOW_H
