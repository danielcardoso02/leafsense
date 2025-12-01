/**
 * @file leafsense_data_bridge.h
 * @brief Bridge between SQLite Database and Qt GUI components
 * @layer Application/GUI
 * 
 * This class serves as the data layer for the GUI, providing:
 * - Real-time sensor data polling from the database
 * - Historical data retrieval for analytics
 * - Alert and health assessment queries
 * - Signal-based updates to connected UI components
 * 
 * The bridge polls the database at regular intervals and emits Qt signals
 * when new data is available, allowing the UI to update reactively.
 */

#ifndef LEAFSENSE_DATA_BRIDGE_H
#define LEAFSENSE_DATA_BRIDGE_H

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QString>
#include <QObject>
#include <QTimer>
#include <QVector>

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */
class dbManager;

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @enum PlantHealthStatus
 * @brief Represents the overall health state of the plant
 */
enum class PlantHealthStatus {
    HEALTHY,    ///< All parameters within acceptable range
    WARNING,    ///< One or more parameters approaching limits
    CRITICAL,   ///< One or more parameters outside safe range
    UNKNOWN     ///< Unable to determine status
};

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @struct SensorData
 * @brief Current sensor readings from the hydroponic system
 */
struct SensorData {
    double ph;                  ///< pH level (0-14 scale)
    double temperature;         ///< Water temperature in Celsius
    double ec;                  ///< Electrical conductivity (µS/cm or ppm)
    QString last_update_time;   ///< Timestamp of last reading
    bool is_valid;              ///< Whether data was successfully retrieved
};

/**
 * @struct DailySensorSummary
 * @brief Aggregated sensor data for a single day (used in Analytics)
 */
struct DailySensorSummary {
    QString date;       ///< Date string (YYYY-MM-DD or YYYY-MM-DD HH:MM)
    double avg_temp;    ///< Average temperature for the day
    double avg_ph;      ///< Average pH for the day
    double avg_ec;      ///< Average EC for the day
};

/**
 * @struct HealthAssessment
 * @brief Plant health evaluation from ML analysis
 */
struct HealthAssessment {
    int overall_score;              ///< Health score (0-100)
    PlantHealthStatus status;       ///< Categorical health status
    QString disease_detection;      ///< Detected disease (if any)
    QString nutrient_status;        ///< Nutrient deficiency status
    QString growth_stage;           ///< Current growth stage
    QString last_analysis_time;     ///< When analysis was performed
};

/**
 * @struct SystemAlert
 * @brief System notification or warning
 */
struct SystemAlert {
    QString title;              ///< Alert type (e.g., "Critical", "Warning")
    QString message;            ///< Detailed alert message
    PlantHealthStatus severity; ///< Alert severity level
    QString timestamp;          ///< When alert was generated
};

/* ============================================================================
 * LeafSenseDataBridge Class
 * ============================================================================ */

/**
 * @class LeafSenseDataBridge
 * @brief Provides reactive data binding between database and GUI
 * 
 * This class implements a polling mechanism that periodically queries
 * the SQLite database and emits Qt signals when data changes.
 * 
 * Usage:
 * @code
 *   LeafSenseDataBridge *bridge = new LeafSenseDataBridge(this);
 *   connect(bridge, &LeafSenseDataBridge::sensor_data_updated, 
 *           this, &MyWidget::on_sensor_update);
 *   bridge->initialize();
 * @endcode
 */
class LeafSenseDataBridge : public QObject
{
    Q_OBJECT

public:
    /* ------------------------------------------------------------------------
     * Constructor / Destructor
     * ------------------------------------------------------------------------ */
    explicit LeafSenseDataBridge(QObject *parent = nullptr);
    ~LeafSenseDataBridge();

    /* ------------------------------------------------------------------------
     * Initialization
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Start the data polling timer
     * @return true if initialization successful
     */
    bool initialize();

    /* ------------------------------------------------------------------------
     * Data Retrieval Methods
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Get current sensor readings from database
     * @return SensorData struct with latest values
     */
    SensorData get_sensor_data();
    
    /**
     * @brief Get current health assessment
     * @return HealthAssessment struct (currently returns mock data)
     */
    HealthAssessment get_health_assessment();
    
    /**
     * @brief Get most recent unread alert
     * @return SystemAlert struct with alert details
     */
    SystemAlert get_latest_alert();
    
    /**
     * @brief Get system status string
     * @return Status description
     */
    QString get_system_status();
    
    /**
     * @brief Get current UTC time formatted string
     * @return Time in "HH:mm:ss UTC" format
     */
    QString get_current_time();

    /* ------------------------------------------------------------------------
     * Historical Data (for Analytics)
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Retrieve historical sensor data
     * @param days Number of days/readings to retrieve (default: 30)
     * @return Vector of daily summaries, or individual readings if < 2 days exist
     * 
     * First attempts to use vw_daily_sensor_summary view.
     * Falls back to individual readings if insufficient daily data exists.
     */
    QVector<DailySensorSummary> get_sensor_history(int days = 30);

signals:
    /* ------------------------------------------------------------------------
     * Data Update Signals
     * ------------------------------------------------------------------------ */
    void sensor_data_updated(const SensorData &data);
    void health_updated(const HealthAssessment &health);
    void alert_received(const SystemAlert &alert);
    void time_updated(const QString &time);

private slots:
    /* ------------------------------------------------------------------------
     * Internal Update Handler
     * ------------------------------------------------------------------------ */
    
    /**
     * @brief Called by timer to refresh all data and emit signals
     */
    void update_data();

private:
    /* ------------------------------------------------------------------------
     * Private Members
     * ------------------------------------------------------------------------ */
    QTimer *update_timer;   ///< Polling timer (2 second interval)
    dbManager *dbReader;    ///< Database access object
};

#endif // LEAFSENSE_DATA_BRIDGE_H
