<h1 align="center">LeafSense - Database</h1>

<p align="center"><em>SQLite schema for persistent storage of sensor data, logs, alerts, and ML results</em></p>

<p align="center">
<strong>Schema Version:</strong> V1.0 &nbsp;|&nbsp; <strong>Last Updated:</strong> January 19, 2026
</p>

---

## Location

| Environment | Path |
|-------------|------|
| Desktop | `./leafsense.db` |
| Raspberry Pi | `/opt/leafsense/leafsense.db` |
| Schema | `database/schema.sql` |

## Schema

The database consists of **10 tables** and **4 views**.

### Main Tables

#### 1. `user`
System user (single-user constraint: id=1).

```sql
CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| id | INTEGER | Constrained to 1 (single-user system) |
| username | TEXT | Username (unique) |
| password_hash | TEXT | SHA-256 hashed password |
| created_at | TIMESTAMP | Account creation date |
| last_login | TIMESTAMP | Last login timestamp |

#### 2. `plant`
Plant registry (single-plant constraint: id=1).

```sql
CREATE TABLE IF NOT EXISTS plant (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    name TEXT NOT NULL,
    planted_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

#### 3. `sensor_readings`
Stores environment sensor readings.

```sql
CREATE TABLE IF NOT EXISTS sensor_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL,
    ph REAL,
    ec REAL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| id | INTEGER | Auto-increment primary key |
| temperature | REAL | Temperature in °C |
| ph | REAL | pH Value (0-14) |
| ec | REAL | Electrical conductivity (µS/cm) |
| timestamp | TIMESTAMP | Reading date/time |

#### 4. `alerts`
System alerts.

```sql
CREATE TABLE IF NOT EXISTS alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL,  -- 'Warning', 'Critical', 'Info'
    message TEXT NOT NULL,
    details TEXT,
    is_read INTEGER DEFAULT 0,  -- 0 for Unread, 1 for Read
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| type | TEXT | Warning, Critical, or Info |
| message | TEXT | Alert message |
| is_read | INTEGER | 0 = Unread, 1 = Read |

#### 5. `logs`
Logging system for events and actions.

```sql
CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    log_type TEXT NOT NULL,  -- 'Disease', 'Deficiency', 'Maintenance', 'Alert'
    message TEXT NOT NULL,
    details TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

| Column | Type | Description |
|--------|------|-----------|
| log_type | TEXT | Disease, Deficiency, Maintenance, or Alert |
| message | TEXT | Log message |
| details | TEXT | Additional details |

#### 6. `plant_images`
Captured plant images.

```sql
CREATE TABLE IF NOT EXISTS plant_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    filename TEXT NOT NULL,
    filepath TEXT NOT NULL,
    captured_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    image_hash TEXT,
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

#### 7. `ml_predictions`
ML inference results (linked to images via foreign key).

```sql
CREATE TABLE IF NOT EXISTS ml_predictions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    image_id INTEGER NOT NULL,
    prediction_type TEXT NOT NULL,  -- 'Disease', 'Deficiency', 'Healthy'
    prediction_label TEXT NOT NULL,
    confidence REAL NOT NULL,
    bounding_box TEXT,  -- JSON string coordinates
    predicted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    model_version TEXT,
    FOREIGN KEY (image_id) REFERENCES plant_images(id) ON DELETE CASCADE
);
```

| Column | Type | Description |
|--------|------|-----------|
| image_id | INTEGER | Foreign key to plant_images |
| prediction_type | TEXT | Disease, Deficiency, Healthy, or Pest |
| prediction_label | TEXT | Specific label (e.g., "Tomato Leaf Mold") |
| confidence | REAL | Confidence score (0.0-1.0) |

#### 8. `health_assessments`
Plant health assessments.

```sql
CREATE TABLE IF NOT EXISTS health_assessments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    image_id INTEGER NOT NULL,
    health_score REAL NOT NULL,  -- 0.0 to 100.0
    health_status TEXT NOT NULL,  -- 'Excellent', 'Healthy', 'Warning', 'Critical'
    assessment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    assessment_details TEXT,
    FOREIGN KEY (image_id) REFERENCES plant_images(id) ON DELETE CASCADE
);
```

#### 9. `ml_detections`
Verified detections with feedback.

```sql
CREATE TABLE IF NOT EXISTS ml_detections (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    prediction_id INTEGER NOT NULL,
    is_verified INTEGER DEFAULT 0,
    actual_label TEXT,
    confidence_correct INTEGER,  -- 0 (False) or 1 (True)
    treatment_applied TEXT,
    notes TEXT,
    verified_at TIMESTAMP,
    action_logged INTEGER DEFAULT 0,
    FOREIGN KEY (prediction_id) REFERENCES ml_predictions(id) ON DELETE CASCADE
);
```

#### 10. `ml_recommendations`
ML-based treatment recommendations.

```sql
CREATE TABLE IF NOT EXISTS ml_recommendations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    prediction_id INTEGER NOT NULL,
    recommendation_type TEXT NOT NULL,
    recommendation_text TEXT NOT NULL,
    confidence REAL,
    generated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    user_acknowledged INTEGER DEFAULT 0,
    action_taken TEXT,
    action_date TIMESTAMP,
    outcome TEXT,
    outcome_date TIMESTAMP,
    FOREIGN KEY (prediction_id) REFERENCES ml_predictions(id) ON DELETE CASCADE
);
```

### Views

#### `vw_latest_sensor_reading`
Returns the most recent sensor reading.

```sql
CREATE VIEW IF NOT EXISTS vw_latest_sensor_reading AS
SELECT * FROM sensor_readings 
ORDER BY timestamp DESC 
LIMIT 1;
```

#### `vw_unread_alerts`
Returns all unread alerts, newest first.

```sql
CREATE VIEW IF NOT EXISTS vw_unread_alerts AS
SELECT * FROM alerts 
WHERE is_read = 0 
ORDER BY timestamp DESC;
```

#### `vw_daily_sensor_summary`
Daily aggregated sensor statistics.

```sql
CREATE VIEW IF NOT EXISTS vw_daily_sensor_summary AS
SELECT 
    strftime('%Y-%m-%d', timestamp) as day,
    ROUND(AVG(temperature), 2) as avg_temp,
    MIN(temperature) as min_temp,
    MAX(temperature) as max_temp,
    ROUND(AVG(ph), 2) as avg_ph,
    MIN(ph) as min_ph,
    MAX(ph) as max_ph,
    ROUND(AVG(ec), 2) as avg_ec,
    MIN(ec) as min_ec,
    MAX(ec) as max_ec,
    COUNT(*) as reading_count
FROM sensor_readings
GROUP BY day
ORDER BY day DESC;
```

#### `vw_pending_recommendations`
Pending (unacknowledged) recommendations.

```sql
CREATE VIEW IF NOT EXISTS vw_pending_recommendations AS
SELECT 
    r.id,
    r.recommendation_type,
    r.recommendation_text,
    r.confidence,
    r.generated_at,
    p.prediction_label,
    p.prediction_type
FROM ml_recommendations r
JOIN ml_predictions p ON r.prediction_id = p.id
WHERE r.user_acknowledged = 0
ORDER BY r.generated_at DESC;
```

## Indexes

```sql
-- Sensor readings (time-series queries)
CREATE INDEX idx_sensor_timestamp ON sensor_readings(timestamp);

-- Alerts (unread queries)
CREATE INDEX idx_alerts_is_read ON alerts(is_read);

-- Logs (category filtering)
CREATE INDEX idx_logs_type ON logs(log_type);

-- Plant images (recent images)
CREATE INDEX idx_images_captured_at ON plant_images(captured_at);

-- ML predictions (joins, confidence filtering)
CREATE INDEX idx_preds_image_id ON ml_predictions(image_id);
CREATE INDEX idx_preds_confidence ON ml_predictions(confidence);
CREATE INDEX idx_preds_type ON ml_predictions(prediction_type);

-- Health assessments (trend analysis)
CREATE INDEX idx_assess_date ON health_assessments(assessment_date);

-- ML detections (verification queries)
CREATE INDEX idx_detect_verified ON ml_detections(is_verified);
CREATE INDEX idx_detect_verified_at ON ml_detections(verified_at);

-- ML recommendations (pending actions)
CREATE INDEX idx_recs_ack ON ml_recommendations(user_acknowledged);
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
