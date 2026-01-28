/**
 * @file info_window.h
 * @author Daniel Cardoso, Marco Costa
 * @brief System Information Dialog
 * @layer Application/GUI
 * 
 * Modal dialog displaying application and user session information.
 */

#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDialog>
#include <QLabel>

/**
 * @class InfoWindow
 * @brief Dialog displaying system and user information
 * 
 * Shows:
 * - Application version and developers
 * - Current user session details
 * - Login timestamp and current time
 */
class InfoWindow : public QDialog
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Constructs the info dialog
     * @param username Currently logged in user
     * @param login_time Timestamp of login
     * @param parent Parent widget (optional)
     */
    explicit InfoWindow(const QString &username, const QString &login_time, QWidget *parent = nullptr);
    
    ~InfoWindow();

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    
    void setup_ui();     ///< Creates and configures all UI components
    void apply_theme();  ///< Applies the current theme colors

    /* ------------------------------------------------------------------------
     * Session Data
     * ------------------------------------------------------------------------ */
    QString username;    ///< Current user's name
    QString login_time;  ///< Session start timestamp
};

#endif // INFO_WINDOW_H
