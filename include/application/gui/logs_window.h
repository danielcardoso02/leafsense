/**
 * @file logs_window.h
 * @author Daniel Cardoso, Marco Costa
 * @brief System Logs Viewer Dialog
 * @layer Application/GUI
 * 
 * Modal dialog for viewing and filtering system logs.
 * Supports categorization by type: Alerts, Diseases, Deficiencies, Maintenance.
 */

#ifndef LOGS_WINDOW_H
#define LOGS_WINDOW_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QWidget>
#include <QString>
#include <QVector>

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @struct LogEntry
 * @brief Represents a single log record
 */
struct LogEntry {
    QString timestamp;  ///< When the log was created (UTC)
    QString type;       ///< Log category (Alert, Disease, Deficiency, Maintenance)
    QString message;    ///< Brief description
    QString details;    ///< Extended information
};

/**
 * @class LogsWindow
 * @brief Dialog for viewing filtered system logs
 * 
 * Features:
 * - Filter buttons for log categories
 * - Scrollable log list
 * - Color-coded entries by type
 */
class LogsWindow : public QDialog
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructs the logs viewer dialog
     * @param plant_name Name of the plant being monitored
     * @param parent Parent widget (optional)
     */
    explicit LogsWindow(const QString &plant_name, QWidget *parent = nullptr);
    
    ~LogsWindow();

private slots:
    /* ------------------------------------------------------------------------
     * Filter Button Handlers
     * ------------------------------------------------------------------------ */
    void on_alerts_button_clicked();       ///< Shows Alert type logs
    void on_diseases_button_clicked();     ///< Shows Disease type logs
    void on_maintenance_button_clicked();  ///< Shows Maintenance type logs
    void on_deficiencies_button_clicked(); ///< Shows Deficiency type logs
    void on_cancel_button_clicked();       ///< Closes the dialog

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    void setup_ui();                       ///< Creates UI components
    void apply_theme();                    ///< Applies current theme
    void load_logs();                      ///< Loads log entries from database
    
    /**
     * @brief Displays logs filtered by type
     * @param filter_type Log category to display
     */
    void display_filtered_logs(const QString &filter_type);

    /* ------------------------------------------------------------------------
     * State
     * ------------------------------------------------------------------------ */
    QString plant_name;          ///< Current plant name
    QString current_filter;      ///< Active filter type
    QVector<LogEntry> all_logs;  ///< All loaded log entries

    /* ------------------------------------------------------------------------
     * UI Components - Filter Buttons
     * ------------------------------------------------------------------------ */
    QPushButton *alerts_btn;       ///< Filter: Alerts
    QPushButton *diseases_btn;     ///< Filter: Diseases
    QPushButton *deficiencies_btn; ///< Filter: Deficiencies
    QPushButton *maintenance_btn;  ///< Filter: Maintenance
    QPushButton *cancel_btn;       ///< Close button

    /* ------------------------------------------------------------------------
     * UI Components - Log Display
     * ------------------------------------------------------------------------ */
    QScrollArea *logs_scroll_area; ///< Scrollable container
    QWidget *logs_container;       ///< Widget holding log entries
};

#endif // LOGS_WINDOW_H
