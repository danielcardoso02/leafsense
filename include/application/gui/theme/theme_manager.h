/**
 * @file theme_manager.h
 * @brief Application Theme and Settings Manager (Singleton)
 * @layer Application/GUI/Theme
 * 
 * Manages global application appearance and sensor parameters.
 * Implements the Singleton pattern for consistent theme access.
 */

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QString>
#include <QColor>

/* ============================================================================
 * Application Includes
 * ============================================================================ */
#include "colors.h"

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @enum ThemeMode
 * @brief Available application themes
 */
enum class ThemeMode {
    LIGHT,  ///< Light theme with white backgrounds
    DARK    ///< Dark theme with dark backgrounds
};

/**
 * @enum PlantStatus
 * @brief Plant health status levels
 */
enum class PlantStatus {
    HEALTHY,   ///< All parameters within range
    WARNING,   ///< Parameters approaching limits
    CRITICAL,  ///< Parameters outside safe range
    UNKNOWN    ///< Status cannot be determined
};

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @struct SensorParameters
 * @brief Ideal sensor value ranges for the current plant
 */
struct SensorParameters {
    double ph_min;    ///< Minimum acceptable pH
    double ph_max;    ///< Maximum acceptable pH
    double temp_min;  ///< Minimum temperature (°C)
    double temp_max;  ///< Maximum temperature (°C)
    double ec_min;    ///< Minimum EC (µS/cm)
    double ec_max;    ///< Maximum EC (µS/cm)
};

/**
 * @struct AppSettings
 * @brief Complete application settings bundle
 */
struct AppSettings {
    SensorParameters sensor_params;  ///< Sensor threshold values
    bool notifications_enabled;      ///< Whether to show notifications
    ThemeMode theme_mode;            ///< Current theme selection
};

/**
 * @class ThemeManager
 * @brief Singleton manager for application theme and settings
 * 
 * Provides:
 * - Theme switching (Light/Dark)
 * - Global stylesheet generation
 * - Sensor parameter storage
 * - Color palette access
 */
class ThemeManager
{
public:
    /* ------------------------------------------------------------------------
     * Singleton Access
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Gets the singleton instance
     * @return Reference to the ThemeManager instance
     */
    static ThemeManager& instance();

    /* ------------------------------------------------------------------------
     * Theme Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Sets the application theme
     * @param mode Theme to apply (LIGHT or DARK)
     */
    void set_theme(ThemeMode mode);
    
    /**
     * @brief Gets the current theme mode
     * @return Current ThemeMode
     */
    ThemeMode get_current_theme() const;

    /* ------------------------------------------------------------------------
     * Color Access
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Gets the current theme color palette
     * @return Reference to ThemeColors struct
     */
    const ThemeColors& get_colors() const;
    
    /**
     * @brief Gets the status indicator color
     * @param status Plant health status
     * @return QColor for the status
     */
    QColor get_status_color(PlantStatus status) const;
    
    /**
     * @brief Gets the button pressed state color
     * @return Darker shade of primary green
     */
    QColor get_button_pressed_color() const;

    /* ------------------------------------------------------------------------
     * Stylesheet Generation
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Generates the global application stylesheet
     * @return QString containing complete CSS
     */
    QString get_stylesheet() const;
    
    /**
     * @brief Gets stylesheet for button states
     * @param is_active Whether button is in active/pressed state
     * @return Button-specific stylesheet
     */
    QString get_button_stylesheet(bool is_active) const;
    
    /**
     * @brief Gets the path to the theme-appropriate logo
     * @return Resource path to logo image
     */
    QString get_logo_path() const;

    /* ------------------------------------------------------------------------
     * Sensor Parameters
     * ------------------------------------------------------------------------ */
    
    void set_sensor_parameters(const SensorParameters &params);
    SensorParameters get_sensor_parameters() const;

    /* ------------------------------------------------------------------------
     * Notifications
     * ------------------------------------------------------------------------ */
    
    void set_notifications_enabled(bool enabled);
    bool get_notifications_enabled() const;

private:
    /* ------------------------------------------------------------------------
     * Private Constructor (Singleton)
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Private constructor with default values
     */
    ThemeManager() : current_theme(ThemeMode::LIGHT), notifications_enabled(true)
    {
        // Default sensor parameters (Lettuce)
        sensor_params.ph_min = 6.5;
        sensor_params.ph_max = 7.5;
        sensor_params.temp_min = 20.0;
        sensor_params.temp_max = 25.0;
        sensor_params.ec_min = 1400;
        sensor_params.ec_max = 2000;
    }

    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    
    void setup_light_theme();  ///< Configures light theme colors
    void setup_dark_theme();   ///< Configures dark theme colors

    /* ------------------------------------------------------------------------
     * Private Members
     * ------------------------------------------------------------------------ */
    ThemeMode current_theme;          ///< Active theme mode
    ThemeColors colors;               ///< Current color palette
    SensorParameters sensor_params;   ///< Sensor threshold values
    bool notifications_enabled;       ///< Notification preference
};

#endif // THEME_MANAGER_H
