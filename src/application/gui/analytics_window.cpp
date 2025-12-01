#include "../include/application/gui/analytics_window.h"
#include "../include/application/gui/theme/theme_manager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPainter> // Added for bounding box
#include <QPixmap>  // Added for image loading

AnalyticsWindow::AnalyticsWindow(LeafSenseDataBridge *bridge, QWidget *parent) 
    : QDialog(parent), data_bridge(bridge)
{
    setWindowTitle("History & Analytics");
    setFixedSize(480, 320);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    mock_model = new QStandardItemModel(this);
    current_img_index = 0; // Initialize gallery index

    setup_ui();
    apply_theme();

    load_sensor_data();   // Load real data from database
    load_gallery_data();  // Initialize Gallery Data

    on_metric_changed(0);
    update_gallery_display(); // Initial Gallery Render
}

AnalyticsWindow::~AnalyticsWindow() {}

void AnalyticsWindow::setup_ui() {
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(8);

    tabs = new QTabWidget();

    // --- TAB 1: TABLE (Unchanged) ---
    QWidget *tab_table = new QWidget();
    QVBoxLayout *table_layout = new QVBoxLayout(tab_table);
    table_layout->setContentsMargins(0,5,0,0);

    data_view = new QTableView();
    data_view->setModel(mock_model);
    data_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data_view->verticalHeader()->setVisible(false);
    data_view->setSelectionMode(QAbstractItemView::SingleSelection);
    data_view->setAlternatingRowColors(true);

    table_layout->addWidget(data_view);
    tabs->addTab(tab_table, "Sensor Readings");

    // --- TAB 2: CHART (Unchanged logic) ---
    QWidget *tab_chart = new QWidget();
    QVBoxLayout *chart_layout = new QVBoxLayout(tab_chart);
    chart_layout->setContentsMargins(0,5,0,0);

    metric_selector = new QComboBox();
    metric_selector->addItem("Temperature (avg)", 1);
    metric_selector->addItem("pH (avg)", 2);
    metric_selector->addItem("EC (avg)", 3);
    connect(metric_selector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsWindow::on_metric_changed);

    chart_layout->addWidget(metric_selector);

    chart = new QChart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0,0,0,0));
    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_layout->addWidget(chart_view);

    tabs->addTab(tab_chart, "Trends");

    // --- TAB 3: GALLERY (NEW) ---
    QWidget *tab_gallery = new QWidget();
    QVBoxLayout *gallery_layout = new QVBoxLayout(tab_gallery);
    gallery_layout->setContentsMargins(5, 5, 5, 5);

    // Image Container
    image_label = new QLabel("No Images");
    image_label->setAlignment(Qt::AlignCenter);
    // Dark background for contrast
    image_label->setStyleSheet("background-color: #222; border-radius: 4px; color: #fff;");
    image_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gallery_layout->addWidget(image_label, 1);

    // Info Text
    info_label = new QLabel("-");
    info_label->setAlignment(Qt::AlignCenter);
    info_label->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 2px;");
    gallery_layout->addWidget(info_label);

    // Controls
    QHBoxLayout *controls = new QHBoxLayout();

    btn_prev = new QPushButton("<");
    btn_prev->setFixedSize(40, 30);
    connect(btn_prev, &QPushButton::clicked, this, &AnalyticsWindow::on_gallery_prev);

    btn_verify = new QPushButton("Confirm Issue");
    btn_verify->setFixedHeight(30);
    connect(btn_verify, &QPushButton::clicked, this, &AnalyticsWindow::on_verify_clicked);

    btn_next = new QPushButton(">");
    btn_next->setFixedSize(40, 30);
    connect(btn_next, &QPushButton::clicked, this, &AnalyticsWindow::on_gallery_next);

    controls->addWidget(btn_prev);
    controls->addWidget(btn_verify);
    controls->addWidget(btn_next);

    gallery_layout->addLayout(controls);
    tabs->addTab(tab_gallery, "Gallery");

    // Fix Image Scaling: Reload when tab is clicked
    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 2) {
            QApplication::processEvents();
            update_gallery_display();
        }
    });

    main_layout->addWidget(tabs);

    // --- CLOSE BUTTON ---
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addStretch();

    close_btn = new QPushButton("Close");
    close_btn->setFixedSize(100, 32);
    connect(close_btn, &QPushButton::clicked, this, &AnalyticsWindow::on_close_clicked);

    button_layout->addWidget(close_btn);
    button_layout->addStretch();

    main_layout->addLayout(button_layout);
}

