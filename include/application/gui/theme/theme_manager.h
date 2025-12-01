#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QString>
#include <QColor>
#include "colors.h"

enum class ThemeMode {
    LIGHT,
    DARK
};

enum class PlantStatus {
    HEALTHY,
    WARNING,
    CRITICAL,
    UNKNOWN
};

struct SensorParameters {
    double ph_min;
    double ph_max;
    double temp_min;
    double temp_max;
    double ec_min;
    double ec_max;
};

struct AppSettings {
    SensorParameters sensor_params;
    bool notifications_enabled;
    ThemeMode theme_mode;
};

class ThemeManager
{
public:
    static ThemeManager& instance();

    void set_theme(ThemeMode mode);
    ThemeMode get_current_theme() const;

    const ThemeColors& get_colors() const;
    QColor get_status_color(PlantStatus status) const;
    QColor get_button_pressed_color() const;

    QString get_stylesheet() const;
    QString get_button_stylesheet(bool is_active) const;

    QString get_logo_path() const;

    void set_sensor_parameters(const SensorParameters &params);
    SensorParameters get_sensor_parameters() const;

    void set_notifications_enabled(bool enabled);
    bool get_notifications_enabled() const;

private:
    ThemeManager() : current_theme(ThemeMode::LIGHT), notifications_enabled(true)
    {
        sensor_params.ph_min = 6.5;
        sensor_params.ph_max = 7.5;
        sensor_params.temp_min = 20.0;
        sensor_params.temp_max = 25.0;
        sensor_params.ec_min = 1400;
        sensor_params.ec_max = 2000;
    }

    void setup_light_theme();
    void setup_dark_theme();

    ThemeMode current_theme;
    ThemeColors colors;
    SensorParameters sensor_params;
    bool notifications_enabled;
};

#endif // THEME_MANAGER_H
