/**
 * @file info_window.cpp
 * @brief Implementation of System Information Dialog
 */

#include "../include/application/gui/info_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>
#include <QDateTime>
#include <QApplication>
#include <QScreen>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

InfoWindow::InfoWindow(const QString &username, const QString &login_time, QWidget *parent)
    : QDialog(parent)
    , username(username)
    , login_time(login_time)
{
     /**
      * @brief Constructs the info window dialog.
      * @param username Username string
      * @param login_time Login time string
      * @param parent Parent widget (optional)
      */
    setWindowTitle("System Information");
    setFixedSize(480, 320);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    setup_ui();
    apply_theme();
}

/**
 * @brief Destructor for InfoWindow.
 * @author Daniel Cardoso, Marco Costa
 */
InfoWindow::~InfoWindow() {}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Sets up the user interface components and layout for info window.
 * @author Daniel Cardoso, Marco Costa
 */
void InfoWindow::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(20, 20, 20, 20);
    main_layout->setSpacing(15);

    // Fonts
    QFont title_font;
    title_font.setPointSize(11);
    title_font.setBold(true);
    
    QFont info_font;
    info_font.setPointSize(9);

    /* ------------------------------------------------------------------------
     * System Information Section
     * ------------------------------------------------------------------------ */
    QLabel *system_title = new QLabel("System Information");
    system_title->setFont(title_font);
    main_layout->addWidget(system_title);

    QVBoxLayout *system_info = new QVBoxLayout();
    system_info->setSpacing(6);
    system_info->setContentsMargins(10, 0, 10, 0);

    QLabel *app_label = new QLabel("Application: LeafSense v1.0.0");
    app_label->setFont(info_font);
    system_info->addWidget(app_label);

    QLabel *creators_label = new QLabel("Developers: Daniel Cardoso and Marco Costa");
    creators_label->setFont(info_font);
    system_info->addWidget(creators_label);
    
    main_layout->addLayout(system_info);
    main_layout->addSpacing(5);

    /* ------------------------------------------------------------------------
     * User Information Section
     * ------------------------------------------------------------------------ */
    QLabel *user_title = new QLabel("User Information");
    user_title->setFont(title_font);
    main_layout->addWidget(user_title);

    QVBoxLayout *user_info = new QVBoxLayout();
    user_info->setSpacing(6);
    user_info->setContentsMargins(10, 0, 10, 0);

    QLabel *username_label = new QLabel("Username: " + username);
    username_label->setFont(info_font);
    user_info->addWidget(username_label);

    QLabel *login_label = new QLabel("Login Time: " + login_time);
    login_label->setFont(info_font);
    user_info->addWidget(login_label);

    QLabel *current_time_label = new QLabel("Current Time: " + 
        QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss UTC"));
    current_time_label->setFont(info_font);
    user_info->addWidget(current_time_label);

    main_layout->addLayout(user_info);
    main_layout->addStretch();

    /* ------------------------------------------------------------------------
     * Close Button
     * ------------------------------------------------------------------------ */
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addStretch();

    QPushButton *close_btn = new QPushButton("Close");
    close_btn->setFixedSize(100, 32);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);

    button_layout->addWidget(close_btn);
    button_layout->addStretch();

    main_layout->addLayout(button_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies the current theme to the info window.
 * @author Daniel Cardoso, Marco Costa
 */
void InfoWindow::apply_theme() 
{
    // Handled by ThemeManager global stylesheet
}
