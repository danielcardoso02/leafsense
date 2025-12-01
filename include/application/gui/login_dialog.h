#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString get_username() const;
    QString get_login_time() const;

private slots:
    void on_login_button_clicked();
    void on_cancel_button_clicked();
    void on_username_return_pressed();
    void on_password_return_pressed();

private:
    void setup_ui();
    void apply_theme();
    bool validate_credentials(const QString &username, const QString &password);

    QLabel *logo_label;
    QLabel *username_label;
    QLineEdit *username_input;
    QLabel *password_label;
    QLineEdit *password_input;
    QPushButton *login_btn;
    QPushButton *cancel_btn;
    QLabel *error_label;

    QString logged_in_user;
    QString login_timestamp;
};

#endif // LOGIN_DIALOG_H
