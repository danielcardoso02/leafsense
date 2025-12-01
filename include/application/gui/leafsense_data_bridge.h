#ifndef LEAFSENSE_DATA_BRIDGE_H
#define LEAFSENSE_DATA_BRIDGE_H

#include <QString>
#include <QObject>
#include <QTimer>
#include <QVector>

class dbManager;  // Forward declaration

enum class PlantHealthStatus {
    HEALTHY,
    WARNING,
    CRITICAL,
    UNKNOWN
};

struct SensorData {
    double ph;
    double temperature;
    double ec;
    QString last_update_time;
    bool is_valid;
};

// Structure for historical sensor readings (daily summary)
struct DailySensorSummary {
    QString date;
    double avg_temp;
    double avg_ph;
    double avg_ec;
};

struct HealthAssessment {
    int overall_score;
    PlantHealthStatus status;
    QString disease_detection;
    QString nutrient_status;
    QString growth_stage;
    QString last_analysis_time;
};

struct SystemAlert {
    QString title;
    QString message;
    PlantHealthStatus severity;
    QString timestamp;
};

class LeafSenseDataBridge : public QObject
{
    Q_OBJECT

public:
    explicit LeafSenseDataBridge(QObject *parent = nullptr);
    ~LeafSenseDataBridge();

    SensorData get_sensor_data();
    HealthAssessment get_health_assessment();
    SystemAlert get_latest_alert();
    QString get_system_status();
    QString get_current_time();
    
    // Historical data for analytics
    QVector<DailySensorSummary> get_sensor_history(int days = 30);

    bool initialize();

signals:
    void sensor_data_updated(const SensorData &data);
    void health_updated(const HealthAssessment &health);
    void alert_received(const SystemAlert &alert);
    void time_updated(const QString &time);

private slots:
    void update_data();

private:
    QTimer *update_timer;
    dbManager *dbReader;
};

#endif // LEAFSENSE_DATA_BRIDGE_H
