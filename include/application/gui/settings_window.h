#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "theme/theme_manager.h"

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    SensorParameters get_sensor_parameters() const;
    void set_sensor_parameters(const SensorParameters &params);

private slots:
    void on_save_button_clicked();
    void on_cancel_button_clicked();

private:
    void setup_ui();
    void apply_theme();
    void load_settings();

    QSpinBox *sensor_poll_interval;
    QSpinBox *control_interval;

    QDoubleSpinBox *ideal_ph_min;
    QDoubleSpinBox *ideal_ph_max;
    QDoubleSpinBox *ideal_temp_min;
    QDoubleSpinBox *ideal_temp_max;
    QDoubleSpinBox *ideal_ec_min;
    QDoubleSpinBox *ideal_ec_max;

    QComboBox *theme_selector;
    QCheckBox *notifications_enabled;

    QPushButton *save_btn;
    QPushButton *cancel_btn;

    SensorParameters current_params;
};

#endif // SETTINGS_WINDOW_H
