/**
 * @file login_dialog.cpp
 * @brief Implementation of User Authentication Dialog
 */

#include "../include/application/gui/login_dialog.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QSpacerItem>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Login");
    setFixedSize(480, 320);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    setup_ui();
    apply_theme();
}

LoginDialog::~LoginDialog() {}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

void LoginDialog::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(20, 20, 20, 20);
    main_layout->setSpacing(5);

    /* ------------------------------------------------------------------------
     * Logo Section
     * ------------------------------------------------------------------------ */
    logo_label = new QLabel();
    logo_label->setFixedSize(200, 80);
    logo_label->setAlignment(Qt::AlignCenter);

    ThemeManager &tm = ThemeManager::instance();
    QString logo_filename = (tm.get_current_theme() == ThemeMode::LIGHT) 
        ? "logo_leafsense.png" 
        : "logo_leafsense_dark.png";
    QString logo_path = "/home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/LeafSenseUI/resources/images/" + logo_filename;
    QPixmap logo_pixmap(logo_path);

    if (!logo_pixmap.isNull()) {
        logo_label->setPixmap(logo_pixmap.scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo_label->setText("LeafSense");
    }

    main_layout->addWidget(logo_label, 0, Qt::AlignCenter);
    main_layout->addSpacerItem(new QSpacerItem(20, 15, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Label font (bold)
    QFont label_font;
    label_font.setPointSize(9);
    label_font.setBold(true);

    /* ------------------------------------------------------------------------
     * Username Input
     * ------------------------------------------------------------------------ */
    username_label = new QLabel("Username");
    username_label->setFont(label_font);

    username_input = new QLineEdit();
    connect(username_input, &QLineEdit::returnPressed, this, &LoginDialog::on_username_return_pressed);

    main_layout->addWidget(username_label);
    main_layout->addWidget(username_input);
    main_layout->addSpacing(5);

    /* ------------------------------------------------------------------------
     * Password Input
     * ------------------------------------------------------------------------ */
    password_label = new QLabel("Password");
    password_label->setFont(label_font);

    password_input = new QLineEdit();
    password_input->setEchoMode(QLineEdit::Password);
    connect(password_input, &QLineEdit::returnPressed, this, &LoginDialog::on_password_return_pressed);

    main_layout->addWidget(password_label);
    main_layout->addWidget(password_input);

    /* ------------------------------------------------------------------------
     * Error Message
     * ------------------------------------------------------------------------ */
    error_label = new QLabel("");
    error_label->setStyleSheet("color: red; font-size: 8pt;");
    error_label->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(error_label);

    main_layout->addStretch();

    /* ------------------------------------------------------------------------
     * Buttons
     * ------------------------------------------------------------------------ */
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setSpacing(15);
    button_layout->setAlignment(Qt::AlignCenter);

    login_btn = new QPushButton("Login");
    login_btn->setFixedSize(100, 32);
    connect(login_btn, &QPushButton::clicked, this, &LoginDialog::on_login_button_clicked);

    cancel_btn = new QPushButton("Exit");
    cancel_btn->setFixedSize(100, 32);
    connect(cancel_btn, &QPushButton::clicked, this, &LoginDialog::on_cancel_button_clicked);

    button_layout->addWidget(login_btn);
    button_layout->addWidget(cancel_btn);

    main_layout->addLayout(button_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

void LoginDialog::apply_theme() 
{
    // Handled by ThemeManager global stylesheet
}

/* ============================================================================
 * Public Accessors
 * ============================================================================ */

QString LoginDialog::get_username() const 
{ 
    return logged_in_user; 
}

QString LoginDialog::get_login_time() const 
{ 
    return login_timestamp; 
}

/* ============================================================================
 * Credential Validation
 * ============================================================================ */

bool LoginDialog::validate_credentials(const QString &u, const QString &p) 
{
    // TODO: Replace with database lookup
    if (u == "admin" && p == "admin") {
        return true;
    }
    error_label->setText("Invalid Credentials");
    return false;
}

/* ============================================================================
 * Event Handlers
 * ============================================================================ */

void LoginDialog::on_login_button_clicked() 
{
    if (validate_credentials(username_input->text(), password_input->text())) {
        logged_in_user = username_input->text();
        login_timestamp = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss UTC");
        accept();
    }
}

void LoginDialog::on_cancel_button_clicked() 
{ 
    reject(); 
}

void LoginDialog::on_username_return_pressed() 
{ 
    password_input->setFocus(); 
}

void LoginDialog::on_password_return_pressed() 
{ 
    on_login_button_clicked(); 
}
