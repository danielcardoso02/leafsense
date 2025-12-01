#ifndef LOGS_WINDOW_H
#define LOGS_WINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QString>
#include <QVector>

struct LogEntry {
    QString timestamp;
    QString type;
    QString message;
    QString details;
};

class LogsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LogsWindow(const QString &plant_name, QWidget *parent = nullptr);
    ~LogsWindow();

private slots:
    void on_alerts_button_clicked();
    void on_diseases_button_clicked();
    void on_maintenance_button_clicked();
    void on_deficiencies_button_clicked();
    void on_cancel_button_clicked();

private:
    void setup_ui();
    void apply_theme();
    void load_logs();
    void display_filtered_logs(const QString &filter_type);

    QString plant_name;

    QPushButton *alerts_btn;
    QPushButton *diseases_btn;
    QPushButton *deficiencies_btn;
    QPushButton *maintenance_btn;
    QPushButton *cancel_btn;

    QScrollArea *logs_scroll_area;
    QWidget *logs_container;

    QVector<LogEntry> all_logs;
    QString current_filter;
};

#endif // LOGS_WINDOW_H
