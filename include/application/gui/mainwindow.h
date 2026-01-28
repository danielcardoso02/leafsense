/**
 * @file mainwindow.h
 * @author Daniel Cardoso, Marco Costa
 * @brief Main Application Window for LeafSense Hydroponic Monitoring System
 * @layer Application/GUI
 * 
 * This is the primary dashboard window that displays:
 * - Real-time sensor data (pH, Temperature, EC)
 * - Plant health status and assessments
 * - System alerts and notifications
 * - Navigation to secondary windows (Analytics, Settings, Logs, Info)
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QColor>

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "theme/theme_manager.h"
#include "leafsense_data_bridge.h"

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @struct Plant
 * @brief Represents a monitored plant with its ideal growing conditions
 */
struct Plant {
    int id;                     ///< Unique plant identifier
    QString name;               ///< Display name (e.g., "Lettuce")
    QString species;            ///< Plant species
    QString location;           ///< Physical location in system
    bool is_active;             ///< Whether plant is currently monitored
    QColor status_color;        ///< Visual status indicator color
    
    // Ideal growing conditions
    double ideal_ph_min;        ///< Minimum acceptable pH
    double ideal_ph_max;        ///< Maximum acceptable pH
    double ideal_temp_min;      ///< Minimum temperature (°C)
    double ideal_temp_max;      ///< Maximum temperature (°C)
    double ideal_ec_min;        ///< Minimum EC (µS/cm)
    double ideal_ec_max;        ///< Maximum EC (µS/cm)
};

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */
class SensorsDisplay;
class HealthDisplay;
class AlertsDisplay;

/* ============================================================================
 * MainWindow Class
 * ============================================================================ */

/**
 * @class MainWindow
 * @brief Central dashboard for the LeafSense monitoring application
 * 
 * Displays real-time sensor data, health assessments, and alerts.
 * Provides navigation to all secondary application windows.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /* ------------------------------------------------------------------------
     * Public Methods - Session Management
     * ------------------------------------------------------------------------ */
    void set_logged_in_user(const QString &user);
    void set_login_time(const QString &time);
    void set_selected_plant(const Plant &plant);

private slots:
    /* ------------------------------------------------------------------------
     * Data Update Slots (connected to LeafSenseDataBridge signals)
     * ------------------------------------------------------------------------ */
    void on_sensor_data_updated(const SensorData &data);
    void on_health_updated(const HealthAssessment &health);
    void on_alert_received(const SystemAlert &alert);
    void on_time_updated(const QString &time);

    /* ------------------------------------------------------------------------
     * Button Click Handlers
     * ------------------------------------------------------------------------ */
    void on_analytics_button_clicked();
    void on_settings_button_clicked();
    void on_info_button_clicked();
    void on_logs_button_clicked();
    void on_logout_button_clicked();

    /* ------------------------------------------------------------------------
     * Theme Management
     * ------------------------------------------------------------------------ */
    void apply_theme_deferred();

private:
    /* ------------------------------------------------------------------------
     * Private Methods - Initialization
     * ------------------------------------------------------------------------ */
    void setup_ui();
    void setup_connections();
    void setup_logo();

    /* ------------------------------------------------------------------------
     * Private Methods - Theme
     * ------------------------------------------------------------------------ */
    void apply_theme();
    void reload_logo_for_theme();
    void refresh_child_widgets_theme();

    /* ------------------------------------------------------------------------
     * UI Components - Header
     * ------------------------------------------------------------------------ */
    QLabel *logo_label;         ///< LeafSense logo display
    QLabel *plant_name_label;   ///< Current plant name
    QLabel *time_label;         ///< UTC time display
    QLabel *greeting_label;     ///< User greeting ("Hi, username")
    QLabel *status_indicator;   ///< System status dot (green/yellow/red)

    /* ------------------------------------------------------------------------
     * UI Components - Display Widgets
     * ------------------------------------------------------------------------ */
    SensorsDisplay *sensors_display;    ///< pH, Temp, EC readings
    HealthDisplay *health_display;      ///< Plant health score
    AlertsDisplay *alerts_display;      ///< Active alerts panel

    /* ------------------------------------------------------------------------
     * UI Components - Navigation Buttons
     * ------------------------------------------------------------------------ */
    QPushButton *logs_btn;
    QPushButton *analytics_btn;
    QPushButton *settings_btn;
    QPushButton *info_btn;
    QPushButton *logout_btn;

    /* ------------------------------------------------------------------------
     * Data Members
     * ------------------------------------------------------------------------ */
    ThemeManager &theme_mgr;            ///< Reference to singleton theme manager
    LeafSenseDataBridge *data_bridge;   ///< Database connection bridge

    QString logged_in_user;             ///< Current user's username
    QString login_time;                 ///< Session start time
    Plant current_plant;                ///< Currently selected plant
};

#endif // MAINWINDOW_H
