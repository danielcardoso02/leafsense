#include "sensors_display.h"
#include "theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>

SensorsDisplay::SensorsDisplay(QWidget *parent)
    : QWidget(parent)
{
    setup_ui();
    apply_theme();
}

void SensorsDisplay::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(2);

    title = new QLabel("Sensor Data");
    QFont title_font = title->font();
    title_font.setPointSize(10);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title);

    // REMOVED: main_layout->addSpacing(10); to fix the "Why is there a space?" issue.

    QFont value_font;
    value_font.setPointSize(9);
    value_font.setBold(true);

    auto createRow = [&](QLabel*& label, QLabel*& val, QString text, QString initVal) {
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

    main_layout->addLayout(createRow(ph_label, ph_value, "pH: ", "0.0"));
    main_layout->addLayout(createRow(temp_label, temp_value, "Temp: ", "0.0C"));
    main_layout->addLayout(createRow(ec_label, ec_value, "EC: ", "0 ppm"));

    main_layout->addStretch();
    setLayout(main_layout);
}

void SensorsDisplay::apply_theme()
{
    ThemeManager& tm = ThemeManager::instance();
    const ThemeColors& colors = tm.get_colors();

    title->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));

    ph_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));
    ph_value->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));

    temp_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));
    temp_value->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));

    ec_label->setStyleSheet(QString("color: %1;").arg(colors.text_primary.name()));
    ec_value->setStyleSheet(QString("color: %1;").arg(colors.primary_green.name()));
}

void SensorsDisplay::update_values(double ph, double temperature, double ec)
{
    qDebug() << "[SensorsDisplay] update_values called: pH=" << ph << "Temp=" << temperature << "EC=" << ec;
    ph_value->setText(QString::number(ph, 'f', 2));
    temp_value->setText(QString::number(temperature, 'f', 2) + "°C");
    ec_value->setText(QString::number(static_cast<int>(ec)) + " ppm");
}
