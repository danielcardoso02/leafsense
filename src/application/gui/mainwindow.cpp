/**
 * @file mainwindow.cpp
 * @brief Implementation of the Main Application Window
 * @author Daniel Cardoso, Marco Costa
 * @layer Application/GUI
 */

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "../include/application/gui/mainwindow.h"
#include "../include/application/gui/sensors_display.h"
#include "../include/application/gui/health_display.h"
#include "../include/application/gui/alerts_display.h"
#include "../include/application/gui/logs_window.h"
#include "../include/application/gui/analytics_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include "../include/application/gui/settings_window.h"
#include "../include/application/gui/info_window.h"

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
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

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , theme_mgr(ThemeManager::instance())
    , data_bridge(nullptr)
    , logged_in_user("Unknown")
    , login_time("")
{
     /**
      * @brief Constructs the main application window.
      * @param parent Parent widget (optional)
      */
    // Window configuration (480x320 for Raspberry Pi touchscreen)
    setFixedSize(480, 320);
    setWindowTitle("LeafSense - Hydroponic Monitoring");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - QRect(0, 0, 480, 320).center());

    // Central widget setup
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // Initialize UI components
    setup_ui();

    // Initialize data bridge and connect signals
    data_bridge = new LeafSenseDataBridge(this);
    setup_connections();
    
    if (!data_bridge->initialize()) {
        qWarning() << "Failed to initialize LeafSense data bridge";
    }

    // Defer theme application to ensure all widgets are ready
    QTimer::singleShot(0, this, &MainWindow::apply_theme_deferred);

    qDebug() << "MainWindow initialized successfully";
}

MainWindow::~MainWindow()
{
    /**
     * @brief Destructor for MainWindow. Cleans up resources.
     */
    if (data_bridge) {
        delete data_bridge;
    }
}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

void MainWindow::setup_ui()
{
     /**
      * @brief Sets up the user interface components and layout.
      */
    QWidget *central = centralWidget();
    QVBoxLayout *main_layout = new QVBoxLayout(central);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(8);

    /* ------------------------------------------------------------------------
     * Header Section
     * ------------------------------------------------------------------------ */
    QHBoxLayout *header_layout = new QHBoxLayout();
    header_layout->setSpacing(10);

    // Logo
    logo_label = new QLabel();
    logo_label->setFixedSize(50, 40);
    logo_label->setAlignment(Qt::AlignCenter);

    // Plant name
    plant_name_label = new QLabel("Plant Name");
    QFont plant_font = plant_name_label->font();
    plant_font.setPointSize(11);
    plant_font.setBold(true);
    plant_name_label->setFont(plant_font);

    header_layout->addWidget(logo_label, 0);
    header_layout->addWidget(plant_name_label, 0);
    header_layout->addStretch();

    // Time display
    time_label = new QLabel("HH:MM:SS UTC");
    QFont time_font = time_label->font();
    time_font.setPointSize(9);
    time_label->setFont(time_font);

    // Status indicator (colored dot)
    status_indicator = new QLabel("•");
    status_indicator->setFont(QFont("Arial", 16, QFont::Bold));

    header_layout->addWidget(time_label, 0);
    header_layout->addWidget(status_indicator, 0);
    main_layout->addLayout(header_layout, 0);

    /* ------------------------------------------------------------------------
     * Main Display Widgets
     * ------------------------------------------------------------------------ */
    sensors_display = new SensorsDisplay();
    main_layout->addWidget(sensors_display, 2);

    health_display = new HealthDisplay();
    main_layout->addWidget(health_display, 2);

    alerts_display = new AlertsDisplay();
    main_layout->addWidget(alerts_display, 1);

    /* ------------------------------------------------------------------------
     * Footer Section - Navigation Buttons
     * ------------------------------------------------------------------------ */
    QHBoxLayout *footer_wrapper = new QHBoxLayout();
    footer_wrapper->addStretch();

    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(6);

    // User greeting
    greeting_label = new QLabel("Hey, admin!");
    greeting_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    button_layout->addWidget(greeting_label);
    button_layout->addSpacing(10);

    // Navigation buttons (75px width standard)
    logs_btn = new QPushButton("Logs");
    logs_btn->setFixedWidth(75);
    connect(logs_btn, &QPushButton::clicked, this, &MainWindow::on_logs_button_clicked);

    analytics_btn = new QPushButton("Analytics");
    analytics_btn->setFixedWidth(75);
    connect(analytics_btn, &QPushButton::clicked, this, &MainWindow::on_analytics_button_clicked);

    settings_btn = new QPushButton("Settings");
    settings_btn->setFixedWidth(75);
    connect(settings_btn, &QPushButton::clicked, this, &MainWindow::on_settings_button_clicked);

    info_btn = new QPushButton("Info");
    info_btn->setFixedWidth(75);
    connect(info_btn, &QPushButton::clicked, this, &MainWindow::on_info_button_clicked);

    logout_btn = new QPushButton("Logout");
    logout_btn->setFixedWidth(75);
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
    /**
     * @brief Connects signals from the data bridge to update slots.
     */
    if (!data_bridge) return;

    // Connect data bridge signals to update slots
    connect(data_bridge, &LeafSenseDataBridge::sensor_data_updated, 
            this, &MainWindow::on_sensor_data_updated);
    connect(data_bridge, &LeafSenseDataBridge::health_updated, 
            this, &MainWindow::on_health_updated);
    connect(data_bridge, &LeafSenseDataBridge::alert_received, 
            this, &MainWindow::on_alert_received);
    connect(data_bridge, &LeafSenseDataBridge::time_updated, 
            this, &MainWindow::on_time_updated);
}

