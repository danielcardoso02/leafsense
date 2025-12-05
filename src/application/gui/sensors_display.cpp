/**
 * @file sensors_display.cpp
 * @author Daniel Cardoso, Marco Costa
 * @brief Implementation of the Sensor Data Display Widget
 * @layer Application/GUI
 */

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "../include/application/gui/sensors_display.h"
#include "../include/application/gui/theme/theme_manager.h"

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>

/* ============================================================================
 * Constructor
 * ============================================================================ */

SensorsDisplay::SensorsDisplay(QWidget *parent)
    : QWidget(parent)
{
     /**
      * @brief Constructs the sensors display widget.
      * @param parent Parent widget (optional)
      */
    setup_ui();
    apply_theme();
}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Sets up the user interface components and layout for sensors display.
 */
void SensorsDisplay::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(2);

    // Section title
    title = new QLabel("Sensor Data");
    QFont title_font = title->font();
    title_font.setPointSize(10);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title);

    // Value display font
    QFont value_font;
    value_font.setPointSize(9);
    value_font.setBold(true);

    /**
     * @brief Helper lambda to create a label-value row
     * @param label Reference to store the label widget
     * @param val Reference to store the value widget
     * @param text Label text (e.g., "pH: ")
     * @param initVal Initial value text
     * @return Configured QHBoxLayout
     */
    auto createRow = [&](QLabel *&label, QLabel *&val, QString text, QString initVal) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(5);
        
        label = new QLabel(text);
        label->setFont(value_font);
        
        val = new QLabel(initVal);
        val->setFont(value_font);
        
        row->addStretch();
        row->addWidget(label);
        row->addWidget(val);
        row->addStretch();
        
        return row;
    };

    // Create sensor value rows
    main_layout->addLayout(createRow(ph_label, ph_value, "pH: ", "0.0"));
    main_layout->addLayout(createRow(temp_label, temp_value, "Temp: ", "0.0C"));
    main_layout->addLayout(createRow(ec_label, ec_value, "EC: ", "0 ppm"));

    main_layout->addStretch();
    setLayout(main_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies current theme colors to widget.
 * @author Daniel Cardoso, Marco Costa
 */
void SensorsDisplay::apply_theme()
{
    ThemeManager &tm = ThemeManager::instance();
    const ThemeColors &colors = tm.get_colors();

    // Title styling
    title->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));

    // Label styling (static text)
    QString labelStyle = QString("color: %1;").arg(colors.text_primary.name());
    ph_label->setStyleSheet(labelStyle);
    temp_label->setStyleSheet(labelStyle);
    ec_label->setStyleSheet(labelStyle);

    // Value styling (dynamic values - highlighted)
    QString valueStyle = QString("color: %1;").arg(colors.primary_green.name());
    ph_value->setStyleSheet(valueStyle);
    temp_value->setStyleSheet(valueStyle);
    ec_value->setStyleSheet(valueStyle);
}

/* ============================================================================
 * Value Update
 * ============================================================================ */

/**
 * @brief Update displayed sensor values.
 * @param ph pH value
 * @param temperature Temperature value (°C)
 * @param ec Electrical conductivity (ppm)
 * @author Daniel Cardoso, Marco Costa
 */
void SensorsDisplay::update_values(double ph, double temperature, double ec)
{
    qDebug() << "[SensorsDisplay] update_values called: pH=" << ph 
             << "Temp=" << temperature << "EC=" << ec;

    // Update displayed values with appropriate precision
    ph_value->setText(QString::number(ph, 'f', 2));
    temp_value->setText(QString::number(temperature, 'f', 2) + "°C");
    ec_value->setText(QString::number(static_cast<int>(ec)) + " ppm");
}
