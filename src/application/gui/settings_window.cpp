#include "../include/application/gui/settings_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFont>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QScrollArea>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setFixedSize(480, 320);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    setup_ui();
    apply_theme();
    load_settings();
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(6);

    QFont title_font; title_font.setPointSize(9); title_font.setBold(true);
    QFont label_font; label_font.setPointSize(8);

    QScrollArea *scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    QWidget *scroll_content = new QWidget();
    QVBoxLayout *scroll_layout = new QVBoxLayout(scroll_content);
    scroll_layout->setSpacing(6);
    scroll_layout->setContentsMargins(0, 0, 0, 0);

    // --- Sensor Params Group ---
    QGroupBox *sensor_params_group = new QGroupBox("Sensor Parameters");
    sensor_params_group->setFont(title_font);
    QGridLayout *params_layout = new QGridLayout(sensor_params_group);
    params_layout->setSpacing(4);
    params_layout->setContentsMargins(6, 8, 6, 6);

    // Helper for rows
    auto addParamRow = [&](QString labelText, QDoubleSpinBox*& minBox, QDoubleSpinBox*& maxBox, int row, double min, double max, int dec) {
        QLabel *l = new QLabel(labelText); l->setFont(label_font);
        minBox = new QDoubleSpinBox(); minBox->setRange(min, max); minBox->setDecimals(dec); minBox->setFixedSize(60, 22);
        maxBox = new QDoubleSpinBox(); maxBox->setRange(min, max); maxBox->setDecimals(dec); maxBox->setFixedSize(60, 22);

        QHBoxLayout *hl = new QHBoxLayout();
        hl->addWidget(l); hl->addWidget(minBox); hl->addWidget(new QLabel("-")); hl->addWidget(maxBox); hl->addStretch();
        params_layout->addLayout(hl, row, 0);
    };

    addParamRow("pH:", ideal_ph_min, ideal_ph_max, 0, 0, 14, 1);
    addParamRow("T(C):", ideal_temp_min, ideal_temp_max, 1, -10, 50, 1);
    addParamRow("EC:", ideal_ec_min, ideal_ec_max, 2, 0, 3000, 0);

    scroll_layout->addWidget(sensor_params_group);

    // --- Display Group ---
    QGroupBox *display_group = new QGroupBox("Display");
    display_group->setFont(title_font);
    QVBoxLayout *display_layout = new QVBoxLayout(display_group);

    QHBoxLayout *theme_layout = new QHBoxLayout();
    QLabel *theme_label = new QLabel("Theme:"); theme_label->setFont(label_font);
    theme_selector = new QComboBox();
    theme_selector->addItem("Light"); theme_selector->addItem("Dark");
    theme_selector->setFixedSize(80, 24);
    theme_layout->addWidget(theme_label); theme_layout->addWidget(theme_selector); theme_layout->addStretch();
    display_layout->addLayout(theme_layout);

    notifications_enabled = new QCheckBox("Notifications");
    notifications_enabled->setFont(label_font);
    notifications_enabled->setChecked(true);
    display_layout->addWidget(notifications_enabled);

    scroll_layout->addWidget(display_group);
    scroll_layout->addStretch();

    scroll_content->setLayout(scroll_layout);
    scroll_area->setWidget(scroll_content);
    main_layout->addWidget(scroll_area, 1);

    // --- BUTTONS (Fixed Uniform Size) ---
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setContentsMargins(0, 0, 0, 0);
    button_layout->setSpacing(10);
    button_layout->setAlignment(Qt::AlignCenter); // Centered

    save_btn = new QPushButton("Save");
    save_btn->setFixedSize(100, 32); // Uniform Size
    connect(save_btn, &QPushButton::clicked, this, &SettingsWindow::on_save_button_clicked);

    cancel_btn = new QPushButton("Cancel");
    cancel_btn->setFixedSize(100, 32); // Uniform Size
    connect(cancel_btn, &QPushButton::clicked, this, &SettingsWindow::on_cancel_button_clicked);

    button_layout->addWidget(save_btn);
    button_layout->addWidget(cancel_btn);

    main_layout->addLayout(button_layout, 0);
    setLayout(main_layout);
}

void SettingsWindow::apply_theme() {
    // handled by ThemeManager global stylesheet
}

// ... (Rest of logic: load_settings, get/set_sensor_parameters, slots remain unchanged) ...
void SettingsWindow::load_settings() {
    ThemeManager& tm = ThemeManager::instance();
    SensorParameters p = tm.get_sensor_parameters();
    ideal_ph_min->setValue(p.ph_min); ideal_ph_max->setValue(p.ph_max);
    ideal_temp_min->setValue(p.temp_min); ideal_temp_max->setValue(p.temp_max);
    ideal_ec_min->setValue(p.ec_min); ideal_ec_max->setValue(p.ec_max);
    theme_selector->setCurrentIndex((tm.get_current_theme() == ThemeMode::LIGHT) ? 0 : 1);
    notifications_enabled->setChecked(tm.get_notifications_enabled());
}
SensorParameters SettingsWindow::get_sensor_parameters() const {
    SensorParameters p;
    p.ph_min = ideal_ph_min->value(); p.ph_max = ideal_ph_max->value();
    p.temp_min = ideal_temp_min->value(); p.temp_max = ideal_temp_max->value();
    p.ec_min = ideal_ec_min->value(); p.ec_max = ideal_ec_max->value();
    return p;
}
void SettingsWindow::set_sensor_parameters(const SensorParameters &p) {
    ideal_ph_min->setValue(p.ph_min); ideal_ph_max->setValue(p.ph_max);
    ideal_temp_min->setValue(p.temp_min); ideal_temp_max->setValue(p.temp_max);
    ideal_ec_min->setValue(p.ec_min); ideal_ec_max->setValue(p.ec_max);
}
void SettingsWindow::on_save_button_clicked() {
    ThemeManager& tm = ThemeManager::instance();
    tm.set_sensor_parameters(get_sensor_parameters());
    tm.set_notifications_enabled(notifications_enabled->isChecked());
    ThemeMode new_theme = (theme_selector->currentIndex() == 0) ? ThemeMode::LIGHT : ThemeMode::DARK;
    if (new_theme != tm.get_current_theme()) tm.set_theme(new_theme);
    accept();
}
void SettingsWindow::on_cancel_button_clicked() { reject(); }
