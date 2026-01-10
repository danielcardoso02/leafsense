/**
 * @file analytics_window.cpp
 * @brief Implementation of the Analytics Window
 * @layer Application/GUI
 * 
 * Provides historical data visualization through three tabs:
 * - Sensor Readings Table
 * - Trends Chart
 * - Image Gallery
 */

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "../include/application/gui/analytics_window.h"
#include "../include/application/gui/theme/theme_manager.h"

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPainter>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

/**
 * @brief Constructs the AnalyticsWindow dialog.
 * @param bridge Pointer to the data bridge for database access.
 * @param parent Parent widget.
 * @author Daniel Cardoso, Marco Costa
 */
AnalyticsWindow::AnalyticsWindow(LeafSenseDataBridge *bridge, QWidget *parent)
    : QDialog(parent)
    , data_bridge(bridge)
    , current_img_index(0)
{
    // Window configuration
    setWindowTitle("History & Analytics");
    setFixedSize(480, 320);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    move(QApplication::primaryScreen()->availableGeometry().center() - rect().center());

    // Initialize data model
    mock_model = new QStandardItemModel(this);

    // Setup UI and load data
    setup_ui();
    apply_theme();

    load_sensor_data();     // Load from database
    load_gallery_data();    // Load gallery items (currently mock)

    on_metric_changed(0);   // Initialize chart
    update_gallery_display();
}

/**
 * @brief Destructor for AnalyticsWindow.
 * @author Daniel Cardoso, Marco Costa
 */
AnalyticsWindow::~AnalyticsWindow() {}

/* ============================================================================
 * UI Setup
 * ============================================================================ */

/**
 * @brief Sets up the UI components for the analytics window.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::setup_ui()
{
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(10, 10, 10, 10);
    main_layout->setSpacing(8);

    tabs = new QTabWidget();

    /* ------------------------------------------------------------------------
     * Tab 1: Sensor Readings Table
     * ------------------------------------------------------------------------ */
    QWidget *tab_table = new QWidget();
    QVBoxLayout *table_layout = new QVBoxLayout(tab_table);
    table_layout->setContentsMargins(0, 5, 0, 0);

    data_view = new QTableView();
    data_view->setModel(mock_model);
    data_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data_view->verticalHeader()->setVisible(false);
    data_view->setSelectionMode(QAbstractItemView::SingleSelection);
    data_view->setAlternatingRowColors(true);

    table_layout->addWidget(data_view);
    tabs->addTab(tab_table, "Sensor Readings");

    /* ------------------------------------------------------------------------
     * Tab 2: Trends Chart
     * ------------------------------------------------------------------------ */
    QWidget *tab_chart = new QWidget();
    QVBoxLayout *chart_layout = new QVBoxLayout(tab_chart);
    chart_layout->setContentsMargins(0, 5, 0, 0);

    // Metric selector dropdown
    metric_selector = new QComboBox();
    metric_selector->addItem("Temperature (avg)", 1);
    metric_selector->addItem("pH (avg)", 2);
    metric_selector->addItem("EC (avg)", 3);
    connect(metric_selector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnalyticsWindow::on_metric_changed);
    chart_layout->addWidget(metric_selector);

    // Chart widget
    chart = new QChart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));

    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    chart_layout->addWidget(chart_view);

    tabs->addTab(tab_chart, "Trends");

    /* ------------------------------------------------------------------------
     * Tab 3: Image Gallery
     * ------------------------------------------------------------------------ */
    QWidget *tab_gallery = new QWidget();
    QVBoxLayout *gallery_layout = new QVBoxLayout(tab_gallery);
    gallery_layout->setContentsMargins(5, 5, 5, 5);

    // Image display area
    image_label = new QLabel("No Images");
    image_label->setAlignment(Qt::AlignCenter);
    image_label->setStyleSheet("background-color: #222; border-radius: 4px; color: #fff;");
    image_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gallery_layout->addWidget(image_label, 1);

    // Image info label
    info_label = new QLabel("-");
    info_label->setAlignment(Qt::AlignCenter);
    info_label->setStyleSheet("font-weight: bold; font-size: 11px; margin-top: 2px;");
    gallery_layout->addWidget(info_label);

    // Navigation controls
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

    // Refresh gallery when tab is selected (fixes scaling issues and reloads images)
    connect(tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 2) {
            qDebug() << "[Gallery] Tab selected - reloading images...";
            load_gallery_data();  // Reload images from disk
            current_img_index = 0;  // Reset to first image
            QApplication::processEvents();
            update_gallery_display();
        }
    });

    main_layout->addWidget(tabs);

    /* ------------------------------------------------------------------------
     * Close Button
     * ------------------------------------------------------------------------ */
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addStretch();

    close_btn = new QPushButton("Close");
    close_btn->setFixedSize(100, 32);
    connect(close_btn, &QPushButton::clicked, this, &AnalyticsWindow::on_close_clicked);

    button_layout->addWidget(close_btn);
    button_layout->addStretch();

    main_layout->addLayout(button_layout);
}

