/**
 * @file analytics_window.h
 * @author Daniel Cardoso, Marco Costa
 * @brief Historical Data Analytics Window for LeafSense
 * @layer Application/GUI
 * 
 * Provides three tabs for data visualization:
 * - Tab 1: Sensor Readings Table (historical data from database)
 * - Tab 2: Trends Chart (line graphs for temperature, pH, EC)
 * - Tab 3: Image Gallery (ML predictions with bounding boxes)
 */

#ifndef ANALYTICS_WINDOW_H
#define ANALYTICS_WINDOW_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDialog>
#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QVector>

/* ============================================================================
 * Qt Charts Includes
 * ============================================================================ */
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "leafsense_data_bridge.h"

QT_CHARTS_USE_NAMESPACE

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @struct GalleryItem
 * @brief Represents an image with ML prediction data for the Gallery tab
 */
struct GalleryItem {
    int image_id;               ///< Database image ID
    QString filepath;           ///< Path to image file
    QString timestamp;          ///< When image was captured
    QString prediction_label;   ///< ML prediction (e.g., "Healthy", "Powdery Mildew")
    QString recommendation_text; ///< ML recommendation text for the prediction
    QString bounding_box;       ///< Bounding box coords "x,y,w,h" (empty if none)
    bool is_verified;           ///< Whether user has verified this prediction
    bool is_acknowledged;       ///< Whether user has acknowledged the recommendation
};

/* ============================================================================
 * AnalyticsWindow Class
 * ============================================================================ */

/**
 * @class AnalyticsWindow
 * @brief Dialog window for viewing historical sensor data and analytics
 * 
 * Features:
 * - Data table with daily/hourly sensor readings
 * - Interactive charts with min/max limit indicators
 * - Image gallery for ML predictions with verification workflow
 */
class AnalyticsWindow : public QDialog
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Construct analytics window
     * @param bridge Pointer to data bridge for database access
     * @param parent Parent widget (typically MainWindow)
     */
    explicit AnalyticsWindow(LeafSenseDataBridge *bridge, QWidget *parent = nullptr);
    ~AnalyticsWindow();

public slots:
    /* ------------------------------------------------------------------------
     * Public Slots
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Reload data from database and refresh all views
     */
    void refresh_data();

private slots:
    /* ------------------------------------------------------------------------
     * UI Event Handlers
     * ------------------------------------------------------------------------ */
    void on_close_clicked();
    void on_metric_changed(int index);

    /* ------------------------------------------------------------------------
     * Gallery Navigation Slots
     * ------------------------------------------------------------------------ */
    void on_gallery_prev();
    void on_gallery_next();
    void on_verify_clicked();
    void on_acknowledge_clicked();

private:
    /* ------------------------------------------------------------------------
     * Private Methods - Initialization
     * ------------------------------------------------------------------------ */
    void setup_ui();
    void apply_theme();

    /* ------------------------------------------------------------------------
     * Private Methods - Data Loading
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Load sensor history from database into table model
     */
    void load_sensor_data();
    
    /**
     * @brief Update chart series based on selected metric
     * @param column_index 1=Temperature, 2=pH, 3=EC
     */
    void update_chart_series(int column_index);

    /* ------------------------------------------------------------------------
     * Private Methods - Gallery
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Load gallery items (currently mock data)
     */
    void load_gallery_data();
    
    /**
     * @brief Update gallery display for current image
     */
    void update_gallery_display();

    /* ------------------------------------------------------------------------
     * Data Members
     * ------------------------------------------------------------------------ */
    LeafSenseDataBridge *data_bridge;   ///< Database access bridge

    /* ------------------------------------------------------------------------
     * UI Components - Main Layout
     * ------------------------------------------------------------------------ */
    QTabWidget *tabs;           ///< Tab container for all views
    QPushButton *close_btn;     ///< Close dialog button

    /* ------------------------------------------------------------------------
     * UI Components - Tab 1: Sensor Readings Table
     * ------------------------------------------------------------------------ */
    QTableView *data_view;              ///< Table view for sensor data
    QStandardItemModel *mock_model;     ///< Data model for table

    /* ------------------------------------------------------------------------
     * UI Components - Tab 2: Trends Chart
     * ------------------------------------------------------------------------ */
    QChartView *chart_view;     ///< Chart display widget
    QChart *chart;              ///< Chart object
    QComboBox *metric_selector; ///< Dropdown to select metric (Temp/pH/EC)

    /* ------------------------------------------------------------------------
     * UI Components - Tab 3: Gallery
     * ------------------------------------------------------------------------ */
    QLabel *image_label;        ///< Image display area
    QLabel *rec_label;          ///< Recommendation text display
    QLabel *info_label;         ///< Image info text
    QPushButton *btn_prev;      ///< Previous image button
    QPushButton *btn_next;      ///< Next image button
    QPushButton *btn_verify;    ///< Verify prediction button
    QPushButton *btn_acknowledge; ///< Acknowledge recommendation button

    /* ------------------------------------------------------------------------
     * Gallery State
     * ------------------------------------------------------------------------ */
    QVector<GalleryItem> gallery_items; ///< List of gallery images
    int current_img_index;              ///< Currently displayed image index
};

#endif // ANALYTICS_WINDOW_H
