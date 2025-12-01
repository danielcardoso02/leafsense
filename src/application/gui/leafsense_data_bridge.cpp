#include "leafsense_data_bridge.h"
#include "middleware/dbManager.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <clocale>

LeafSenseDataBridge::LeafSenseDataBridge(QObject *parent)
    : QObject(parent), update_timer(nullptr), dbReader(nullptr) {
    // Set C locale for numeric parsing (use '.' as decimal separator)
    std::setlocale(LC_NUMERIC, "C");
    
    // Use application directory path for database
    QString dbPath = QCoreApplication::applicationDirPath() + "/leafsense.db";
    qDebug() << "[DataBridge] Opening database at:" << dbPath;
    dbReader = new dbManager(dbPath.toStdString());
}

LeafSenseDataBridge::~LeafSenseDataBridge() {
    if(update_timer) { update_timer->stop(); delete update_timer; }
    delete dbReader;
}

bool LeafSenseDataBridge::initialize() {
    qDebug() << "[DataBridge] Initializing timer...";
    update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, this, &LeafSenseDataBridge::update_data);
    update_timer->start(2000); // Poll DB every 2 seconds
    qDebug() << "[DataBridge] Timer started, interval: 2000ms";
    
    // Trigger first update immediately
    update_data();
    return true;
}

SensorData LeafSenseDataBridge::get_sensor_data() {
    SensorData data{0,0,0,"--:--",false};
    // Read what the Master thread wrote
    DBResult res = dbReader->read("SELECT temperature, ph, ec, timestamp FROM vw_latest_sensor_reading;");
    
    qDebug() << "[DataBridge] Query returned" << res.rows.size() << "rows";
    
    if (!res.rows.empty()) {
        // Debug raw values
        qDebug() << "[DataBridge] Raw row[0]:" << QString::fromStdString(res.rows[0][0])
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
        } catch(...) {
            qDebug() << "[DataBridge] Parse error!";
        }
    }
    return data;
}

SystemAlert LeafSenseDataBridge::get_latest_alert() {
    SystemAlert alert{"System OK", "No active alerts", PlantHealthStatus::HEALTHY, ""};
    DBResult res = dbReader->read("SELECT type, message, timestamp FROM vw_unread_alerts LIMIT 1;");
    if (!res.rows.empty()) {
        alert.title = QString::fromStdString(res.rows[0][0]);
        alert.message = QString::fromStdString(res.rows[0][1]);
        alert.timestamp = QString::fromStdString(res.rows[0][2]);
        alert.severity = (alert.title == "Critical") ? PlantHealthStatus::CRITICAL : PlantHealthStatus::WARNING;
    }
    return alert;
}

// ... (Keep get_health_assessment / update_data as standard)
HealthAssessment LeafSenseDataBridge::get_health_assessment() {
    return {95, PlantHealthStatus::HEALTHY, "None"};
}

QVector<DailySensorSummary> LeafSenseDataBridge::get_sensor_history(int days) {
    QVector<DailySensorSummary> history;
    
    // First try daily summary
    DBResult res = dbReader->read("SELECT day, avg_temp, avg_ph, avg_ec FROM vw_daily_sensor_summary LIMIT 30;");
    
    qDebug() << "[DataBridge] Daily summary query returned" << res.rows.size() << "rows";
    
    // If only 1 day of data, fall back to individual readings (for testing/development)
    if (res.rows.size() <= 1) {
        qDebug() << "[DataBridge] Not enough daily data, using individual readings";
        QString query = QString("SELECT timestamp, temperature, ph, ec FROM sensor_readings ORDER BY timestamp DESC LIMIT %1;").arg(days);
        res = dbReader->read(query.toStdString());
        qDebug() << "[DataBridge] Individual readings query returned" << res.rows.size() << "rows";
        
        for (const auto& row : res.rows) {
            if (row.size() >= 4) {
                try {
                    DailySensorSummary summary;
                    // Extract just the date part or datetime
                    summary.date = QString::fromStdString(row[0]).left(16); // "YYYY-MM-DD HH:MM"
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
        // Use daily summaries
        for (const auto& row : res.rows) {
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

QString LeafSenseDataBridge::get_current_time() {
    return QDateTime::currentDateTimeUtc().toString("HH:mm:ss UTC");
}
void LeafSenseDataBridge::update_data() {
    emit sensor_data_updated(get_sensor_data());
    emit health_updated(get_health_assessment());
    emit alert_received(get_latest_alert());
    emit time_updated(get_current_time());
}