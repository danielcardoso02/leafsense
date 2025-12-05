/**
 * @file login_dialog.h
 * @author Daniel Cardoso, Marco Costa
 * @brief User Authentication Dialog
 * @layer Application/GUI
 * 
 * Modal dialog for user login with credential validation.
 * Supports keyboard navigation (Enter to submit, Tab between fields).
 */

#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QEvent>

/**
 * @class LoginDialog
 * @brief Authentication dialog for user login
 * 
 * Features:
 * - Username and password input fields
 * - Credential validation against database
 * - Session timestamp recording
 * - Keyboard-friendly navigation
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    /* ------------------------------------------------------------------------
     * Public Accessors
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Gets the authenticated username
     * @return Username string (empty if not logged in)
     */
    QString get_username() const;
    
    /**
     * @brief Gets the login timestamp
     * @return UTC timestamp string of successful login
     */
    QString get_login_time() const;

private slots:
    /* ------------------------------------------------------------------------
     * Event Handlers
     * ------------------------------------------------------------------------ */
    
    void on_login_button_clicked();      ///< Validates and submits credentials
    void on_cancel_button_clicked();     ///< Closes dialog with rejection
    void on_username_return_pressed();   ///< Moves focus to password field
    void on_password_return_pressed();   ///< Triggers login attempt

private:
    /* ------------------------------------------------------------------------
     * Private Methods
     * ------------------------------------------------------------------------ */
    
    void setup_ui();                     ///< Creates UI components
    void apply_theme();                  ///< Applies current theme
    bool eventFilter(QObject *obj, QEvent *event) override; ///< Handle focus events
    
    /**
     * @brief Validates user credentials
     * @param username User's login name
     * @param password User's password
     * @return true if credentials are valid
     */
    bool validate_credentials(const QString &username, const QString &password);

    /* ------------------------------------------------------------------------
     * UI Components
     * ------------------------------------------------------------------------ */
    QLabel *logo_label;         ///< Application logo
    QLabel *username_label;     ///< Username field label
    QLineEdit *username_input;  ///< Username text input
    QLabel *password_label;     ///< Password field label
    QLineEdit *password_input;  ///< Password text input (masked)
    QPushButton *login_btn;     ///< Submit button
    QPushButton *cancel_btn;    ///< Cancel/Exit button
    QLabel *error_label;        ///< Error message display

    /* ------------------------------------------------------------------------
     * Session State
     * ------------------------------------------------------------------------ */
    QString logged_in_user;     ///< Successfully authenticated username
    QString login_timestamp;    ///< UTC timestamp of login
};

#endif // LOGIN_DIALOG_H