/* ============================================================================
 * Theme Application
 * ============================================================================ */

/**
 * @brief Applies the current theme to the analytics window.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::apply_theme()
{
    ThemeManager &tm = ThemeManager::instance();
    const ThemeColors &colors = tm.get_colors();

    // Chart styling
    chart->setBackgroundBrush(QBrush(colors.bg_primary));
    chart->setTitleBrush(QBrush(colors.text_primary));

    // Gallery button styling
    QString btnStyle = tm.get_button_stylesheet(false);
    btn_prev->setStyleSheet(btnStyle);
    btn_next->setStyleSheet(btnStyle);

    // Note: btn_verify styling is handled dynamically in update_gallery_display()
}

/* ============================================================================
 * Event Handlers
 * ============================================================================ */

/**
 * @brief Handles Close button click.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::on_close_clicked()
{
    accept();
}

/* ============================================================================
 * Sensor Data Loading
 * ============================================================================ */

/**
 * @brief Loads sensor data from the database and populates the table model.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::load_sensor_data()
{
    mock_model->clear();
    mock_model->setHorizontalHeaderLabels({"Date", "Temp (C)", "pH", "EC"});

    if (!data_bridge) {
        qDebug() << "[Analytics] No data bridge available";
        return;
    }

    // Load historical data from database
    QVector<DailySensorSummary> history = data_bridge->get_sensor_history(30);
    qDebug() << "[Analytics] Loaded" << history.size() << "days of sensor history";

    // Populate table model
    for (const auto &day : history) {
        QList<QStandardItem *> row;
        row << new QStandardItem(day.date);
        row << new QStandardItem(QString::number(day.avg_temp, 'f', 2));
        row << new QStandardItem(QString::number(day.avg_ph, 'f', 2));
        row << new QStandardItem(QString::number(day.avg_ec, 'f', 1));
        mock_model->appendRow(row);
    }

    if (history.isEmpty()) {
        qDebug() << "[Analytics] No historical data found in database";
    }
}

/**
 * @brief Refreshes sensor data and updates the chart.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::refresh_data()
{
    load_sensor_data();
    on_metric_changed(metric_selector->currentIndex());
    qDebug() << "[Analytics] Data refreshed";
}

/* ============================================================================
 * Chart Rendering
 * ============================================================================ */

