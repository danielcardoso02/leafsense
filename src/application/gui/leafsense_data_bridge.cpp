/**
 * @file leafsense_data_bridge.cpp
 * @brief Implementation of the Database-to-GUI Bridge
 * @layer Application/GUI
 * 
 * This module provides real-time data synchronization between the SQLite
 * database (written by middleware threads) and the Qt GUI components.
 */

/* ============================================================================
 * Project Includes
 * ============================================================================ */
#include "leafsense_data_bridge.h"
#include "middleware/dbManager.h"

/* ============================================================================
 * Qt Framework Includes
 * ============================================================================ */
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

/* ============================================================================
 * Standard Library Includes
 * ============================================================================ */
#include <clocale>

/* ============================================================================
 * Constructor / Destructor
 * ============================================================================ */

LeafSenseDataBridge::LeafSenseDataBridge(QObject *parent)
    : QObject(parent)
    , update_timer(nullptr)
    , dbReader(nullptr)
{
    // IMPORTANT: Set C locale for numeric parsing
    // This ensures std::stod() uses '.' as decimal separator regardless of system locale.
    // Required for systems with Portuguese/European locale that use ',' as decimal separator.
    std::setlocale(LC_NUMERIC, "C");

    // Database path relative to application executable
    QString dbPath = QCoreApplication::applicationDirPath() + "/leafsense.db";
    qDebug() << "[DataBridge] Opening database at:" << dbPath;
    
    dbReader = new dbManager(dbPath.toStdString());
}

/**
 * @brief Destructor for LeafSenseDataBridge.
 * @author Daniel Cardoso, Marco Costa
 */
LeafSenseDataBridge::~LeafSenseDataBridge()
{
    if (update_timer) {
        update_timer->stop();
        delete update_timer;
    }
    delete dbReader;
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * @brief Initializes the data bridge and starts the update timer.
 * @return True if initialization succeeds.
 * @author Daniel Cardoso, Marco Costa
 */
bool LeafSenseDataBridge::initialize()
{
    qDebug() << "[DataBridge] Initializing timer...";
    
    update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, this, &LeafSenseDataBridge::update_data);
    
    // Poll database every 2 seconds
    update_timer->start(2000);
    qDebug() << "[DataBridge] Timer started, interval: 2000ms";

    // Trigger first update immediately
    update_data();
    
    return true;
}

/* ============================================================================
 * Real-Time Data Retrieval
 * ============================================================================ */

/**
 * @brief Retrieves the latest sensor data from the database.
 * @return SensorData struct with latest readings.
 * @author Daniel Cardoso, Marco Costa
 */
SensorData LeafSenseDataBridge::get_sensor_data()
{
    SensorData data{0, 0, 0, "--:--", false};

    // Query latest sensor reading from database view
    DBResult res = dbReader->read(
        "SELECT temperature, ph, ec, timestamp FROM vw_latest_sensor_reading;");

    qDebug() << "[DataBridge] Query returned" << res.rows.size() << "rows";

    if (!res.rows.empty()) {
        // Debug: Log raw values from database
        qDebug() << "[DataBridge] Raw row[0]:" 
                 << QString::fromStdString(res.rows[0][0])
                 << QString::fromStdString(res.rows[0][1])
                 << QString::fromStdString(res.rows[0][2])
                 << QString::fromStdString(res.rows[0][3]);

        try {
            data.temperature = std::stod(res.rows[0][0]);
            data.ph = std::stod(res.rows[0][1]);
            data.ec = std::stod(res.rows[0][2]);
            data.last_update_time = QString::fromStdString(res.rows[0][3]);
            data.is_valid = true;

            qDebug() << "[DataBridge] Parsed: Temp:" << QString::number(data.temperature, 'f', 2)
                     << "pH:" << QString::number(data.ph, 'f', 2)
                     << "EC:" << QString::number(data.ec, 'f', 1);
        } catch (...) {
            qDebug() << "[DataBridge] Parse error!";
        }
    }

    return data;
}

/**
 * @brief Retrieves the latest system alert from the database.
 * @return SystemAlert struct with alert details.
 * @author Daniel Cardoso, Marco Costa
 */
SystemAlert LeafSenseDataBridge::get_latest_alert()
{
    SystemAlert alert{"System OK", "No active alerts", PlantHealthStatus::HEALTHY, ""};

    // Query latest unread alert
    DBResult res = dbReader->read(
        "SELECT type, message, timestamp FROM vw_unread_alerts LIMIT 1;");

    if (!res.rows.empty()) {
        alert.title = QString::fromStdString(res.rows[0][0]);
        alert.message = QString::fromStdString(res.rows[0][1]);
        alert.timestamp = QString::fromStdString(res.rows[0][2]);
        
        // Determine severity from alert type
        alert.severity = (alert.title == "Critical") 
            ? PlantHealthStatus::CRITICAL 
            : PlantHealthStatus::WARNING;
    }

    return alert;
}

