# LeafSense - Project Demonstration Guide

## Overview

This guide provides step-by-step instructions to demonstrate all functionalities of the LeafSense system for evaluation, presentation, or testing purposes.

---

## Table of Contents

1. [Pre-Demonstration Setup](#pre-demonstration-setup)
2. [System Startup](#system-startup)
3. [Feature Demonstrations](#feature-demonstrations)
4. [Command Reference](#command-reference)
5. [Troubleshooting Demo Issues](#troubleshooting-demo-issues)

---

## Pre-Demonstration Setup

### 1. Hardware Checklist

Ensure all hardware is connected and powered:

- [x] Raspberry Pi 4B (2GB RAM) powered via USB-C (5V/3A)
- [x] Waveshare 3.5" LCD (C) touchscreen connected to GPIO header
- [x] Raspberry Pi Camera Module v1 (OV5647) connected to CSI port
- [x] Network cable connected (or WiFi configured)
- [x] SSH access available from development machine

**Optional** (if available):
- [ ] DS18B20 temperature sensor on GPIO 19
- [ ] pH sensor via I2C (ADS1115 at 0x48)
- [ ] TDS sensor via I2C (ADS1115 at 0x48)
- [ ] Dosing pumps on GPIO 5, 6, 13
- [ ] Heater relay on GPIO 26
- [ ] Alert LED

### 2. Network Configuration

**From your development machine:**

```bash
# Test connectivity
ping 10.42.0.196

# Should respond:
# 64 bytes from 10.42.0.196: icmp_seq=1 ttl=64 time=1.2 ms
```

If no response:
```bash
# Check USB-Ethernet interface (adjust IP if needed)
sudo ip addr add 10.42.0.1/24 dev usb0
sudo ip link set usb0 up
```

### 3. SSH Access Test

```bash
# Connect to Raspberry Pi
ssh root@10.42.0.196
# Password: (set during deployment, default is empty or 'root')

# Verify system is running
uname -a
# Should show: Linux leafsense 6.12.41-v8 #1 SMP PREEMPT aarch64 GNU/Linux

# Check LeafSense files
ls -lh /opt/leafsense/
# Should show: LeafSense binary, database/, gallery/, resources/
```

---

## System Startup

### Method 1: Automatic Startup (On Boot)

If LeafSense is configured for auto-start:

```bash
# Reboot the Raspberry Pi
ssh root@10.42.0.196 'reboot'

# Wait 30-45 seconds for boot

# Verify LeafSense is running
ssh root@10.42.0.196 'ps aux | grep LeafSense | grep -v grep'
# Should show: LeafSense process with -platform linuxfb:fb=/dev/fb1

# Check the touchscreen - should display login screen
```

### Method 2: Manual Startup

```bash
# SSH into Raspberry Pi
ssh root@10.42.0.196

# Stop any running instance
killall LeafSense 2>/dev/null

# Start LeafSense with correct environment
cd /opt/leafsense
env \
    QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1 \
    QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins \
    QT_QPA_FONTDIR=/usr/share/fonts \
    QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx" \
    ./LeafSense -platform linuxfb:fb=/dev/fb1 > /var/log/leafsense.log 2>&1 &

# Verify startup
sleep 3
tail -20 /var/log/leafsense.log
```

**Expected output:**
```
[Database] Connected: /opt/leafsense/database/leafsense.db
[ML] Model loaded successfully: ./leafsense_model.onnx
[GUI] Login dialog shown
```

### Method 3: Using Startup Script

```bash
ssh root@10.42.0.196 '/opt/leafsense/start.sh &'
```

---

## Feature Demonstrations

### Demo 1: Login & Dashboard

**Objective**: Show user authentication and main interface

**Steps**:
1. On the touchscreen, you should see the login dialog
2. Default credentials:
   - **Username**: `admin`
   - **Password**: `admin`
3. Touch the "Login" button or press Enter
4. Dashboard appears showing:
   - Real-time sensor values (Temperature, pH, EC)
   - Plant health assessment
   - Alert status

**What to observe**:
- Touch responsiveness (cursor follows pen accurately due to evdev with rotate=180:invertx)
- Sensor values updating every ~2 seconds
- GUI rendering on 480x320 display

**Commands to monitor**:
```bash
# Watch sensor readings in real-time
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log | grep SENSOR'

# Example output:
# [Daemon] Processed: SENSOR|23.5|6.45|1250
# [Daemon] Processed: SENSOR|23.6|6.46|1252
```

---

### Demo 2: Real-Time Sensor Monitoring

**Objective**: Demonstrate sensor data acquisition and display

**Current Status**: Mock sensors (random values)

**Steps**:
1. On dashboard, observe sensor displays updating
2. Values change every 2 seconds:
   - **Temperature**: 18-26°C range
   - **pH**: 5.5-7.5 range
   - **EC**: 800-1500 ppm range

**Database Integration**:
```bash
# SSH into Pi
ssh root@10.42.0.196

# Query recent sensor readings
sqlite3 /opt/leafsense/database/leafsense.db \
  "SELECT datetime(timestamp, 'localtime'), temperature, ph, ec 
   FROM sensor_readings 
   ORDER BY timestamp DESC 
   LIMIT 10;"

# Expected output:
# 2026-01-10 01:23:45|23.5|6.45|1250
# 2026-01-10 01:23:43|23.6|6.46|1252
```

**What to demonstrate**:
- Real-time updates on GUI
- Data persistence in SQLite database
- Timestamp accuracy

---

### Demo 3: Plant Health Assessment (ML)

**Objective**: Show ML-based disease detection

**Current Status**: 
- Model loaded: `leafsense_model.onnx` (5.9 MB, 4 classes)
- Camera integration: Implemented (periodic capture every 30 min)
- Classes: Healthy, Disease, Nutrient Deficiency, Pest Damage

**Steps**:

1. **View Current Health Status**:
   - Dashboard shows "Health Score: XX%"
   - Status shows classification (e.g., "Healthy")

2. **Manual Camera Capture** (if needed for demo):
   ```bash
   # SSH into Pi
   ssh root@10.42.0.196
   
   # Modify counter to trigger immediate capture
   # (Already in code - captures every 30 min automatically)
   
   # Monitor for capture event
   tail -f /var/log/leafsense.log | grep -E "Camera|ML"
   
   # Expected:
   # [Master] Capturing photo for ML analysis...
   # [Camera] Photo captured: /opt/leafsense/gallery/plant_20260110_123456.jpg
   # [ML] Prediction: Healthy (confidence: 95.3%)
   ```

3. **View Captured Images**:
   - Touch "Analytics" button on dashboard
   - Select "Gallery" tab
   - Images display with timestamp and prediction
   - Use < > buttons to navigate between images

**What to demonstrate**:
- ONNX Runtime inference (CPU-based)
- Model predictions with confidence scores
- Image gallery integration
- Database logging of ML results

---

### Demo 4: Automatic Control System

**Objective**: Demonstrate autonomous actuator control

**Current Status**: Mock actuators (log-based simulation)

**Control Logic** (implemented in Master.cpp):

| Condition | Action | Threshold |
|-----------|--------|-----------|
| pH < 6.0 | Activate pH Up pump | 500ms dose |
| pH > 7.0 | Activate pH Down pump | 500ms dose |
| Temp < 20°C | Turn ON heater | Until > 22°C |
| Temp > 24°C | Turn OFF heater | Until < 22°C |
| EC < 1000 | Activate nutrient pump | 500ms dose |

**Steps to demonstrate**:

1. **Monitor Control Actions**:
   ```bash
   ssh root@10.42.0.196 'tail -f /var/log/leafsense.log | grep -E "Pump|Heater"'
   
   # Example output:
   # [Pump 13] Dosing for 500ms
   # [Daemon] Processed: LOG|Maintenance|pH Down|Dosed 500ms
   # [Heater] State: ON
   ```

2. **View Control History**:
   - Touch "Logs" button on dashboard
   - Filter by "Maintenance" category
   - Shows all pump activations and heater toggles

3. **Database Query**:
   ```bash
   sqlite3 /opt/leafsense/database/leafsense.db \
     "SELECT datetime(timestamp, 'localtime'), action, details 
      FROM logs 
      WHERE category='Maintenance' 
      ORDER BY timestamp DESC 
      LIMIT 10;"
   ```

**What to demonstrate**:
- Autonomous decision-making based on sensor data
- Hysteresis in temperature control (prevents rapid cycling)
- Safety limits (max 500ms pump duration)
- Logging of all control actions

---

### Demo 5: Historical Data & Analytics

**Objective**: Show data visualization and trend analysis

**Steps**:

1. **Open Analytics Window**:
   - Touch "Analytics" button on main dashboard
   - Tab view appears with 3 tabs

2. **Tab 1: Sensor Readings Table**:
   - Shows last 50 sensor readings
   - Columns: Timestamp, Temperature, pH, EC
   - Scrollable table

3. **Tab 2: Trends Chart**:
   - Line chart with 3 series (Temperature, pH, EC)
   - X-axis: Time
   - Y-axis: Values (auto-scaled)
   - Shows last 100 data points

4. **Tab 3: Gallery**:
   - Displays captured plant images
   - Shows ML predictions
   - Navigation controls (< >)
   - "Confirm Issue" button for validation

**Command to populate more data** (for demo):
```bash
# Generate sample historical data
ssh root@10.42.0.196 << 'EOF'
sqlite3 /opt/leafsense/database/leafsense.db << SQL
-- Insert sample readings for the past hour
INSERT INTO sensor_readings (temperature, ph, ec, timestamp) VALUES
  (22.5, 6.5, 1200, datetime('now', '-60 minutes')),
  (22.7, 6.4, 1210, datetime('now', '-55 minutes')),
  (22.9, 6.6, 1205, datetime('now', '-50 minutes')),
  (23.1, 6.5, 1198, datetime('now', '-45 minutes')),
  (23.3, 6.7, 1215, datetime('now', '-40 minutes'));
SQL
EOF

# Restart to reload data
ssh root@10.42.0.196 'killall LeafSense; /opt/leafsense/start.sh &'
```

---

### Demo 6: System Configuration

**Objective**: Show settings and customization

**Steps**:

1. **Open Settings Window**:
   - Touch "Settings" button
   - Settings dialog appears

2. **Ideal Conditions**:
   - View/edit ideal ranges:
     - Temperature: 20-24°C
     - pH: 6.0-7.0
     - EC: 1000-1500 ppm

3. **Theme Selection**:
   - Toggle between Light/Dark themes
   - Observe GUI color changes

4. **Database Management**:
   - View database location
   - Check database size
   - Option to export data (if implemented)

5. **System Information**:
   - Touch "Info" button
   - Shows:
     - LeafSense version
     - Raspberry Pi model
     - CPU temperature
     - Uptime
     - Network info

---

### Demo 7: Alert System

**Objective**: Demonstrate alert detection and LED notification

**Current Status**: 
- Alert detection: ✅ Implemented
- LED kernel module: ✅ Compiled
- LED activation: Requires hardware connection

**Steps**:

1. **View Alerts Panel**:
   - Dashboard shows "Alerts" section
   - Lists active alerts (pH/Temp/EC out of range)

2. **Alert LED** (if hardware connected):
   ```bash
   # Load LED kernel module
   ssh root@10.42.0.196 'insmod /lib/modules/$(uname -r)/extra/ledmodule.ko'
   
   # Verify module loaded
   ssh root@10.42.0.196 'lsmod | grep led'
   
   # Check device
   ssh root@10.42.0.196 'ls -l /dev/led0'
   
   # Manual test
   ssh root@10.42.0.196 'echo 1 > /dev/led0'  # LED ON
   ssh root@10.42.0.196 'echo 0 > /dev/led0'  # LED OFF
   ```

3. **Alert Persistence**:
   ```bash
   sqlite3 /opt/leafsense/database/leafsense.db \
     "SELECT datetime(timestamp, 'localtime'), category, action, details 
      FROM logs 
      WHERE category='Alert' 
      ORDER BY timestamp DESC 
      LIMIT 5;"
   ```

**What to demonstrate**:
- Real-time alert detection
- Visual feedback (LED)
- Alert history in logs
- Kernel module integration

---

### Demo 8: Database Operations

**Objective**: Show database structure and query capabilities

**Commands**:

1. **Connect to Database**:
   ```bash
   ssh root@10.42.0.196
   sqlite3 /opt/leafsense/database/leafsense.db
   ```

2. **View Schema**:
   ```sql
   .schema
   
   -- Shows all tables:
   -- users, sensor_readings, ideal_conditions, logs,
   -- plant_images, ml_predictions, ml_detections
   ```

3. **Query Examples**:
   ```sql
   -- Latest sensor reading
   SELECT * FROM sensor_readings ORDER BY timestamp DESC LIMIT 1;
   
   -- Average values last hour
   SELECT 
     AVG(temperature) as avg_temp,
     AVG(ph) as avg_ph,
     AVG(ec) as avg_ec
   FROM sensor_readings
   WHERE timestamp > datetime('now', '-1 hour');
   
   -- Count by log category
   SELECT category, COUNT(*) as count
   FROM logs
   GROUP BY category
   ORDER BY count DESC;
   
   -- ML predictions summary
   SELECT predicted_class, COUNT(*) as count
   FROM ml_predictions
   GROUP BY predicted_class;
   
   -- Exit
   .quit
   ```

4. **Database Backup**:
   ```bash
   # Create backup
   ssh root@10.42.0.196 'sqlite3 /opt/leafsense/database/leafsense.db .dump' > leafsense_backup.sql
   
   # Check backup size
   ls -lh leafsense_backup.sql
   ```

---

### Demo 9: Touchscreen Calibration

**Objective**: Verify touchscreen accuracy

**Calibration**: ✅ Already configured with `rotate=180:invertx`

**Test**:
1. Touch each corner of the screen
2. Verify cursor appears at touch point
3. Test button responsiveness:
   - Login button
   - Navigation buttons (Analytics, Settings, Logs)
   - Gallery navigation (< >)

**If calibration is off**:
```bash
# Run calibration test
ssh root@10.42.0.196 '/tmp/calib_test.sh'

# Outputs raw coordinates for corner touches
# Adjust rotate parameter if needed
```

---

### Demo 10: System Performance

**Objective**: Show system resource usage and stability

**Commands**:

1. **CPU & Memory Usage**:
   ```bash
   ssh root@10.42.0.196 'top -b -n 1 | head -15'
   
   # Look for:
   # LeafSense CPU: should be < 30%
   # Memory: should be < 500MB
   ```

2. **CPU Temperature**:
   ```bash
   ssh root@10.42.0.196 'cat /sys/class/thermal/thermal_zone0/temp'
   # Divide by 1000 for °C (e.g., 45000 = 45°C)
   ```

3. **Disk Usage**:
   ```bash
   ssh root@10.42.0.196 'df -h'
   
   # Check /opt/leafsense usage
   ssh root@10.42.0.196 'du -sh /opt/leafsense/*'
   ```

4. **Uptime & Stability**:
   ```bash
   ssh root@10.42.0.196 'uptime'
   # Shows: load average and uptime
   
   # Check for crashes
   ssh root@10.42.0.196 'dmesg | tail -50 | grep -i error'
   ```

5. **Thread Status**:
   ```bash
   ssh root@10.42.0.196 'ps -eLf | grep LeafSense'
   # Shows all threads (timer, sensors, actuators, etc.)
   ```

---

## Command Reference

### Quick Commands Cheat Sheet

```bash
# === SYSTEM CONTROL ===
ssh root@10.42.0.196                              # Connect to Pi
reboot                                             # Restart Pi
poweroff                                           # Shutdown Pi

# === LeafSense CONTROL ===
killall LeafSense                                  # Stop application
/opt/leafsense/start.sh &                         # Start application
ps aux | grep LeafSense                            # Check if running
tail -f /var/log/leafsense.log                    # View live logs

# === MONITORING ===
tail -f /var/log/leafsense.log | grep SENSOR      # Watch sensor data
tail -f /var/log/leafsense.log | grep Pump        # Watch pump activity
tail -f /var/log/leafsense.log | grep ML          # Watch ML predictions

# === DATABASE ===
sqlite3 /opt/leafsense/database/leafsense.db      # Open database
sqlite3 /opt/leafsense/database/leafsense.db ".tables"  # List tables
sqlite3 /opt/leafsense/database/leafsense.db \
  "SELECT * FROM sensor_readings ORDER BY timestamp DESC LIMIT 10;"

# === CAMERA ===
ls -lh /opt/leafsense/gallery/                    # List captured images
cat /sys/class/video4linux/video0/name            # Check camera device

# === LED MODULE ===
insmod /lib/modules/$(uname -r)/extra/ledmodule.ko    # Load module
lsmod | grep led                                  # Check if loaded
echo 1 > /dev/led0                              # LED ON
echo 0 > /dev/led0                              # LED OFF
rmmod ledmodule                                   # Unload module

# === TOUCHSCREEN ===
cat /proc/bus/input/devices | grep -A5 ADS7846   # Check touch device
hexdump -C /dev/input/event0 | head -20           # View raw touch events

# === NETWORK ===
ip addr show usb0                                 # Check USB network
ping 10.42.0.196                                  # Test connectivity
ifconfig                                          # View all interfaces

# === SYSTEM INFO ===
uname -a                                          # Kernel version
cat /proc/cpuinfo | grep Model                    # Pi model
cat /sys/class/thermal/thermal_zone0/temp         # CPU temp (÷1000)
free -h                                           # Memory usage
df -h                                             # Disk usage
uptime                                            # System uptime

# === GPIO (if libgpiod installed) ===
gpiodetect                                        # List GPIO chips
gpioinfo                                          # List all GPIO lines
gpioset gpiochip0 13=1                            # Set GPIO 13 HIGH
gpioget gpiochip0 13                              # Read GPIO 13

# === BUILDROOT PACKAGE CHECK ===
opkg list-installed | grep opencv                 # Check OpenCV
opkg list-installed | grep qt5                    # Check Qt5
opkg list-installed | grep sqlite                 # Check SQLite
```

---

## Troubleshooting Demo Issues

### Issue 1: LeafSense doesn't start

**Symptoms**: Black screen or application crashes

**Diagnosis**:
```bash
# Check log for errors
ssh root@10.42.0.196 'cat /var/log/leafsense.log | grep -i error'

# Check if binary exists
ssh root@10.42.0.196 'ls -lh /opt/leafsense/LeafSense'

# Check library dependencies
ssh root@10.42.0.196 'ldd /opt/leafsense/LeafSense | grep "not found"'
```

**Solutions**:
```bash
# Redeploy binary
scp build-arm64/LeafSense root@10.42.0.196:/opt/leafsense/

# Restart with debug output
ssh root@10.42.0.196 'cd /opt/leafsense && ./LeafSense -platform linuxfb:fb=/dev/fb1 2>&1 | tee /tmp/debug.log'
```

### Issue 2: Touchscreen not responding

**Diagnosis**:
```bash
# Check touch device
ssh root@10.42.0.196 'ls -l /dev/input/event0'

# Test touch events
ssh root@10.42.0.196 'hexdump -C /dev/input/event0' &
# Touch screen, should see data

# Check environment variable
ssh root@10.42.0.196 'ps aux | grep QT_QPA_EVDEV'
```

**Solution**:
```bash
# Restart with correct touch parameter
ssh root@10.42.0.196 'killall LeafSense; cd /opt/leafsense && \
  QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx" \
  ./LeafSense -platform linuxfb:fb=/dev/fb1 > /var/log/leafsense.log 2>&1 &'
```

### Issue 3: Database locked

**Symptoms**: "database is locked" errors

**Solution**:
```bash
# Check for multiple instances
ssh root@10.42.0.196 'ps aux | grep LeafSense'

# Kill all instances
ssh root@10.42.0.196 'killall -9 LeafSense'

# Remove lock file if exists
ssh root@10.42.0.196 'rm -f /opt/leafsense/database/leafsense.db-wal'

# Restart
ssh root@10.42.0.196 '/opt/leafsense/start.sh &'
```

### Issue 4: Camera not capturing

**Diagnosis**:
```bash
# Check camera device
ssh root@10.42.0.196 'ls -l /dev/video0'

# Check camera name
ssh root@10.42.0.196 'cat /sys/class/video4linux/video0/name'

# Check OpenCV version
ssh root@10.42.0.196 'ldd /opt/leafsense/LeafSense | grep opencv'
```

**Temporary Workaround**:
See sensor integration guide for alternative camera capture methods.

### Issue 5: GUI rendering issues

**Symptoms**: Distorted display or missing elements

**Solution**:
```bash
# Check framebuffer
ssh root@10.42.0.196 'fbset -fb /dev/fb1'

# Should show: 480x320

# Restart with correct framebuffer
ssh root@10.42.0.196 'killall LeafSense && \
  QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1 ./LeafSense ...'
```

---

## Presentation Script

### 5-Minute Demo Flow

**Minute 1: Introduction & System Overview**
- Show Raspberry Pi with touchscreen
- Explain hardware (sensors, actuators, display)
- Mention technologies (Qt5, SQLite, ONNX Runtime)

**Minute 2: User Interface & Real-Time Monitoring**
- Touch screen to login
- Show dashboard with live sensor data
- Explain update frequency (2 seconds)
- Open Analytics window, show charts

**Minute 3: Automatic Control System**
- Open terminal, show logs with pump/heater activity
- Explain control logic (pH adjustment, temperature control)
- Query database to show logged actions

**Minute 4: Machine Learning & Image Gallery**
- Show gallery with captured images
- Explain ML model (4 classes, 99.39% accuracy)
- Demonstrate navigation and predictions
- Show ML logs

**Minute 5: System Architecture & Future Work**
- Show database schema in terminal
- Explain multi-threaded architecture
- Discuss sensor integration (next phase)
- Q&A

---

## Recording Demo for Submission

If creating a video demonstration:

1. **Setup screen recording**:
   ```bash
   # On development machine, record SSH session
   asciinema rec leafsense_demo.cast
   
   # Or use OBS Studio to record:
   # - Terminal window (SSH session)
   # - Webcam/phone pointed at touchscreen
   # - Split-screen view
   ```

2. **Script narration**:
   - Introduce project and objectives
   - Walk through each feature
   - Show both GUI (touchscreen) and terminal (logs/database)
   - Demonstrate responsiveness and stability

3. **Export commands**:
   ```bash
   # Generate PDF report from database
   ssh root@10.42.0.196 'sqlite3 /opt/leafsense/database/leafsense.db' << SQL
   .mode markdown
   .output /tmp/sensor_summary.md
   SELECT 
     DATE(timestamp) as date,
     ROUND(AVG(temperature), 2) as avg_temp,
     ROUND(AVG(ph), 2) as avg_ph,
     ROUND(AVG(ec), 0) as avg_ec,
     COUNT(*) as readings
   FROM sensor_readings
   GROUP BY DATE(timestamp)
   ORDER BY date DESC;
   SQL
   
   # Download report
   scp root@10.42.0.196:/tmp/sensor_summary.md ./
   ```

---

**Authors**: Daniel Cardoso, Marco Costa  
**Course**: Embedded Systems and Computers  
**Last Updated**: January 10, 2026  
**Version**: 1.0
