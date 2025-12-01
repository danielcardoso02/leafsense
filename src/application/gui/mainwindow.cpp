#include "../include/application/gui/mainwindow.h"
#include "../include/application/gui/sensors_display.h"
#include "../include/application/gui/health_display.h"
#include "../include/application/gui/alerts_display.h"
#include "../include/application/gui/logs_window.h"
#include "../include/application/gui/analytics_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include "../include/application/gui/settings_window.h"
#include "../include/application/gui/info_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QApplication>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QCoreApplication>
#include <QScreen>
#include <QTimer>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , theme_mgr(ThemeManager::instance())
    , data_bridge(nullptr)
    , logged_in_user("Unknown")
    , login_time("")
{
    setFixedSize(480, 320);
    setWindowTitle("LeafSense - Hydroponic Monitoring");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - QRect(0, 0, 480, 320).center());

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    setup_ui();

    data_bridge = new LeafSenseDataBridge(this);
    setup_connections();
    if (!data_bridge->initialize()) qWarning() << "Failed to initialize LeafSense data bridge";

    QTimer::singleShot(0, this, &MainWindow::apply_theme_deferred);

    qDebug() << "MainWindow initialized successfully";
}

MainWindow::~MainWindow()
{
    if (data_bridge) delete data_bridge;
}

void MainWindow::setup_ui()
{
    QWidget *central = centralWidget();
    QVBoxLayout *main_layout = new QVBoxLayout(central);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(8);

    // Header
    QHBoxLayout *header_layout = new QHBoxLayout();
    header_layout->setSpacing(10);

    logo_label = new QLabel();
    logo_label->setFixedSize(50, 40);
    logo_label->setAlignment(Qt::AlignCenter);

    plant_name_label = new QLabel("Plant Name");
    QFont plant_font = plant_name_label->font();
    plant_font.setPointSize(11); plant_font.setBold(true);
    plant_name_label->setFont(plant_font);

    header_layout->addWidget(logo_label, 0);
    header_layout->addWidget(plant_name_label, 0);
    header_layout->addStretch();

    time_label = new QLabel("HH:MM:SS UTC");
    QFont time_font = time_label->font(); time_font.setPointSize(9);
    time_label->setFont(time_font);

    status_indicator = new QLabel("•");
    status_indicator->setFont(QFont("Arial", 16, QFont::Bold));

    header_layout->addWidget(time_label, 0);
    header_layout->addWidget(status_indicator, 0);
    main_layout->addLayout(header_layout, 0);

    // Displays
    sensors_display = new SensorsDisplay(); main_layout->addWidget(sensors_display, 2);
    health_display = new HealthDisplay(); main_layout->addWidget(health_display, 2);
    alerts_display = new AlertsDisplay(); main_layout->addWidget(alerts_display, 1);

    // Footer
    QHBoxLayout *footer_wrapper = new QHBoxLayout();
    footer_wrapper->addStretch();

    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(6);

    greeting_label = new QLabel("Hey, admin!");
    greeting_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    button_layout->addWidget(greeting_label);
    button_layout->addSpacing(10);

    // Buttons (75px width standard for main window)
    logs_btn = new QPushButton("Logs"); logs_btn->setFixedWidth(75);
    connect(logs_btn, &QPushButton::clicked, this, &MainWindow::on_logs_button_clicked);

    analytics_btn = new QPushButton("Analytics"); analytics_btn->setFixedWidth(75);
    connect(analytics_btn, &QPushButton::clicked, this, &MainWindow::on_analytics_button_clicked);

    settings_btn = new QPushButton("Settings"); settings_btn->setFixedWidth(75);
    connect(settings_btn, &QPushButton::clicked, this, &MainWindow::on_settings_button_clicked);

    info_btn = new QPushButton("Info"); info_btn->setFixedWidth(75);
    connect(info_btn, &QPushButton::clicked, this, &MainWindow::on_info_button_clicked);

    logout_btn = new QPushButton("Logout"); logout_btn->setFixedWidth(75);
    connect(logout_btn, &QPushButton::clicked, this, &MainWindow::on_logout_button_clicked);

    button_layout->addWidget(logs_btn);
    button_layout->addWidget(analytics_btn);
    button_layout->addWidget(settings_btn);
    button_layout->addWidget(info_btn);
    button_layout->addWidget(logout_btn);

    footer_wrapper->addLayout(button_layout);
    footer_wrapper->addStretch();
    main_layout->addLayout(footer_wrapper);
}

void MainWindow::setup_connections()
{
    if (!data_bridge) return;
    connect(data_bridge, &LeafSenseDataBridge::sensor_data_updated, this, &MainWindow::on_sensor_data_updated);
    connect(data_bridge, &LeafSenseDataBridge::health_updated, this, &MainWindow::on_health_updated);
    connect(data_bridge, &LeafSenseDataBridge::alert_received, this, &MainWindow::on_alert_received);
    connect(data_bridge, &LeafSenseDataBridge::time_updated, this, &MainWindow::on_time_updated);
}