/**
 * @brief Retrieves the latest health assessment (mock/ML).
 * @return HealthAssessment struct.
 * @author Daniel Cardoso, Marco Costa
 */
HealthAssessment LeafSenseDataBridge::get_health_assessment()
{
    // TODO: Implement ML-based health assessment retrieval
    // Currently returns mock data
    return {95, PlantHealthStatus::HEALTHY, "None"};
}

/* ============================================================================
 * Historical Data Retrieval (for Analytics)
 * ============================================================================ */

/**
 * @brief Retrieves historical sensor data for analytics.
 * @param days Number of days to retrieve.
 * @return QVector of DailySensorSummary structs.
 * @author Daniel Cardoso, Marco Costa
 */
QVector<DailySensorSummary> LeafSenseDataBridge::get_sensor_history(int days)
{
    QVector<DailySensorSummary> history;

    // First, try to get daily aggregated summaries
    DBResult res = dbReader->read(
        "SELECT day, avg_temp, avg_ph, avg_ec FROM vw_daily_sensor_summary LIMIT 30;");

    qDebug() << "[DataBridge] Daily summary query returned" << res.rows.size() << "rows";

    // If less than 5 days of data exist, fall back to individual readings
    // This provides better granularity during development/testing
    if (res.rows.size() < 5) {
        qDebug() << "[DataBridge] Not enough daily data, using individual readings";

        QString query = QString(
            "SELECT timestamp, temperature, ph, ec "
            "FROM sensor_readings "
            "ORDER BY timestamp DESC "
            "LIMIT %1;").arg(days);
        
        res = dbReader->read(query.toStdString());
        qDebug() << "[DataBridge] Individual readings query returned" << res.rows.size() << "rows";

        for (const auto &row : res.rows) {
            if (row.size() >= 4) {
                try {
                    DailySensorSummary summary;
                    // Extract datetime (first 16 chars: "YYYY-MM-DD HH:MM")
                    summary.date = QString::fromStdString(row[0]).left(16);
                    summary.avg_temp = std::stod(row[1]);
                    summary.avg_ph = std::stod(row[2]);
                    summary.avg_ec = std::stod(row[3]);
                    history.append(summary);
                } catch (...) {
                    qDebug() << "[DataBridge] Error parsing reading row";
                }
            }
        }
    } else {
        // Use daily summaries from the view
        for (const auto &row : res.rows) {
            if (row.size() >= 4) {
                try {
                    DailySensorSummary summary;
                    summary.date = QString::fromStdString(row[0]);
                    summary.avg_temp = std::stod(row[1]);
                    summary.avg_ph = std::stod(row[2]);
                    summary.avg_ec = std::stod(row[3]);
                    history.append(summary);
                } catch (...) {
                    qDebug() << "[DataBridge] Error parsing history row";
                }
            }
        }
    }

    return history;
}

/* ============================================================================
 * Image Prediction Retrieval
 * ============================================================================ */

/**
 * @brief Gets the ML prediction label for an image file.
 * @param filename The image filename (not full path).
 * @return Prediction label string, or empty if not found.
 * @author Daniel Cardoso, Marco Costa
 */
QString LeafSenseDataBridge::get_image_prediction(const QString &filename)
{
    QString query = QString(
        "SELECT p.prediction_label, p.confidence "
        "FROM ml_predictions p "
        "JOIN plant_images i ON p.image_id = i.id "
        "WHERE i.filename = '%1' "
        "ORDER BY p.predicted_at DESC LIMIT 1;"
    ).arg(filename);
    
    DBResult res = dbReader->read(query.toStdString());
    
    if (!res.rows.empty() && res.rows[0].size() >= 2) {
        QString label = QString::fromStdString(res.rows[0][0]);
        QString conf = QString::fromStdString(res.rows[0][1]);
        // Format: "Healthy (95.2%)"
        double confidence = conf.toDouble() * 100;
        return QString("%1 (%2%)").arg(label).arg(confidence, 0, 'f', 1);
    }
    
    return QString();
}

/* ============================================================================
 * Utility Methods
 * ============================================================================ */

/**
 * @brief Gets the current UTC time as a string.
 * @return Current time string.
 * @author Daniel Cardoso, Marco Costa
 */
QString LeafSenseDataBridge::get_current_time()
{
    return QDateTime::currentDateTimeUtc().toString("HH:mm:ss UTC");
}

/* ============================================================================
 * Timer Callback
 * ============================================================================ */

/**
 * @brief Timer callback to update and emit latest data to GUI.
 * @author Daniel Cardoso, Marco Costa
 */
void LeafSenseDataBridge::update_data()
{
    // Emit signals with latest data (connected widgets will update)
    emit sensor_data_updated(get_sensor_data());
    emit health_updated(get_health_assessment());
    emit alert_received(get_latest_alert());
    emit time_updated(get_current_time());
}