/**
 * @brief Handles metric selection change for chart display.
 * @param index Index of the selected metric.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::on_metric_changed(int index)
{
    update_chart_series(metric_selector->itemData(index).toInt());
}

/**
 * @brief Updates the chart series based on the selected metric column.
 * @param column_index Index of the metric column.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::update_chart_series(int column_index)
{
    // Clear existing chart data
    chart->removeAllSeries();
    for (QAbstractAxis *axis : chart->axes()) {
        chart->removeAxis(axis);
    }

    ThemeManager &tm = ThemeManager::instance();
    SensorParameters params = tm.get_sensor_parameters();
    const ThemeColors &colors = tm.get_colors();

    /* ------------------------------------------------------------------------
     * Determine limit lines based on selected metric
     * ------------------------------------------------------------------------ */
    double limit_min = 0.0;
    double limit_max = 0.0;
    bool show_limits = true;

    switch (column_index) {
        case 1:  // Temperature
            limit_min = params.temp_min;
            limit_max = params.temp_max;
            break;
        case 2:  // pH
            limit_min = params.ph_min;
            limit_max = params.ph_max;
            break;
        case 3:  // EC
            limit_min = params.ec_min;
            limit_max = params.ec_max;
            break;
        default:
            show_limits = false;
    }

    /* ------------------------------------------------------------------------
     * Create data series
     * ------------------------------------------------------------------------ */
    QLineSeries *dataSeries = new QLineSeries();
    QLineSeries *minLimitSeries = new QLineSeries();
    QLineSeries *maxLimitSeries = new QLineSeries();

    double y_min = 10000;
    double y_max = -10000;

    if (show_limits) {
        y_min = limit_min;
        y_max = limit_max;
    }

    qint64 x_min_time = 0;
    qint64 x_max_time = 0;

    /* ------------------------------------------------------------------------
     * Populate data series from table model
     * ------------------------------------------------------------------------ */
    int rowCount = mock_model->rowCount();
    qDebug() << "[Analytics] Chart rowCount:" << rowCount;

    if (rowCount > 0) {
        for (int i = 0; i < rowCount; ++i) {
            QString date_str = mock_model->item(i, 0)->text();

            // Try multiple date formats for flexibility
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
            
            // Track time range
            if (i == 0) x_max_time = x_ms;
            if (i == rowCount - 1) x_min_time = x_ms;

            // Get Y value from selected column
            double y_val = mock_model->item(i, column_index)->text().toDouble();
            
            // Track Y range
            if (y_val < y_min) y_min = y_val;
            if (y_val > y_max) y_max = y_val;

            dataSeries->append(x_ms, y_val);

            // Debug first few points
            if (i < 3) {
                qDebug() << "[Analytics] Point" << i << ":" << date_str << "->" << y_val;
            }
        }
    }

    /* ------------------------------------------------------------------------
     * Create limit lines if enabled
     * ------------------------------------------------------------------------ */
    if (show_limits && rowCount > 0) {
        minLimitSeries->append(x_min_time, limit_min);
        minLimitSeries->append(x_max_time, limit_min);
        maxLimitSeries->append(x_min_time, limit_max);
        maxLimitSeries->append(x_max_time, limit_max);
    }

    /* ------------------------------------------------------------------------
     * Add series to chart
     * ------------------------------------------------------------------------ */
    chart->addSeries(dataSeries);
    if (show_limits) {
        chart->addSeries(minLimitSeries);
        chart->addSeries(maxLimitSeries);
    }

    /* ------------------------------------------------------------------------
     * Configure X-axis (DateTime)
     * ------------------------------------------------------------------------ */
    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(5);
    
    // Use HH:mm format if data spans less than 24 hours
    if (x_max_time - x_min_time < 86400000) {
        axisX->setFormat("HH:mm");
    } else {
        axisX->setFormat("MM/dd");
    }
    
    axisX->setLabelsColor(colors.text_secondary);
    chart->addAxis(axisX, Qt::AlignBottom);
    
    dataSeries->attachAxis(axisX);
    if (show_limits) {
        minLimitSeries->attachAxis(axisX);
        maxLimitSeries->attachAxis(axisX);
    }

    /* ------------------------------------------------------------------------
     * Configure Y-axis (Value)
     * ------------------------------------------------------------------------ */
    QValueAxis *axisY = new QValueAxis;
    double buffer = (y_max - y_min) * 0.15;
    if (buffer == 0) buffer = 1.0;
    
    axisY->setRange(y_min - buffer, y_max + buffer);
    axisY->setLabelsColor(colors.text_secondary);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    dataSeries->attachAxis(axisY);
    if (show_limits) {
        minLimitSeries->attachAxis(axisY);
        maxLimitSeries->attachAxis(axisY);
    }

    /* ------------------------------------------------------------------------
     * Apply series styling
     * ------------------------------------------------------------------------ */
    // Data line: solid green
    QPen dataPen(colors.primary_green);
    dataPen.setWidth(3);
    dataSeries->setPen(dataPen);

    // Limit lines: dashed red
    if (show_limits) {
        QPen limitPen(colors.alert_red);
        limitPen.setWidth(2);
        limitPen.setStyle(Qt::DashLine);
        minLimitSeries->setPen(limitPen);
        maxLimitSeries->setPen(limitPen);
    }
}

/* ============================================================================
 * Gallery - Data Loading
 * ============================================================================ */

