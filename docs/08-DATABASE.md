# LeafSense - Database

## Overview

LeafSense uses SQLite as an embedded database for persistent storage of sensor data, logs, alerts, and ML results.

## Location

| Environment | Path |
|-------------|------|
| Desktop | `./leafsense.db` |
| Raspberry Pi | `/opt/leafsense/leafsense.db` |
| Schema | `database/schema.sql` |

## Schema

### Main Tables

#### `sensor_readings`
Stores environment sensor readings.

```sql
CREATE TABLE sensor_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL NOT NULL,
    ph REAL NOT NULL,
    ec REAL NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| id | INTEGER | Auto-increment primary key |
| temperature | REAL | Temperature in °C |
| ph | REAL | pH Value (0-14) |
| ec | REAL | Electrical conductivity (µS/cm) |
| timestamp | DATETIME | Reading date/time |

#### `logs`
Logging system for events and actions.

```sql
CREATE TABLE logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    log_type VARCHAR(50) NOT NULL,
    message TEXT NOT NULL,
    details TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| id | INTEGER | Primary key |
| log_type | VARCHAR | Type: Maintenance, Alert, System, ML |
| message | TEXT | Main message |
| details | TEXT | Additional details |
| timestamp | DATETIME | Event date/time |

#### `alerts`
System alerts.

```sql
CREATE TABLE alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    alert_type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) NOT NULL CHECK(severity IN ('low','medium','high','critical')),
    message TEXT NOT NULL,
    is_read BOOLEAN DEFAULT 0,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| severity | VARCHAR | low, medium, high, critical |
| is_read | BOOLEAN | Whether read by user |

#### `health_assessments`
Plant health assessments.

```sql
CREATE TABLE health_assessments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    plant_id INTEGER,
    assessment_type VARCHAR(50),
    status VARCHAR(50) NOT NULL,
    confidence REAL,
    notes TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (plant_id) REFERENCES plant(id)
);
```

#### `ml_predictions`
ML inference results.

```sql
CREATE TABLE ml_predictions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    image_path TEXT,
    predicted_class VARCHAR(100) NOT NULL,
    confidence REAL NOT NULL,
    all_probabilities TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

#### `ml_detections`
Disease detections.

```sql
CREATE TABLE ml_detections (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    prediction_id INTEGER,
    disease_name VARCHAR(100),
    severity VARCHAR(20),
    affected_area REAL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (prediction_id) REFERENCES ml_predictions(id)
);
```

#### `ml_recommendations`
ML-based recommendations.

```sql
CREATE TABLE ml_recommendations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    detection_id INTEGER,
    recommendation TEXT NOT NULL,
    priority VARCHAR(20),
    is_completed BOOLEAN DEFAULT 0,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (detection_id) REFERENCES ml_detections(id)
);
```

#### `plant`
Registry of monitored plants.

```sql
CREATE TABLE plant (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    species VARCHAR(100),
    planting_date DATE,
    location VARCHAR(100),
    notes TEXT
);
```

#### `plant_images`
Captured plant images.

```sql
CREATE TABLE plant_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    plant_id INTEGER,
    image_path TEXT NOT NULL,
    capture_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (plant_id) REFERENCES plant(id)
);
```

#### `user`
System users.

```sql
CREATE TABLE user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(100),
    role VARCHAR(20) DEFAULT 'user',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### Views

#### `vw_latest_sensor_reading`
Latest sensor reading.

```sql
CREATE VIEW vw_latest_sensor_reading AS
SELECT * FROM sensor_readings
ORDER BY timestamp DESC
LIMIT 1;
```

#### `vw_daily_sensor_summary`
Daily sensor summary.

```sql
CREATE VIEW vw_daily_sensor_summary AS
SELECT 
    DATE(timestamp) as date,
    AVG(temperature) as avg_temperature,
    MIN(temperature) as min_temperature,
    MAX(temperature) as max_temperature,
    AVG(ph) as avg_ph,
    AVG(ec) as avg_ec,
    COUNT(*) as reading_count
FROM sensor_readings
GROUP BY DATE(timestamp)
ORDER BY date DESC;
```

#### `vw_unread_alerts`
Unread alerts.

```sql
CREATE VIEW vw_unread_alerts AS
SELECT * FROM alerts
WHERE is_read = 0
ORDER BY 
    CASE severity 
        WHEN 'critical' THEN 1 
        WHEN 'high' THEN 2 
        WHEN 'medium' THEN 3 
        ELSE 4 
    END,
    timestamp DESC;
