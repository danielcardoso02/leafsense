/**
 * @file health_display.h
 * @brief Plant Health Assessment Display Widget
 * @layer Application/GUI
 * 
 * Displays the overall plant health score and status.
 * Uses color-coded indicators based on health thresholds.
 */

#ifndef HEALTH_DISPLAY_H
#define HEALTH_DISPLAY_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QWidget>
#include <QLabel>
#include <QProgressBar>

/**
 * @class HealthDisplay
 * @brief Widget for displaying plant health assessment
 * 
 * Shows:
 * - Numeric health score (0-100%)
 * - Visual progress bar
 * - Status text (Healthy/Warning/Critical)
 * 
 * Color coding:
 * - Green (>=80%): Healthy
 * - Orange (60-79%): Warning
 * - Red (<60%): Critical
 */
class HealthDisplay : public QWidget
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructs the health display widget
     * @param parent Parent widget (optional)
     */
    explicit HealthDisplay(QWidget *parent = nullptr);

    /* ------------------------------------------------------------------------
     * Public Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Updates the health display with new values
     * @param health_score Score from 0-100
     * @param status Status text (e.g., "Healthy", "Warning")
     */
    void update_values(int health_score, const QString &status);
    
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
    QLabel *title;          ///< Section title label
    QLabel *score_label;    ///< "Health Score:" label
    QLabel *score_value;    ///< Numeric score display
    QProgressBar *score_bar;///< Visual progress indicator
    QLabel *status_label;   ///< "Status:" label
    QLabel *status_value;   ///< Status text display
};

#endif // HEALTH_DISPLAY_H
