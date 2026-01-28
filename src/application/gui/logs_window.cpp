/**
 * @file logs_window.cpp
 * @brief Implementation of System Logs Viewer Dialog
 */

#include "../include/application/gui/logs_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include "middleware/dbManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QScroller>
#include <QCoreApplication>
#include <QDebug>

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
 * @brief Loads log entries from the database.
 * @author Daniel Cardoso, Marco Costa
 */
void LogsWindow::load_logs()
{
    all_logs.clear();
    
    // Open database connection
    QString dbPath = QCoreApplication::applicationDirPath() + "/leafsense.db";
    dbManager db(dbPath.toStdString());
    
    // Load from logs table - maps log_type to our display categories
    // Database log_type: 'Disease', 'Deficiency', 'Maintenance', 'ML Analysis'
    // We map 'ML Analysis' to appropriate category based on content
    DBResult res = db.read(
        "SELECT timestamp, log_type, message, details FROM logs "
        "ORDER BY timestamp DESC LIMIT 100;");
    
    qDebug() << "[LogsWindow] Loaded" << res.rows.size() << "log entries";
    
    for (const auto& row : res.rows) {
        if (row.size() >= 4) {
            LogEntry entry;
            entry.timestamp = QString::fromStdString(row[0]);
            entry.message = QString::fromStdString(row[2]);
            entry.details = QString::fromStdString(row[3]);
            
            // Map log_type to display category
            QString dbType = QString::fromStdString(row[1]);
            if (dbType == "Disease" || dbType == "Pest Damage") {
                entry.type = "Disease";
            } else if (dbType == "Deficiency") {
                entry.type = "Deficiency";
            } else if (dbType == "Maintenance") {
                entry.type = "Maintenance";
            } else if (dbType == "ML Analysis") {
                // Check message content to categorize
                if (entry.message.contains("Disease") || entry.message.contains("Pest")) {
                    entry.type = "Disease";
                } else if (entry.message.contains("Deficiency")) {
                    entry.type = "Deficiency";
                } else {
                    entry.type = "Maintenance";  // Healthy checks go to maintenance
                }
            } else {
                entry.type = "Maintenance";  // Default
            }
            
            all_logs.append(entry);
        }
    }
    
    // Also load alerts (they go to Alerts tab)
    DBResult alertRes = db.read(
        "SELECT timestamp, type, message, details FROM alerts "
        "ORDER BY timestamp DESC LIMIT 50;");
    
    qDebug() << "[LogsWindow] Loaded" << alertRes.rows.size() << "alert entries";
    
    for (const auto& row : alertRes.rows) {
        if (row.size() >= 3) {
            LogEntry entry;
            entry.timestamp = QString::fromStdString(row[0]);
            entry.type = "Alert";
            entry.message = QString::fromStdString(row[2]);
            entry.details = row.size() >= 4 ? QString::fromStdString(row[3]) : "";
            all_logs.append(entry);
        }
    }
    
    qDebug() << "[LogsWindow] Total entries:" << all_logs.size();
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