```

#### `vw_pending_recommendations`
Pending recommendations.

```sql
CREATE VIEW vw_pending_recommendations AS
SELECT 
    r.id,
    r.recommendation,
    r.priority,
    d.disease_name,
    r.timestamp
FROM ml_recommendations r
JOIN ml_detections d ON r.detection_id = d.id
WHERE r.is_completed = 0
ORDER BY 
    CASE r.priority 
        WHEN 'critical' THEN 1 
        WHEN 'high' THEN 2 
        WHEN 'medium' THEN 3 
        ELSE 4 
    END;
```

## Indexs

```sql
-- Performance indexes
CREATE INDEX idx_sensor_timestamp ON sensor_readings(timestamp);
CREATE INDEX idx_logs_timestamp ON logs(timestamp);
CREATE INDEX idx_logs_type ON logs(log_type);
CREATE INDEX idx_alerts_severity ON alerts(severity);
CREATE INDEX idx_alerts_read ON alerts(is_read);
CREATE INDEX idx_ml_predictions_timestamp ON ml_predictions(timestamp);
```

## Initialization

### Create Database
```bash
sqlite3 leafsense.db < database/schema.sql
```

### Verify Tables
```bash
sqlite3 leafsense.db '.tables'
# alerts                      plant_images              
# health_assessments          sensor_readings           
# logs                        user                      
# ml_detections               vw_daily_sensor_summary   
# ml_predictions              vw_latest_sensor_reading  
# ml_recommendations          vw_pending_recommendations
# plant                       vw_unread_alerts
```

## Common Queries

### Insert Sensor Reading
```sql
INSERT INTO sensor_readings (temperature, ph, ec) 
VALUES (23.5, 6.2, 1250);
```

### Last 10 Readings
```sql
SELECT * FROM sensor_readings 
ORDER BY timestamp DESC 
LIMIT 10;
```

### Daily Average
```sql
SELECT 
    AVG(temperature) as avg_temp,
    AVG(ph) as avg_ph,
    AVG(ec) as avg_ec
FROM sensor_readings 
WHERE DATE(timestamp) = DATE('now');
```

### Unread Critical Alerts
```sql
SELECT * FROM vw_unread_alerts 
WHERE severity = 'critical';
```

### Latest ML Predictions
```sql
SELECT 
    predicted_class,
    confidence,
    timestamp
FROM ml_predictions 
ORDER BY timestamp DESC 
LIMIT 5;
```

## Integration C++ (Qt)

### Database Manager
```cpp
// include/middleware/dbManager.h
class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();
    
    bool initialize(const QString& dbPath);
    
    // Sensor readings
    bool insertSensorReading(double temp, double ph, double ec);
    QVector<SensorReading> getRecentReadings(int count);
    
    // Logs
    bool insertLog(const QString& type, const QString& message, 
                   const QString& details = "");
    
    // Alerts
    bool insertAlert(const QString& type, const QString& severity,
                     const QString& message);
    int getUnreadAlertCount();
    
    // ML
    bool insertPrediction(const QString& imagePath, 
                          const QString& predictedClass,
                          double confidence);

private:
    QSqlDatabase db;
    bool executeQuery(const QString& query);
};
```

### Usage Example
```cpp
auto& db = DatabaseManager::instance();
db.initialize("/opt/leafsense/leafsense.db");

// Insert reading
db.insertSensorReading(23.5, 6.2, 1250);

// Insert log
db.insertLog("Maintenance", "pH Down", "Dosed 500ms");

// Get recent readings
auto readings = db.getRecentReadings(10);
for (const auto& r : readings) {
    qDebug() << r.temperature << r.ph << r.ec;
}
```

## Backup and Maintenance

### Backup
```bash
# Copy file
cp /opt/leafsense/leafsense.db /opt/leafsense/backup/leafsense_$(date +%Y%m%d).db

# Or use SQLite .backup
sqlite3 /opt/leafsense/leafsense.db ".backup /opt/leafsense/backup/leafsense.db"
```

### Clean Old Data
```sql
-- Keep only last 30 days of readings
DELETE FROM sensor_readings 
WHERE timestamp < datetime('now', '-30 days');

-- Keep only last 7 days of logs
DELETE FROM logs 
WHERE timestamp < datetime('now', '-7 days');

-- Vacuum to recover space
VACUUM;
```

### Statistics
```sql
-- Count per table
SELECT 'sensor_readings' as table_name, COUNT(*) as count FROM sensor_readings
UNION ALL
SELECT 'logs', COUNT(*) FROM logs
UNION ALL
SELECT 'alerts', COUNT(*) FROM alerts
UNION ALL
SELECT 'ml_predictions', COUNT(*) FROM ml_predictions;
```