void MainWindow::apply_theme_deferred()
{
    setup_logo();
    apply_theme();
    refresh_child_widgets_theme();
}

void MainWindow::apply_theme()
{
    const ThemeColors& colors = theme_mgr.get_colors();

    qApp->setStyleSheet(theme_mgr.get_stylesheet());

    plant_name_label->setStyleSheet(QString("color: %1; font-weight: bold;").arg(colors.primary_green.name()));
    time_label->setStyleSheet(QString("color: %1;").arg(colors.text_secondary.name()));
    greeting_label->setStyleSheet(QString("color: %1;").arg(colors.text_secondary.name()));

    // FIX: Removed local button styling overrides.
    // Now buttons use the Global ThemeManager style (same as sub-windows).

    centralWidget()->style()->unpolish(centralWidget());
    centralWidget()->style()->polish(centralWidget());
}

void MainWindow::refresh_child_widgets_theme()
{
    if (sensors_display) sensors_display->apply_theme();
    if (health_display) health_display->apply_theme();
    if (alerts_display) alerts_display->apply_theme();
}

void MainWindow::reload_logo_for_theme()
{
    setup_logo();
    apply_theme();
    refresh_child_widgets_theme();
    QTimer::singleShot(0, this, [this]() {
        centralWidget()->style()->unpolish(centralWidget());
        centralWidget()->style()->polish(centralWidget());
        centralWidget()->update();
    });
}

void MainWindow::setup_logo()
{
    ThemeManager& tm = ThemeManager::instance();
    QString logo_filename = (tm.get_current_theme() == ThemeMode::LIGHT) ? "logo_leafsense.png" : "logo_leafsense_dark.png";
    QString logo_path = "/home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/LeafSenseUI/resources/images/" + logo_filename;
    QPixmap logo_pixmap(logo_path);
    if (!logo_pixmap.isNull()) {
        logo_label->setPixmap(logo_pixmap.scaled(50, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo_label->setText("LS");
    }
}

// Event Handlers
void MainWindow::on_sensor_data_updated(const SensorData &data) { if (sensors_display) sensors_display->update_values(data.ph, data.temperature, data.ec); }
void MainWindow::on_health_updated(const HealthAssessment &health) {
    if (health_display) {
        QString s = (health.status == PlantHealthStatus::HEALTHY) ? "Healthy" : (health.status == PlantHealthStatus::WARNING) ? "Warning" : "Critical";
        health_display->update_values(health.overall_score, s);
    }
}
void MainWindow::on_alert_received(const SystemAlert &alert) {
    if (alerts_display) alerts_display->update_alerts();
    QColor c = (alert.severity == PlantHealthStatus::HEALTHY) ? theme_mgr.get_colors().status_healthy :
               (alert.severity == PlantHealthStatus::WARNING) ? theme_mgr.get_colors().status_warning : theme_mgr.get_colors().status_critical;
    status_indicator->setStyleSheet(QString("color: %1; font-weight: bold;").arg(c.name()));
}
void MainWindow::on_time_updated(const QString &time) { time_label->setText(time); }
void MainWindow::set_logged_in_user(const QString &user) { logged_in_user = user; greeting_label->setText("Hi, " + user); }
void MainWindow::set_login_time(const QString &time) { login_time = time; }
void MainWindow::set_selected_plant(const Plant &plant) { current_plant = plant; plant_name_label->setText(plant.name); }
void MainWindow::on_analytics_button_clicked() { AnalyticsWindow w(data_bridge, this); w.exec(); }
void MainWindow::on_settings_button_clicked() {
    SettingsWindow w(this);
    ThemeMode before = ThemeManager::instance().get_current_theme();
    if (w.exec() == QDialog::Accepted) { if (before != ThemeManager::instance().get_current_theme()) reload_logo_for_theme(); }
}
void MainWindow::on_info_button_clicked() { InfoWindow w(logged_in_user, login_time, this); w.exec(); }
void MainWindow::on_logs_button_clicked() { LogsWindow w(current_plant.name, this); w.exec(); }
void MainWindow::on_logout_button_clicked() {
    QDialog d(this); d.setWindowTitle("Logout"); d.setFixedSize(300, 150); d.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    QVBoxLayout *l = new QVBoxLayout(&d); l->setContentsMargins(20, 20, 20, 20);
    QLabel *t = new QLabel("Are you sure you want to logout?"); t->setAlignment(Qt::AlignCenter); t->setWordWrap(true); l->addWidget(t);
    QHBoxLayout *b = new QHBoxLayout();
    QPushButton *n = new QPushButton("No"); QPushButton *y = new QPushButton("Yes");
    n->setFixedSize(80, 32); y->setFixedSize(80, 32); // Uniform size
    b->addWidget(n); b->addWidget(y); l->addLayout(b);
    connect(n, &QPushButton::clicked, &d, &QDialog::reject); connect(y, &QPushButton::clicked, &d, &QDialog::accept);
    d.move(mapToGlobal(rect().center()) - d.rect().center());
    if (d.exec() == QDialog::Accepted) this->close();
}