/* ============================================================================
 * Theme Management
 * ============================================================================ */

void MainWindow::apply_theme_deferred()
{
     /**
      * @brief Applies the theme after all widgets are initialized.
      */
    setup_logo();
    apply_theme();
    refresh_child_widgets_theme();
}

void MainWindow::apply_theme()
{
     /**
      * @brief Applies the current theme to the main window and widgets.
      */
    const ThemeColors &colors = theme_mgr.get_colors();

    // Apply global stylesheet
    qApp->setStyleSheet(theme_mgr.get_stylesheet());

    // Apply specific widget styles
    plant_name_label->setStyleSheet(
        QString("color: %1; font-weight: bold;").arg(colors.primary_green.name()));
    time_label->setStyleSheet(
        QString("color: %1;").arg(colors.text_secondary.name()));
    greeting_label->setStyleSheet(
        QString("color: %1;").arg(colors.text_secondary.name()));

    // Note: Button styling is handled by ThemeManager global stylesheet

    // Force style refresh
    centralWidget()->style()->unpolish(centralWidget());
    centralWidget()->style()->polish(centralWidget());
}

void MainWindow::refresh_child_widgets_theme()
{
     /**
      * @brief Refreshes the theme for all child widgets.
      */
    if (sensors_display) sensors_display->apply_theme();
    if (health_display)  health_display->apply_theme();
    if (alerts_display)  alerts_display->apply_theme();
}

void MainWindow::reload_logo_for_theme()
{
     /**
      * @brief Reloads the logo and reapplies the theme when it changes.
      */
    setup_logo();
    apply_theme();
    refresh_child_widgets_theme();
    
    // Deferred style refresh
    QTimer::singleShot(0, this, [this]() {
        centralWidget()->style()->unpolish(centralWidget());
        centralWidget()->style()->polish(centralWidget());
        centralWidget()->update();
    });
}

