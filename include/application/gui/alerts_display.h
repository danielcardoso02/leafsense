#ifndef ALERTS_DISPLAY_H
#define ALERTS_DISPLAY_H

#include <QWidget>
#include <QLabel>

class AlertsDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit AlertsDisplay(QWidget *parent = nullptr);
    void update_alerts();
    void apply_theme();

private:
    void setup_ui();

    QLabel *status_label;
    QLabel *alert_label;
    QLabel *time_label;
};

#endif // ALERTS_DISPLAY_H
