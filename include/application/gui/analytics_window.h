#ifndef ANALYTICS_WINDOW_H
#define ANALYTICS_WINDOW_H

#include <QDialog>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QVector>

// Qt Charts Headers
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#include "leafsense_data_bridge.h"

// Namespace macro
QT_CHARTS_USE_NAMESPACE

// Structure for Gallery Data
struct GalleryItem {
    int image_id;
    QString filepath;
    QString timestamp;
    QString prediction_label;
    QString bounding_box;
    bool is_verified;
};

class AnalyticsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AnalyticsWindow(LeafSenseDataBridge *bridge, QWidget *parent = nullptr);
    ~AnalyticsWindow();

public slots:
    void refresh_data();  // Slot to update data from database

private slots:
    void on_close_clicked();
    void on_metric_changed(int index);

    // Gallery Slots (NEW)
    void on_gallery_prev();
    void on_gallery_next();
    void on_verify_clicked();

private:
    void setup_ui();
    void apply_theme();
    void load_sensor_data();  // Load real data from database
    void update_chart_series(int column_index);

    // Gallery Methods (NEW)
    void load_gallery_data();
    void update_gallery_display();

    LeafSenseDataBridge *data_bridge;  // Reference to data bridge
    QTabWidget *tabs;

    // Tab 1: Table
    QTableView *data_view;
    QStandardItemModel *mock_model;

    // Tab 2: Charts
    QChartView *chart_view;
    QChart *chart;
    QComboBox *metric_selector;

    // Tab 3: Gallery (NEW Components)
    QLabel *image_label;
    QLabel *info_label;
    QPushButton *btn_prev;
    QPushButton *btn_next;
    QPushButton *btn_verify;

    QVector<GalleryItem> gallery_items;
    int current_img_index;

    QPushButton *close_btn;
};

#endif // ANALYTICS_WINDOW_H
