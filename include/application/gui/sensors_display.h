#ifndef SENSORS_DISPLAY_H
#define SENSORS_DISPLAY_H

#include <QWidget>
#include <QLabel>

class SensorsDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit SensorsDisplay(QWidget *parent = nullptr);
    void update_values(double ph, double temperature, double ec);
    void apply_theme();

private:
    void setup_ui();

    QLabel *ph_value;
    QLabel *temp_value;
    QLabel *ec_value;
    QLabel *title;
    QLabel *ph_label;
    QLabel *temp_label;
    QLabel *ec_label;
};

#endif // SENSORS_DISPLAY_H
