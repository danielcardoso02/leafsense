/**
 * @file health_display.cpp
 * @author Daniel Cardoso, Marco Costa
 * @brief Implementation of Plant Health Assessment Display Widget
 */

#include "../include/application/gui/health_display.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

/* ============================================================================
 * Constructor
 * ============================================================================ */

HealthDisplay::HealthDisplay(QWidget *parent)
    : QWidget(parent)
{
     /**
      * @brief Constructs the health display widget.
      * @param parent Parent widget (optional)
      */
    setup_ui();
    apply_theme();
}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Sets up the user interface components and layout for health display.
 */
void HealthDisplay::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(4);

    // Section title
    title = new QLabel("Plant Health Assessment");
    QFont title_font = title->font();
    title_font.setPointSize(9);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title);

    // Label font (bold)
    QFont label_font;
    label_font.setPointSize(8);
    label_font.setBold(true);

    // Centered content container
    QWidget *centered_widget = new QWidget();
    QVBoxLayout *centered_layout = new QVBoxLayout(centered_widget);
    centered_layout->setSpacing(4);
    centered_layout->setContentsMargins(0, 0, 0, 0);
    centered_layout->setAlignment(Qt::AlignCenter);

    /* ------------------------------------------------------------------------
     * Health Score Row
     * ------------------------------------------------------------------------ */
    QHBoxLayout *score_layout = new QHBoxLayout();
    score_layout->setSpacing(10);
    score_layout->setAlignment(Qt::AlignCenter);

    score_label = new QLabel("Health Score:");
    score_label->setFont(label_font);

    score_value = new QLabel("0%");

    score_bar = new QProgressBar();
    score_bar->setMaximum(100);
    score_bar->setTextVisible(false);
    score_bar->setFixedHeight(8);
    score_bar->setMinimumWidth(150);
    score_bar->setMaximumWidth(200);

    score_layout->addWidget(score_label);
    score_layout->addWidget(score_value);
    score_layout->addWidget(score_bar);
    centered_layout->addLayout(score_layout);

    /* ------------------------------------------------------------------------
     * Status Row
     * ------------------------------------------------------------------------ */
    QHBoxLayout *status_layout = new QHBoxLayout();
    status_layout->setSpacing(10);
    status_layout->setAlignment(Qt::AlignCenter);

    status_label = new QLabel("Status:");
    status_label->setFont(label_font);

    status_value = new QLabel("Unknown");

    status_layout->addWidget(status_label);
    status_layout->addWidget(status_value);
    centered_layout->addLayout(status_layout);

    centered_widget->setLayout(centered_layout);
    main_layout->addWidget(centered_widget, 1, Qt::AlignCenter);

    main_layout->addStretch();
    setLayout(main_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies the current theme colors to all UI elements.
 * @author Daniel Cardoso, Marco Costa
 */
void HealthDisplay::apply_theme()
{
    ThemeManager &tm = ThemeManager::instance();
    const ThemeColors &colors = tm.get_colors();

    // Title styling
    title->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));

    // Label styling
    score_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));
    score_value->setStyleSheet(QString("color: %1; font-weight: bold;").arg(colors.primary_green.name()));
    status_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));

    // Progress bar styling
    QString progress_style = QString(R"(
        QProgressBar {
            border: 1px solid %1;
            border-radius: 4px;
            background-color: %2;
        }
        QProgressBar::chunk {
            background-color: %3;
            border-radius: 3px;
        }
    )")
        .arg(colors.primary_green.name())
        .arg(colors.bg_secondary.name())
        .arg(colors.primary_green.name());

    score_bar->setStyleSheet(progress_style);

    // Update status color based on current score
    int score = score_value->text().remove('%').toInt();
    QColor color;
    if (score >= 80) {
        color = colors.status_healthy;
    } else if (score >= 60) {
        color = colors.status_warning;
    } else {
        color = colors.status_critical;
    }
    status_value->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color.name()));
}

/* ============================================================================
 * Value Update
 * ============================================================================ */

/**
 * @brief Updates the health display with new values.
 * @param health_score Health score percentage
 * @param status Status string (e.g., "Healthy")
 * @author Daniel Cardoso, Marco Costa
 */
void HealthDisplay::update_values(int health_score, const QString &status)
{
    score_value->setText(QString::number(health_score) + "%");
    score_bar->setValue(health_score);
    status_value->setText(status);

    // Update status color based on score threshold
    ThemeManager &tm = ThemeManager::instance();
    const ThemeColors &colors = tm.get_colors();

    QColor color;
    if (health_score >= 80) {
        color = colors.status_healthy;
    } else if (health_score >= 60) {
        color = colors.status_warning;
    } else {
        color = colors.status_critical;
    }

    status_value->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color.name()));
}
