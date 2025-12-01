/**
 * @file theme_manager.cpp
 * @brief Implementation of Theme Manager Singleton
 */

#include "../include/application/gui/theme/theme_manager.h"
#include <QDebug>
#include <QApplication>

/* ============================================================================
 * Singleton Access
 * ============================================================================ */

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

/* ============================================================================
 * Theme Management
 * ============================================================================ */

void ThemeManager::set_theme(ThemeMode mode)
{
    current_theme = mode;

    switch (mode) {
        case ThemeMode::LIGHT:
            setup_light_theme();
            qDebug() << "Theme: LIGHT MODE";
            break;
        case ThemeMode::DARK:
            setup_dark_theme();
            qDebug() << "Theme: DARK MODE";
            break;
    }

    qApp->setStyleSheet(get_stylesheet());
}

ThemeMode ThemeManager::get_current_theme() const
{
    return current_theme;
}

/* ============================================================================
 * Color Access
 * ============================================================================ */

const ThemeColors& ThemeManager::get_colors() const
{
    return colors;
}

QColor ThemeManager::get_button_pressed_color() const
{
    return colors.primary_green.darker(150);
}

/* ============================================================================
 * Sensor Parameters
 * ============================================================================ */

void ThemeManager::set_sensor_parameters(const SensorParameters &params)
{
    sensor_params = params;
}

SensorParameters ThemeManager::get_sensor_parameters() const
{
    return sensor_params;
}

/* ============================================================================
 * Notification Settings
 * ============================================================================ */

void ThemeManager::set_notifications_enabled(bool enabled)
{
    notifications_enabled = enabled;
}

bool ThemeManager::get_notifications_enabled() const
{
    return notifications_enabled;
}

/* ============================================================================
 * Light Theme Setup
 * ============================================================================ */

void ThemeManager::setup_light_theme()
{
    // Brand colors
    colors.primary_green = QColor(76, 175, 80);     // #4CAF50
    colors.secondary_green = QColor(102, 205, 170);
    colors.accent_orange = QColor(46, 139, 87);
    colors.alert_red = QColor(220, 20, 60);

    // Background colors
    colors.bg_primary = QColor(248, 248, 248);
    colors.bg_secondary = QColor(255, 255, 255);
    colors.bg_tertiary = QColor(230, 230, 230);

    // Text colors
    colors.text_primary = QColor(33, 33, 33);
    colors.text_secondary = QColor(89, 89, 89);
    colors.text_muted = QColor(140, 140, 140);

    // Border colors
    colors.border_light = QColor(200, 200, 200);
    colors.border_dark = QColor(170, 170, 170);

    // Status colors
    colors.status_healthy = QColor(76, 175, 80);
    colors.status_warning = QColor(255, 165, 0);
    colors.status_critical = QColor(220, 20, 60);
}

/* ============================================================================
 * Dark Theme Setup
 * ============================================================================ */

void ThemeManager::setup_dark_theme()
{
    // Brand colors
    colors.primary_green = QColor(76, 175, 80);
    colors.secondary_green = QColor(102, 205, 170);
    colors.accent_orange = QColor(102, 205, 170);
    colors.alert_red = QColor(255, 107, 107);

    // Background colors
    colors.bg_primary = QColor(30, 30, 40);
    colors.bg_secondary = QColor(45, 45, 55);
    colors.bg_tertiary = QColor(60, 60, 75);

    // Text colors
    colors.text_primary = QColor(240, 240, 240);
    colors.text_secondary = QColor(180, 180, 190);
    colors.text_muted = QColor(130, 130, 140);

    // Border colors
    colors.border_light = QColor(70, 70, 90);
    colors.border_dark = QColor(50, 50, 70);

    // Status colors
    colors.status_healthy = QColor(76, 175, 80);
    colors.status_warning = QColor(255, 165, 0);
    colors.status_critical = QColor(255, 107, 107);
}

QString ThemeManager::get_stylesheet() const
{
    const ThemeColors& c = colors;

    // KEY FIX: Hardcoded #4CAF50 for buttons to ensure absolute uniformity across all windows/modes.

    return QString(R"(
        QMainWindow, QDialog {
            background-color: %1;
        }
        QWidget {
            background-color: %1;
            color: %2;
            font-family: "Arial";
            font-size: 9pt;
        }
        QLabel {
            color: %2;
            background-color: transparent;
        }
        QLineEdit {
            background-color: %3;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 8px;
            selection-background-color: %5;
            min-height: 24px;
        }
        QLineEdit:focus {
            border: 1px solid %5;
            background-color: %3;
        }
        QTableView {
            background-color: %1;
            gridline-color: %9;
            border: 1px solid %9;
            alternate-background-color: %3;
            selection-background-color: %5;
            selection-color: white;
        }
        QHeaderView::section {
            background-color: %4;
            color: %2;
            padding: 4px;
            border: none;
            border-right: 1px solid %9;
            font-weight: bold;
        }
        QTableCornerButton::section {
            background-color: %4;
            border: none;
        }
        QTabWidget::pane { border: 1px solid %9; }
        QTabBar::tab {
            background: %4; color: %2;
            min-width: 140px; padding: 8px 0px; margin-right: 2px;
            border-top-left-radius: 4px; border-top-right-radius: 4px;
            text-align: center;
        }
        QTabBar::tab:selected {
            background: %3; font-weight: bold; border-bottom: 2px solid #4CAF50;
        }
        QProgressBar {
            border: 1px solid %9;
            border-radius: 4px;
            background-color: %3;
            height: 8px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #4CAF50; /* Bar Green */
            border-radius: 3px;
        }

        /* UNIFORM BUTTON STYLING */
        /* Forced #4CAF50 (Bar Green) for all modes */
        QPushButton, QPushButton:focus, QPushButton:default {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 4px;
            font-weight: bold;
            min-height: 32px;
            outline: none; /* Kills Red Focus Ring */
        }
        QPushButton:hover {
            background-color: #66BB6A; /* Slightly Lighter Green */
            border: none;
        }
        QPushButton:pressed {
            background-color: #388E3C; /* Darker Green */
            border: none;
            outline: none;
        }
        QPushButton:checked {
             background-color: #388E3C; /* Darker Green */
             border: none;
             outline: none;
        }

        QGroupBox {
            border: 1px solid %9;
            border-radius: 4px;
            margin-top: 1.2em;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
            color: #4CAF50;
            font-weight: bold;
        }
        QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: %3;
            color: %2;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px;
            min-height: 24px;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
    )")
        .arg(c.bg_primary.name())           // %1
        .arg(c.text_primary.name())         // %2
        .arg(c.bg_secondary.name())         // %3
        .arg(c.bg_tertiary.name())          // %4
        .arg(c.primary_green.name())        // %5
        .arg(c.secondary_green.name())      // %6
        .arg(c.accent_orange.name())        // %7
        .arg(c.alert_red.name())            // %8
        .arg(c.border_light.name())         // %9
        .arg(c.primary_green.darker(150).name()); // %10
}

QString ThemeManager::get_button_stylesheet(bool is_active) const
{
    if (is_active) return QString("QPushButton { background-color: #388E3C; color: white; border: none; }");
    else return QString("QPushButton { background-color: #4CAF50; color: white; border: none; }");
}

QString ThemeManager::get_logo_path() const
{
    return ":/images/logo_leafsense.png";
}
