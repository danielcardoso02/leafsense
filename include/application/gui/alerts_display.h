/**
 * @file alerts_display.h
 * @author Daniel Cardoso, Marco Costa
 * @brief Alert Status Display Widget
 * @layer Application/GUI
 * 
 * Displays the current system alert status with visual indicators.
 * Shows alert type, message, and timestamp of last check.
 */

#ifndef ALERTS_DISPLAY_H
#define ALERTS_DISPLAY_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QWidget>
#include <QLabel>

/**
 * @class AlertsDisplay
 * @brief Widget for displaying system alerts and status
 * 
 * Provides a compact view of the current alert status with:
 * - Status indicator (colored dot)
 * - Alert message text
 * - Timestamp of last system check
 */
class AlertsDisplay : public QWidget
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructs the alerts display widget
     * @param parent Parent widget (optional)
     */
    explicit AlertsDisplay(QWidget *parent = nullptr);

    /* ------------------------------------------------------------------------
     * Public Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Updates the alert display with current system status
     */
    void update_alerts();
    
    /**
     * @brief Applies the current theme colors to all UI elements
     */
    void apply_theme();

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Creates and configures all UI components
     */
    void setup_ui();

    /* ------------------------------------------------------------------------
     * UI Components
     * ------------------------------------------------------------------------ */
    QLabel *status_label;   ///< Colored status indicator dot
    QLabel *alert_label;    ///< Alert message text
    QLabel *time_label;     ///< Last check timestamp
};

#endif // ALERTS_DISPLAY_H
