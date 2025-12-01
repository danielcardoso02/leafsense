#ifndef HEALTH_DISPLAY_H
#define HEALTH_DISPLAY_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>

class HealthDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit HealthDisplay(QWidget *parent = nullptr);
    void update_values(int health_score, const QString &status);
    void apply_theme();

private:
    void setup_ui();

    QLabel *score_value;
    QProgressBar *score_bar;
    QLabel *status_value;
    QLabel *title;
    QLabel *score_label;
    QLabel *status_label;
};

#endif // HEALTH_DISPLAY_H