/**
 * @brief Loads gallery image data from the gallery directory.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::load_gallery_data()
{
    gallery_items.clear();

    // Gallery directory where camera saves images
    QString gallery_dir = "/opt/leafsense/gallery/";
    
    QDir dir(gallery_dir);
    if (!dir.exists()) {
        qDebug() << "[Gallery] Directory does not exist:" << gallery_dir;
        return;
    }
    
    // Get all JPEG images sorted by name (which includes timestamp)
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::Reversed);
    
    qDebug() << "[Gallery] Found" << files.size() << "images in" << gallery_dir;
    
    // Load each image as a gallery item
    int id = 1;
    for (const QFileInfo &fileInfo : files) {
        GalleryItem item;
        item.image_id = id++;
        item.filepath = fileInfo.absoluteFilePath();
        item.timestamp = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss");
        item.prediction_label = "Processing...";  // Will be updated by ML
        item.is_verified = false;
        
        gallery_items.append(item);
    }
    
    if (gallery_items.isEmpty()) {
        qDebug() << "[Gallery] No images found. Capture a photo from the main window.";
    }
}

/* ============================================================================
 * Gallery - Display Update
 * ============================================================================ */

/**
 * @brief Updates the gallery display with the current image and info.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::update_gallery_display()
{
    if (gallery_items.empty()) {
        qDebug() << "[Gallery] No images to display";
        image_label->setText("No Images Available\n\nImages will appear here after camera capture");
        info_label->setText("Waiting for images...");
        btn_prev->setEnabled(false);
        btn_next->setEnabled(false);
        btn_verify->setEnabled(false);
        return;
    }

    qDebug() << "[Gallery] Displaying image" << current_img_index + 1 << "of" << gallery_items.size();

    GalleryItem &item = gallery_items[current_img_index];

    // Load and display image
    QPixmap pixmap(item.filepath);
    if (!pixmap.isNull()) {
        qDebug() << "[Gallery] Loaded image:" << item.filepath;
        QPixmap drawing = pixmap.copy();

        // Draw bounding box if present
        if (!item.bounding_box.isEmpty()) {
            QPainter painter(&drawing);
            QPen pen(Qt::red);
            pen.setWidth(5);
            painter.setPen(pen);

            QStringList coords = item.bounding_box.split(",");
            if (coords.size() == 4) {
                painter.drawRect(
                    coords[0].toInt(),
                    coords[1].toInt(),
                    coords[2].toInt(),
                    coords[3].toInt()
                );
            }
        }

        // Scale to fit label
        QSize labelSize = image_label->size();
        if (labelSize.width() < 10) {
            labelSize = QSize(300, 200);  // Fallback size
        }
        qDebug() << "[Gallery] Scaling to" << labelSize;
        image_label->setPixmap(drawing.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qCritical() << "[Gallery] Failed to load image:" << item.filepath;
        image_label->setText("Failed to load image:\n" + item.filepath);
    }

    // Update info label
    QString status = item.is_verified ? "[VERIFIED] " : "[PENDING] ";
    QString info = QString("%1/%2 - %3%4%5")
        .arg(current_img_index + 1)
        .arg(gallery_items.size())
        .arg(item.timestamp)
        .arg(status)
        .arg(item.prediction_label);
    info_label->setText(info);
    
    qDebug() << "[Gallery] Info:" << info;

    // Enable/disable navigation buttons
    btn_prev->setEnabled(current_img_index > 0);
    btn_next->setEnabled(current_img_index < gallery_items.size() - 1);

    // Update verify button state
    if (item.is_verified) {
        btn_verify->setText("Verified");
        btn_verify->setEnabled(false);
        btn_verify->setStyleSheet(
            "background-color: #cccccc; color: #666; border: none; border-radius: 4px;");
    } else {
        btn_verify->setText("Confirm Issue");
        btn_verify->setEnabled(true);
        ThemeManager &tm = ThemeManager::instance();
        btn_verify->setStyleSheet(
            QString("background-color: %1; color: white; border: none; border-radius: 4px; font-weight: bold;")
                .arg(tm.get_colors().accent_orange.name()));
    }
}

/* ============================================================================
 * Gallery - Navigation
 * ============================================================================ */

/**
 * @brief Handles previous image navigation in gallery.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::on_gallery_prev()
{
    if (current_img_index > 0) {
        current_img_index--;
        update_gallery_display();
    }
}

/**
 * @brief Handles next image navigation in gallery.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::on_gallery_next()
{
    if (current_img_index < gallery_items.size() - 1) {
        current_img_index++;
        update_gallery_display();
    }
}

/**
 * @brief Handles verification of the current gallery item.
 * @author Daniel Cardoso, Marco Costa
 */
void AnalyticsWindow::on_verify_clicked()
{
    // Mark current item as verified
    gallery_items[current_img_index].is_verified = true;
    
    // TODO: Update database (ml_detections.is_verified = 1)
    
    update_gallery_display();
}
