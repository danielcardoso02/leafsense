/**
 * @file alerts_display.cpp
 * @brief Implementation of Alert Status Display Widget
 */

#include "../include/application/gui/alerts_display.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

/* ============================================================================
 * Constructor
 * ============================================================================ */

AlertsDisplay::AlertsDisplay(QWidget *parent)
    : QWidget(parent)
{
     /**
      * @brief Constructs the alerts display widget.
      * @param parent Parent widget (optional)
      */
    setup_ui();
    apply_theme();
}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Creates and configures all UI components for alerts display.
 */
void AlertsDisplay::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(4);

    QFont label_font;
    label_font.setPointSize(8);

    // Centered content container
    QWidget *centered_widget = new QWidget();
    QVBoxLayout *centered_layout = new QVBoxLayout(centered_widget);
    centered_layout->setSpacing(2);
    centered_layout->setContentsMargins(0, 0, 0, 0);
    centered_layout->setAlignment(Qt::AlignCenter);

    // Status row: indicator dot + message
    QHBoxLayout *status_layout = new QHBoxLayout();
    status_layout->setSpacing(8);
    status_layout->setAlignment(Qt::AlignCenter);

    status_label = new QLabel("•");
    status_label->setFont(QFont("Arial", 12, QFont::Bold));

    alert_label = new QLabel("System OK");
    alert_label->setFont(label_font);

    status_layout->addWidget(status_label);
    status_layout->addWidget(alert_label);
    centered_layout->addLayout(status_layout);

    // Timestamp row
    time_label = new QLabel("Last check: --:--:-- UTC");
    time_label->setFont(label_font);
    time_label->setAlignment(Qt::AlignCenter);
    centered_layout->addWidget(time_label);

    centered_widget->setLayout(centered_layout);
    main_layout->addWidget(centered_widget, 1, Qt::AlignCenter);

    setLayout(main_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies the current theme colors to all UI elements.
 * @author Daniel Cardoso, Marco Costa
 */
void AlertsDisplay::apply_theme()
{
    ThemeManager &tm = ThemeManager::instance();
    const ThemeColors &colors = tm.get_colors();

    QFont label_font;
    label_font.setPointSize(8);

    alert_label->setFont(label_font);
    alert_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));

    time_label->setFont(label_font);
    time_label->setStyleSheet(QString("color: %1;").arg(colors.text_secondary.name()));

    status_label->setStyleSheet(QString("color: %1;").arg(colors.status_healthy.name()));
}

/* ============================================================================
 * Alert Update
 * ============================================================================ */

/**
 * @brief Updates the alert display with current system status.
 * @author Daniel Cardoso, Marco Costa
 */
void AlertsDisplay::update_alerts()
{
    alert_label->setText("System OK");
    status_label->setText("•");
}
