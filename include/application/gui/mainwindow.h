#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QColor>
#include "theme/theme_manager.h"
#include "leafsense_data_bridge.h"

struct Plant {
    int id;
    QString name;
    QString species;
    QString location;
    bool is_active;
    QColor status_color;
    double ideal_ph_min;
    double ideal_ph_max;
    double ideal_temp_min;
    double ideal_temp_max;
    double ideal_ec_min;
    double ideal_ec_max;
};

class SensorsDisplay;
class HealthDisplay;
class AlertsDisplay;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void set_logged_in_user(const QString &user);
    void set_login_time(const QString &time);
    void set_selected_plant(const Plant &plant);

private slots:
    void on_sensor_data_updated(const SensorData &data);
    void on_health_updated(const HealthAssessment &health);
    void on_alert_received(const SystemAlert &alert);
    void on_time_updated(const QString &time);

    void on_analytics_button_clicked();
    void on_settings_button_clicked();
    void on_info_button_clicked();
    void on_logs_button_clicked();
    void on_logout_button_clicked();

    void apply_theme_deferred();

private:
    void setup_ui();
    void setup_connections();
    void apply_theme();
    void reload_logo_for_theme();
    void setup_logo();
    void refresh_child_widgets_theme();

    QLabel *logo_label;
    QLabel *plant_name_label;
    QLabel *time_label;
    QLabel *greeting_label;
    QLabel *status_indicator;

    SensorsDisplay *sensors_display;
    HealthDisplay *health_display;
    AlertsDisplay *alerts_display;

    QPushButton *logs_btn;
    QPushButton *analytics_btn;
    QPushButton *settings_btn;
    QPushButton *info_btn;
    QPushButton *logout_btn;

    ThemeManager& theme_mgr;
    LeafSenseDataBridge *data_bridge;

    QString logged_in_user;
    QString login_time;
    Plant current_plant;
};

#endif // MAINWINDOW_H