void AnalyticsWindow::apply_theme() {
    ThemeManager& tm = ThemeManager::instance();
    const ThemeColors& colors = tm.get_colors();

    chart->setBackgroundBrush(QBrush(colors.bg_primary));
    chart->setTitleBrush(QBrush(colors.text_primary));

    // Apply button styling to new gallery buttons
    QString btnStyle = tm.get_button_stylesheet(false);
    btn_prev->setStyleSheet(btnStyle);
    btn_next->setStyleSheet(btnStyle);

    // Verify button gets dynamic styling in update_gallery_display
}

void AnalyticsWindow::on_close_clicked() { accept(); }

void AnalyticsWindow::load_sensor_data() {
    mock_model->clear();
    mock_model->setHorizontalHeaderLabels({"Date", "Temp (C)", "pH", "EC"});

    if (!data_bridge) {
        qDebug() << "[Analytics] No data bridge available";
        return;
    }

    QVector<DailySensorSummary> history = data_bridge->get_sensor_history(30);
    qDebug() << "[Analytics] Loaded" << history.size() << "days of sensor history";

    for (const auto& day : history) {
        QList<QStandardItem*> row;
        row << new QStandardItem(day.date);
        row << new QStandardItem(QString::number(day.avg_temp, 'f', 2));
        row << new QStandardItem(QString::number(day.avg_ph, 'f', 2));
        row << new QStandardItem(QString::number(day.avg_ec, 'f', 1));
        mock_model->appendRow(row);
    }
    
    // If no data from DB, show a message
    if (history.isEmpty()) {
        qDebug() << "[Analytics] No historical data found in database";
    }
}

void AnalyticsWindow::refresh_data() {
    load_sensor_data();
    on_metric_changed(metric_selector->currentIndex());
    qDebug() << "[Analytics] Data refreshed";
}

void AnalyticsWindow::on_metric_changed(int index) {
    update_chart_series(metric_selector->itemData(index).toInt());
}

void AnalyticsWindow::update_chart_series(int column_index) {
    chart->removeAllSeries();

    // Remove existing axes using the non-deprecated method
    for (QAbstractAxis* axis : chart->axes()) {
        chart->removeAxis(axis);
    }

    ThemeManager& tm = ThemeManager::instance();
    SensorParameters params = tm.get_sensor_parameters();
    const ThemeColors& colors = tm.get_colors();

    double limit_min = 0.0;
    double limit_max = 0.0;
    bool show_limits = true;

    switch (column_index) {
        case 1: limit_min = params.temp_min; limit_max = params.temp_max; break;
        case 2: limit_min = params.ph_min; limit_max = params.ph_max; break;
        case 3: limit_min = params.ec_min; limit_max = params.ec_max; break;
        default: show_limits = false;
    }

    QLineSeries *dataSeries = new QLineSeries();
    QLineSeries *minLimitSeries = new QLineSeries();
    QLineSeries *maxLimitSeries = new QLineSeries();

    double y_min = 10000;
    double y_max = -10000;

    if (show_limits) { y_min = limit_min; y_max = limit_max; }
    qint64 x_min_time = 0; qint64 x_max_time = 0;

    int rowCount = mock_model->rowCount();
    qDebug() << "[Analytics] Chart rowCount:" << rowCount;
    
    if (rowCount > 0) {
        for (int i = 0; i < rowCount; ++i) {
            QString date_str = mock_model->item(i, 0)->text();
            // Try multiple date formats
            QDateTime date = QDateTime::fromString(date_str, "yyyy-MM-dd HH:mm");
            if (!date.isValid()) {
                date = QDateTime::fromString(date_str, "yyyy-MM-dd HH:mm:ss");
            }
            if (!date.isValid()) {
                date = QDateTime::fromString(date_str, "yyyy-MM-dd");
            }
            if (!date.isValid()) {
                qDebug() << "[Analytics] Failed to parse date:" << date_str;
                continue;
            }
            
            qint64 x_ms = date.toMSecsSinceEpoch();
            if (i == 0) x_max_time = x_ms;
            if (i == rowCount - 1) x_min_time = x_ms;
            double y_val = mock_model->item(i, column_index)->text().toDouble();
            if (y_val < y_min) y_min = y_val; if (y_val > y_max) y_max = y_val;
            dataSeries->append(x_ms, y_val);
            
            if (i < 3) qDebug() << "[Analytics] Point" << i << ":" << date_str << "->" << y_val;
        }
    }

    if (show_limits && rowCount > 0) {
        minLimitSeries->append(x_min_time, limit_min); minLimitSeries->append(x_max_time, limit_min);
        maxLimitSeries->append(x_min_time, limit_max); maxLimitSeries->append(x_max_time, limit_max);
    }

    chart->addSeries(dataSeries);
    if (show_limits) { chart->addSeries(minLimitSeries); chart->addSeries(maxLimitSeries); }

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(5);
    // Use HH:mm format if data spans less than 1 day, otherwise use MM/dd
    if (x_max_time - x_min_time < 86400000) { // Less than 24 hours in ms
        axisX->setFormat("HH:mm");
    } else {
        axisX->setFormat("MM/dd");
    }
    axisX->setLabelsColor(colors.text_secondary);
    chart->addAxis(axisX, Qt::AlignBottom); dataSeries->attachAxis(axisX);
    if (show_limits) { minLimitSeries->attachAxis(axisX); maxLimitSeries->attachAxis(axisX); }

    QValueAxis *axisY = new QValueAxis;
    double buffer = (y_max - y_min) * 0.15; if (buffer == 0) buffer = 1.0;
    axisY->setRange(y_min - buffer, y_max + buffer); axisY->setLabelsColor(colors.text_secondary);
    chart->addAxis(axisY, Qt::AlignLeft); dataSeries->attachAxis(axisY);
    if (show_limits) { minLimitSeries->attachAxis(axisY); maxLimitSeries->attachAxis(axisY); }

    QPen dataPen(colors.primary_green); dataPen.setWidth(3); dataSeries->setPen(dataPen);

    if (show_limits) {
        QPen limitPen(colors.alert_red); limitPen.setWidth(2); limitPen.setStyle(Qt::DashLine);
        minLimitSeries->setPen(limitPen); maxLimitSeries->setPen(limitPen);
    }
}

