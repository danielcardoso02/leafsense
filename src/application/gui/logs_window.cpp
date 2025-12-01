#include "../include/application/gui/logs_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QFrame>

LogsWindow::LogsWindow(const QString &plant_name, QWidget *parent)
    : QDialog(parent), plant_name(plant_name), current_filter("Alert")
{
    setWindowTitle("Logs");
    setFixedSize(480, 320);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    setup_ui();
    apply_theme();
    load_logs();
    display_filtered_logs("Alert");
}

LogsWindow::~LogsWindow() {}

void LogsWindow::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(10);

    // Filters
    QHBoxLayout *filter_layout = new QHBoxLayout();
    filter_layout->setSpacing(5);
    auto createFilterBtn = [&](QString text) {
        QPushButton* btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setFixedHeight(28);
        return btn;
    };
    alerts_btn = createFilterBtn("Alerts"); alerts_btn->setChecked(true);
    connect(alerts_btn, &QPushButton::clicked, this, &LogsWindow::on_alerts_button_clicked);
    diseases_btn = createFilterBtn("Diseases");
    connect(diseases_btn, &QPushButton::clicked, this, &LogsWindow::on_diseases_button_clicked);
    deficiencies_btn = createFilterBtn("Deficiencies");
    connect(deficiencies_btn, &QPushButton::clicked, this, &LogsWindow::on_deficiencies_button_clicked);
    maintenance_btn = createFilterBtn("Maint.");
    connect(maintenance_btn, &QPushButton::clicked, this, &LogsWindow::on_maintenance_button_clicked);

    filter_layout->addWidget(alerts_btn); filter_layout->addWidget(diseases_btn);
    filter_layout->addWidget(deficiencies_btn); filter_layout->addWidget(maintenance_btn);
    main_layout->addLayout(filter_layout);

    // Scroll
    logs_scroll_area = new QScrollArea(); logs_scroll_area->setWidgetResizable(true);
    logs_container = new QWidget();
    QVBoxLayout *logs_layout = new QVBoxLayout(logs_container);
    logs_layout->setSpacing(5); logs_layout->setContentsMargins(5, 5, 5, 5);
    logs_container->setLayout(logs_layout);
    logs_scroll_area->setWidget(logs_container);
    main_layout->addWidget(logs_scroll_area, 1);

    // Close Button
    QHBoxLayout *bl = new QHBoxLayout(); bl->addStretch();
    cancel_btn = new QPushButton("Close"); cancel_btn->setFixedSize(100, 32);
    connect(cancel_btn, &QPushButton::clicked, this, &LogsWindow::on_cancel_button_clicked);
    bl->addWidget(cancel_btn); bl->addStretch();
    main_layout->addLayout(bl);
}

void LogsWindow::apply_theme() {}

void LogsWindow::load_logs()
{
    all_logs.clear();
    all_logs.append({"2025-11-02 20:15:00 UTC", "Alert", "pH too high", "Current pH: 8.2 (Ideal: 6.5-7.5)"});
    all_logs.append({"2025-11-02 18:30:00 UTC", "Alert", "Temperature warning", "Current Temp: 26.5C (Ideal: 20-25C)"});
    all_logs.append({"2025-11-01 14:20:00 UTC", "Alert", "EC low alert", "Current EC: 1200 ppm"});
    all_logs.append({"2025-11-01 10:00:00 UTC", "Disease", "No diseases detected", "Plant health check: All parameters normal"});
    all_logs.append({"2025-10-31 15:30:00 UTC", "Disease", "Minor leaf spot", "Location: Lower leaves"});
    all_logs.append({"2025-11-02 16:00:00 UTC", "Deficiency", "Nitrogen deficiency", "Symptoms: Yellowing"});
    all_logs.append({"2025-11-02 22:00:00 UTC", "Maintenance", "Nutrient dosing", "Amount: 50ml"});
}

void LogsWindow::display_filtered_logs(const QString &filter_type)
{
    QLayoutItem *item;
    while ((item = logs_container->layout()->takeAt(0)) != nullptr) { delete item->widget(); delete item; }
    const ThemeColors& colors = ThemeManager::instance().get_colors();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(logs_container->layout());
    QFont type_font; type_font.setBold(true); type_font.setPointSize(9);

    for (const LogEntry &log : all_logs) {
        if (log.type == filter_type) {
            QWidget *w = new QWidget(); QVBoxLayout *vl = new QVBoxLayout(w);
            vl->setContentsMargins(5,5,5,5); vl->setSpacing(2);

            QLabel *ts = new QLabel(log.timestamp); ts->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(colors.text_secondary.name()));
            vl->addWidget(ts);

            // RESTORED LABEL LOGIC
            QLabel *type_lbl = new QLabel(log.type); type_lbl->setFont(type_font);
            QColor tc = (log.type == "Alert" || log.type == "Disease") ? colors.alert_red : (log.type == "Maintenance") ? colors.primary_green : colors.accent_orange;
            type_lbl->setStyleSheet(QString("color: %1;").arg(tc.name()));
            vl->addWidget(type_lbl);

            QLabel *msg = new QLabel(log.message); msg->setStyleSheet(QString("color: %1; font-weight: bold;").arg(colors.text_primary.name()));
            vl->addWidget(msg);
            QLabel *det = new QLabel(log.details); det->setWordWrap(true); det->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(colors.text_secondary.name()));
            vl->addWidget(det);

            QFrame *l = new QFrame(); l->setFrameShape(QFrame::HLine); l->setFrameShadow(QFrame::Sunken); l->setStyleSheet(QString("background-color: %1;").arg(colors.border_light.name()));
            vl->addWidget(l);
            layout->addWidget(w);
        }
    }
    layout->addStretch();
}

void LogsWindow::on_alerts_button_clicked() {
    alerts_btn->setChecked(true); diseases_btn->setChecked(false); deficiencies_btn->setChecked(false); maintenance_btn->setChecked(false);
    display_filtered_logs("Alert");
}
void LogsWindow::on_diseases_button_clicked() {
    alerts_btn->setChecked(false); diseases_btn->setChecked(true); deficiencies_btn->setChecked(false); maintenance_btn->setChecked(false);
    display_filtered_logs("Disease");
}
void LogsWindow::on_deficiencies_button_clicked() {
    alerts_btn->setChecked(false); diseases_btn->setChecked(false); deficiencies_btn->setChecked(true); maintenance_btn->setChecked(false);
    display_filtered_logs("Deficiency");
}
void LogsWindow::on_maintenance_button_clicked() {
    alerts_btn->setChecked(false); diseases_btn->setChecked(false); deficiencies_btn->setChecked(false); maintenance_btn->setChecked(true);
    display_filtered_logs("Maintenance");
}
void LogsWindow::on_cancel_button_clicked() { reject(); }
