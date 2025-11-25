-- LeafSense Database Schema
-- SQLite3 Syntax
-- V1.0

-- Enable Foreign Keys (Crucial for ML cascading deletes)
PRAGMA foreign_keys = ON;

-- 1. USER TABLE
-- Constraint: Single user system (id=1)
CREATE TABLE IF NOT EXISTS user (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);

-- 2. PLANT TABLE
-- Constraint: Single plant system (id=1)
CREATE TABLE IF NOT EXISTS plant (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    name TEXT NOT NULL,
    planted_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. SENSOR_READINGS TABLE
CREATE TABLE IF NOT EXISTS sensor_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL,
    ph REAL,
    ec REAL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- ERD [INDEX]: Optimized for time-series plotting
CREATE INDEX IF NOT EXISTS idx_sensor_timestamp ON sensor_readings(timestamp);

-- 4. ALERTS TABLE
CREATE TABLE IF NOT EXISTS alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL, -- 'Warning', 'Critical', 'Info'
    message TEXT NOT NULL,
    details TEXT,
    is_read INTEGER DEFAULT 0, -- 0 for Unread, 1 for Read
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- ERD [INDEX]: Optimized for "Show me unread alerts"
CREATE INDEX IF NOT EXISTS idx_alerts_is_read ON alerts(is_read);

-- 5. LOGS TABLE
CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    log_type TEXT NOT NULL, -- 'Disease', 'Deficiency', 'Maintenance', 'Alert'
    message TEXT NOT NULL,
    details TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- ERD [INDEX]: Optimized for filtering logs by category tabs
CREATE INDEX IF NOT EXISTS idx_logs_type ON logs(log_type);

-- 6. PLANT_IMAGES TABLE
CREATE TABLE IF NOT EXISTS plant_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    filename TEXT NOT NULL,
    filepath TEXT NOT NULL,
    captured_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    image_hash TEXT,
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- ERD [INDEX]: Optimized for retrieving recent images
CREATE INDEX IF NOT EXISTS idx_images_captured_at ON plant_images(captured_at);

-- 7. ML_PREDICTIONS TABLE
CREATE TABLE IF NOT EXISTS ml_predictions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    image_id INTEGER NOT NULL,
    prediction_type TEXT NOT NULL, -- 'Disease', 'Deficiency', 'Healthy'
    prediction_label TEXT NOT NULL,
    confidence REAL NOT NULL,
    bounding_box TEXT, -- JSON string coordinates
    predicted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    model_version TEXT,
    FOREIGN KEY (image_id) REFERENCES plant_images(id) ON DELETE CASCADE
);
-- ERD [INDEX]: Optimized for joining with images
CREATE INDEX IF NOT EXISTS idx_preds_image_id ON ml_predictions(image_id);
-- ERD [INDEX]: Optimized for filtering high-confidence predictions
CREATE INDEX IF NOT EXISTS idx_preds_confidence ON ml_predictions(confidence);
-- ERD [INDEX]: Optimized for grouping by type
CREATE INDEX IF NOT EXISTS idx_preds_type ON ml_predictions(prediction_type);

-- 8. HEALTH_ASSESSMENTS TABLE
CREATE TABLE IF NOT EXISTS health_assessments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    image_id INTEGER NOT NULL,
    health_score REAL NOT NULL, -- 0.0 to 100.0
    health_status TEXT NOT NULL, -- 'Excellent', 'Healthy', 'Warning', 'Critical'
    assessment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    assessment_details TEXT,
    FOREIGN KEY (image_id) REFERENCES plant_images(id) ON DELETE CASCADE
);
-- ERD [INDEX]: Optimized for trend analysis graphs
CREATE INDEX IF NOT EXISTS idx_assess_date ON health_assessments(assessment_date);

-- 9. ML_DETECTIONS TABLE
CREATE TABLE IF NOT EXISTS ml_detections (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    prediction_id INTEGER NOT NULL,
    is_verified INTEGER DEFAULT 0,
    actual_label TEXT,
    confidence_correct INTEGER, -- 0 (False) or 1 (True)
    treatment_applied TEXT,
    notes TEXT,
    verified_at TIMESTAMP,
    action_logged INTEGER DEFAULT 0,
    FOREIGN KEY (prediction_id) REFERENCES ml_predictions(id) ON DELETE CASCADE
);
-- ERD [INDEX]: Optimized for finding unverified detections
CREATE INDEX IF NOT EXISTS idx_detect_verified ON ml_detections(is_verified);
-- ERD [INDEX]: Optimized for reporting verification history
CREATE INDEX IF NOT EXISTS idx_detect_verified_at ON ml_detections(verified_at);

-- 10. ML_RECOMMENDATIONS TABLE
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
-- ERD [INDEX]: Optimized for the "Actions Needed" dashboard panel
CREATE INDEX IF NOT EXISTS idx_recs_ack ON ml_recommendations(user_acknowledged);

-- ==========================================
-- VIEWS (Virtual Tables for Analytics)
-- ==========================================

-- View 1: Latest Sensor Reading
-- Used for: Main Dashboard "Current Status" cards
CREATE VIEW IF NOT EXISTS vw_latest_sensor_reading AS
SELECT * FROM sensor_readings 
ORDER BY timestamp DESC 
LIMIT 1;

-- View 2: Unread Alerts
-- Used for: Notification badges and Alert Panel
CREATE VIEW IF NOT EXISTS vw_unread_alerts AS
SELECT * FROM alerts 
WHERE is_read = 0 
ORDER BY timestamp DESC;

-- View 3: Daily Sensor Summary
-- Used for: Analytics charts (daily averages, min/max trends)
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

-- View 4: Pending Recommendations
-- Used for: "Actions Needed" panel
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

-- ==========================================
-- SEED DATA (Initial Configuration)
-- ==========================================
INSERT OR IGNORE INTO user (id, username, password_hash) 
VALUES (1, 'admin', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918');

INSERT OR IGNORE INTO plant (id, name) 
VALUES (1, 'Lettuce');