void MainWindow::setup_logo()
{
     /**
      * @brief Sets up the logo based on the current theme.
      */
    ThemeManager &tm = ThemeManager::instance();
    
    // Select logo based on current theme
    QString logo_filename = (tm.get_current_theme() == ThemeMode::LIGHT) 
        ? "logo_leafsense.png" 
        : "logo_leafsense_dark.png";
    
    // Try Qt resource first, then fall back to local path
    QString logo_path = ":/images/images/" + logo_filename;
    QPixmap logo_pixmap(logo_path);
    if (logo_pixmap.isNull()) {
        logo_pixmap = QPixmap("./images/" + logo_filename);
    }
    if (!logo_pixmap.isNull()) {
        logo_label->setPixmap(logo_pixmap.scaled(50, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo_label->setText("LS");  // Fallback text
    }
}

/* ============================================================================
 * Data Update Handlers
 * ============================================================================ */

void MainWindow::on_sensor_data_updated(const SensorData &data)
{
    /**
     * @brief Slot to handle updated sensor data.
     * @param data Latest sensor readings
     */
    if (sensors_display) {
        sensors_display->update_values(data.ph, data.temperature, data.ec);
    }
}

void MainWindow::on_health_updated(const HealthAssessment &health)
{
    /**
     * @brief Slot to handle updated health assessment.
     * @param health Latest health assessment
     */
    if (health_display) {
        QString status_str;
        switch (health.status) {
            case PlantHealthStatus::HEALTHY:  status_str = "Healthy";  break;
            case PlantHealthStatus::WARNING:  status_str = "Warning";  break;
            case PlantHealthStatus::CRITICAL: status_str = "Critical"; break;
            default:                          status_str = "Unknown";  break;
        }
        health_display->update_values(health.overall_score, status_str);
    }
}

void MainWindow::on_alert_received(const SystemAlert &alert)
{
     /**
      * @brief Slot to handle received system alerts.
      * @param alert Latest system alert
      */
    // Update the alerts widget
    alerts_display->update_alerts();

    const ThemeColors &c = theme_mgr.get_colors();

    // Update header status indicator color
    QColor statusColor;
    switch (alert.severity) {
        case PlantHealthStatus::CRITICAL: statusColor = c.status_critical; break;
        case PlantHealthStatus::WARNING:  statusColor = c.status_warning;  break;
        default:                          statusColor = c.status_healthy;  break;
    }
    status_indicator->setStyleSheet(
        QString("color: %1; font-weight: bold;").arg(statusColor.name()));

    // Update Logs button with notification indicator
    QString baseStyle = theme_mgr.get_button_stylesheet(false);
    
    if (alert.severity != PlantHealthStatus::HEALTHY) {
        // Alert state: Red border + "(!) " indicator
        QString alertStyle = baseStyle + QString("border: 2px solid %1; color: %1;")
            .arg(c.alert_red.name());
        logs_btn->setStyleSheet(alertStyle);
        logs_btn->setText("Logs (!)");
    } else {
        // Normal state: Reset to standard style
        logs_btn->setStyleSheet(baseStyle);
        logs_btn->setText("Logs");
    }
}

void MainWindow::on_time_updated(const QString &time)
{
     /**
      * @brief Slot to handle updated time string.
      * @param time Current UTC time string
      */
    time_label->setText(time);
}

/* ============================================================================
 * Session Management
 * ============================================================================ */

void MainWindow::set_logged_in_user(const QString &user)
{
     /**
      * @brief Sets the currently logged-in user and updates greeting.
      * @param user Username string
      */
    logged_in_user = user;
    greeting_label->setText("Hi, " + user);
}

void MainWindow::set_login_time(const QString &time)
{
     /**
      * @brief Sets the login time for the current session.
      * @param time Login time string
      */
    login_time = time;
}

void MainWindow::set_selected_plant(const Plant &plant)
{
     /**
      * @brief Sets the selected plant and updates the display.
      * @param plant Plant struct with details
      */
    current_plant = plant;
    plant_name_label->setText(plant.name);
}

/* ============================================================================
 * Navigation Button Handlers
 * ============================================================================ */

void MainWindow::on_analytics_button_clicked()
{
     /**
      * @brief Opens the analytics window dialog.
      */
    AnalyticsWindow w(data_bridge, this);
    w.exec();
}

void MainWindow::on_settings_button_clicked()
{
    /**
     * @brief Opens the settings window dialog and reloads theme if changed.
     */
    SettingsWindow w(this);
    ThemeMode before = ThemeManager::instance().get_current_theme();
    
    if (w.exec() == QDialog::Accepted) {
        if (before != ThemeManager::instance().get_current_theme()) {
            reload_logo_for_theme();
        }
    }
}

void MainWindow::on_info_button_clicked()
{
     /**
      * @brief Opens the info window dialog.
      */
    InfoWindow w(logged_in_user, login_time, this);
    w.exec();
}

void MainWindow::on_logs_button_clicked()
{
     /**
      * @brief Opens the logs window dialog.
      */
    LogsWindow w(current_plant.name, this);
    w.exec();
}

void MainWindow::on_logout_button_clicked()
{
     /**
      * @brief Handles logout button click and shows confirmation dialog.
      */
    // Create confirmation dialog
    QDialog d(this);
    d.setWindowTitle("Logout");
    d.setFixedSize(300, 150);
    d.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    QVBoxLayout *l = new QVBoxLayout(&d);
    l->setContentsMargins(20, 20, 20, 20);

    QLabel *t = new QLabel("Are you sure you want to logout?");
    t->setAlignment(Qt::AlignCenter);
    t->setWordWrap(true);
    l->addWidget(t);

    QHBoxLayout *b = new QHBoxLayout();
    QPushButton *n = new QPushButton("No");
    QPushButton *y = new QPushButton("Yes");
    n->setFixedSize(80, 32);
    y->setFixedSize(80, 32);
    b->addWidget(n);
    b->addWidget(y);
    l->addLayout(b);

    connect(n, &QPushButton::clicked, &d, &QDialog::reject);
    connect(y, &QPushButton::clicked, &d, &QDialog::accept);

    d.move(mapToGlobal(rect().center()) - d.rect().center());

    if (d.exec() == QDialog::Accepted) {
        this->close();
    }
}
