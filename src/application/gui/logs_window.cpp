/**
 * @file logs_window.cpp
 * @brief Implementation of System Logs Viewer Dialog
 */

#include "../include/application/gui/logs_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QScroller>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

/**
 * @brief Constructs the LogsWindow dialog.
 * @param plant_name Name of the plant for which logs are displayed.
 * @param parent Parent widget.
 * @author Daniel Cardoso, Marco Costa
 */
LogsWindow::LogsWindow(const QString &plant_name, QWidget *parent)
    : QDialog(parent)
    , plant_name(plant_name)
    , current_filter("Alert")
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

/**
 * @brief Destructor for LogsWindow.
 * @author Daniel Cardoso, Marco Costa
 */
LogsWindow::~LogsWindow() {}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Sets up the UI components for the logs window.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(10);

    /* ------------------------------------------------------------------------
     * Filter Buttons Row
     * ------------------------------------------------------------------------ */
    QHBoxLayout *filter_layout = new QHBoxLayout();
    filter_layout->setSpacing(5);
    
    // Helper lambda to create filter buttons
    auto createFilterBtn = [&](QString text) {
        QPushButton *btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setFixedHeight(28);
        return btn;
    };
    
    alerts_btn = createFilterBtn("Alerts");
    alerts_btn->setChecked(true);
    connect(alerts_btn, &QPushButton::clicked, this, &LogsWindow::on_alerts_button_clicked);
    
    diseases_btn = createFilterBtn("Diseases");
    connect(diseases_btn, &QPushButton::clicked, this, &LogsWindow::on_diseases_button_clicked);
    
    deficiencies_btn = createFilterBtn("Deficiencies");
    connect(deficiencies_btn, &QPushButton::clicked, this, &LogsWindow::on_deficiencies_button_clicked);
    
    maintenance_btn = createFilterBtn("Maint.");
    connect(maintenance_btn, &QPushButton::clicked, this, &LogsWindow::on_maintenance_button_clicked);

    filter_layout->addWidget(alerts_btn);
    filter_layout->addWidget(diseases_btn);
    filter_layout->addWidget(deficiencies_btn);
    filter_layout->addWidget(maintenance_btn);
    main_layout->addLayout(filter_layout);

    /* ------------------------------------------------------------------------
     * Scrollable Log List
     * ------------------------------------------------------------------------ */
    logs_scroll_area = new QScrollArea();
    logs_scroll_area->setWidgetResizable(true);
    
    logs_container = new QWidget();
    QVBoxLayout *logs_layout = new QVBoxLayout(logs_container);
    logs_layout->setSpacing(5);
    logs_layout->setContentsMargins(5, 5, 5, 5);
    logs_container->setLayout(logs_layout);
    
    logs_scroll_area->setWidget(logs_container);
    
    // Enable touch scrolling
    QScroller::grabGesture(logs_scroll_area->viewport(), QScroller::TouchGesture);
    
    main_layout->addWidget(logs_scroll_area, 1);

    /* ------------------------------------------------------------------------
     * Close Button
     * ------------------------------------------------------------------------ */
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    cancel_btn = new QPushButton("Close");
    cancel_btn->setFixedSize(100, 32);
    connect(cancel_btn, &QPushButton::clicked, this, &LogsWindow::on_cancel_button_clicked);
    
    button_layout->addWidget(cancel_btn);
    button_layout->addStretch();
    main_layout->addLayout(button_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies the current theme to the logs window.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::apply_theme() 
{
    // Handled by ThemeManager global stylesheet
}

/* ============================================================================
 * Data Loading
 * ============================================================================ */

/**
 * @brief Loads log entries from the database (currently mock data).
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::load_logs()
{
    all_logs.clear();
    
    // TODO: Load from database (logs table)
    // Currently using mock data for demonstration
    
    all_logs.append({"2025-11-02 20:15:00 UTC", "Alert", "pH too high", "Current pH: 8.2 (Ideal: 6.5-7.5)"});
    all_logs.append({"2025-11-02 18:30:00 UTC", "Alert", "Temperature warning", "Current Temp: 26.5C (Ideal: 20-25C)"});
    all_logs.append({"2025-11-01 14:20:00 UTC", "Alert", "EC low alert", "Current EC: 1200 ppm"});
    all_logs.append({"2025-11-01 10:00:00 UTC", "Disease", "No diseases detected", "Plant health check: All parameters normal"});
    all_logs.append({"2025-10-31 15:30:00 UTC", "Disease", "Minor leaf spot", "Location: Lower leaves"});
    all_logs.append({"2025-11-02 16:00:00 UTC", "Deficiency", "Nitrogen deficiency", "Symptoms: Yellowing"});
    all_logs.append({"2025-11-02 22:00:00 UTC", "Maintenance", "Nutrient dosing", "Amount: 50ml"});
}

/* ============================================================================
 * Log Display
 * ============================================================================ */

/**
 * @brief Displays logs filtered by type.
 * @param filter_type Type of log to display (Alert, Disease, Deficiency, Maintenance).
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::display_filtered_logs(const QString &filter_type)
{
    // Clear existing log entries
    QLayoutItem *item;
    while ((item = logs_container->layout()->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    const ThemeColors &colors = ThemeManager::instance().get_colors();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(logs_container->layout());
    
    QFont type_font;
    type_font.setBold(true);
    type_font.setPointSize(9);

    // Add entries matching filter
    for (const LogEntry &log : all_logs) {
        if (log.type == filter_type) {
            QWidget *entry_widget = new QWidget();
            QVBoxLayout *entry_layout = new QVBoxLayout(entry_widget);
            entry_layout->setContentsMargins(5, 5, 5, 5);
            entry_layout->setSpacing(2);

            // Timestamp
            QLabel *timestamp_label = new QLabel(log.timestamp);
            timestamp_label->setStyleSheet(QString("color: %1; font-size: 8pt;")
                .arg(colors.text_secondary.name()));
            entry_layout->addWidget(timestamp_label);

            // Type label with color coding
            QLabel *type_label = new QLabel(log.type);
            type_label->setFont(type_font);
            QColor type_color;
            if (log.type == "Alert" || log.type == "Disease") {
                type_color = colors.alert_red;
            } else if (log.type == "Maintenance") {
                type_color = colors.primary_green;
            } else {
                type_color = colors.accent_orange;
            }
            type_label->setStyleSheet(QString("color: %1;").arg(type_color.name()));
            entry_layout->addWidget(type_label);

            // Message
            QLabel *message_label = new QLabel(log.message);
            message_label->setStyleSheet(QString("color: %1; font-weight: bold;")
                .arg(colors.text_primary.name()));
            entry_layout->addWidget(message_label);
            
            // Details
            QLabel *details_label = new QLabel(log.details);
            details_label->setWordWrap(true);
            details_label->setStyleSheet(QString("color: %1; font-size: 8pt;")
                .arg(colors.text_secondary.name()));
            entry_layout->addWidget(details_label);

            // Separator line
            QFrame *separator = new QFrame();
            separator->setFrameShape(QFrame::HLine);
            separator->setFrameShadow(QFrame::Sunken);
            separator->setStyleSheet(QString("background-color: %1;")
                .arg(colors.border_light.name()));
            entry_layout->addWidget(separator);
            
            layout->addWidget(entry_widget);
        }
    }
    
    layout->addStretch();
}

/* ============================================================================
 * Filter Button Handlers
 * ============================================================================ */

/**
 * @brief Handles Alerts filter button click.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::on_alerts_button_clicked() 
{
    alerts_btn->setChecked(true);
    diseases_btn->setChecked(false);
    deficiencies_btn->setChecked(false);
    maintenance_btn->setChecked(false);
    display_filtered_logs("Alert");
}

/**
 * @brief Handles Diseases filter button click.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::on_diseases_button_clicked() 
{
    alerts_btn->setChecked(false);
    diseases_btn->setChecked(true);
    deficiencies_btn->setChecked(false);
    maintenance_btn->setChecked(false);
    display_filtered_logs("Disease");
}

/**
 * @brief Handles Deficiencies filter button click.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::on_deficiencies_button_clicked() 
{
    alerts_btn->setChecked(false);
    diseases_btn->setChecked(false);
    deficiencies_btn->setChecked(true);
    maintenance_btn->setChecked(false);
    display_filtered_logs("Deficiency");
}

/**
 * @brief Handles Maintenance filter button click.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::on_maintenance_button_clicked() 
{
    alerts_btn->setChecked(false);
    diseases_btn->setChecked(false);
    deficiencies_btn->setChecked(false);
    maintenance_btn->setChecked(true);
    display_filtered_logs("Maintenance");
}

/**
 * @brief Handles Close button click.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::on_cancel_button_clicked() 
{ 
    reject(); 
}