// --- GALLERY LOGIC (NEW) ---

void AnalyticsWindow::load_gallery_data() {
    gallery_items.clear();

    // Mock Image 1: Healthy
    GalleryItem item1;
    item1.image_id = 1;
    item1.filepath = ":/images/images/logo_leafsense.png";
    item1.timestamp = "2025-11-29 10:00";
    item1.prediction_label = "Healthy";
    item1.is_verified = true;
    gallery_items.append(item1);

    // Mock Image 2: Disease
    GalleryItem item2;
    item2.image_id = 2;
    item2.filepath = ":/images/images/logo_leafsense.png";
    item2.timestamp = "2025-11-30 14:00";
    item2.prediction_label = "Powdery Mildew";
    item2.bounding_box = "80,20,60,60";
    item2.is_verified = false;
    gallery_items.append(item2);
}

void AnalyticsWindow::update_gallery_display() {
    if (gallery_items.empty()) return;

    GalleryItem &item = gallery_items[current_img_index];

    QPixmap pixmap(item.filepath);
    if (!pixmap.isNull()) {
        QPixmap drawing = pixmap.copy();
        if (!item.bounding_box.isEmpty()) {
            QPainter painter(&drawing);
            QPen pen(Qt::red);
            pen.setWidth(5);
            painter.setPen(pen);

            QStringList coords = item.bounding_box.split(",");
            if (coords.size() == 4) {
                painter.drawRect(coords[0].toInt(), coords[1].toInt(),
                                 coords[2].toInt(), coords[3].toInt());
            }
        }

        QSize labelSize = image_label->size();
        if (labelSize.width() < 10) labelSize = QSize(300, 200);

        image_label->setPixmap(drawing.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        image_label->setText("Image not found");
    }

    QString status = item.is_verified ? "[VERIFIED] " : "[PENDING] ";
    info_label->setText(item.timestamp + " - " + status + item.prediction_label);

    if (item.is_verified) {
        btn_verify->setText("Verified");
        btn_verify->setEnabled(false);
        btn_verify->setStyleSheet("background-color: #cccccc; color: #666; border: none; border-radius: 4px;");
    } else {
        btn_verify->setText("Confirm Issue");
        btn_verify->setEnabled(true);
        ThemeManager& tm = ThemeManager::instance();
        btn_verify->setStyleSheet(QString("background-color: %1; color: white; border: none; border-radius: 4px; font-weight: bold;")
                                  .arg(tm.get_colors().accent_orange.name()));
    }
}

void AnalyticsWindow::on_gallery_prev() {
    if (current_img_index > 0) {
        current_img_index--;
        update_gallery_display();
    }
}

void AnalyticsWindow::on_gallery_next() {
    if (current_img_index < gallery_items.size() - 1) {
        current_img_index++;
        update_gallery_display();
    }
}

void AnalyticsWindow::on_verify_clicked() {
    gallery_items[current_img_index].is_verified = true;
    update_gallery_display();
}
