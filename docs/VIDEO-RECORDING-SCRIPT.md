# LeafSense Demo Video Recording Script

**Project:** LeafSense - Real-Time Hydroponic Plant Monitoring System  
**Duration:** Approximately 8-10 minutes  
**Equipment Needed:** 
- Screen recording software (OBS Studio, Kazam, or similar)
- Camera for physical device shots (optional)
- Raspberry Pi 4 with LeafSense running
- SSH access from host machine (for terminal demonstrations)

---

## Pre-Recording Checklist

- [ ] LeafSense application running on Pi (`ps aux | grep LeafSense`)
- [ ] LED module loaded (`lsmod | grep led`)
- [ ] Database has sample data (sensor readings, alerts, predictions)
- [ ] Camera connected and functional (`ls /dev/video0`)
- [ ] Host machine connected to Pi via SSH
- [ ] Screen recording software configured (1080p, 30fps recommended)
- [ ] Microphone tested (if doing voiceover)

---

## Video Structure

### PART 1: Introduction (30 seconds)
**[Screen: Title slide or project logo]**

**Script:**
> "Welcome to the LeafSense demonstration. LeafSense is a real-time hydroponic plant monitoring system developed for the Embedded Systems course at University of Minho. 
> 
> The system combines environmental sensing, machine learning for plant disease detection, and a touchscreen GUI, all running on a custom Buildroot Linux image on a Raspberry Pi 4."

---

### PART 2: System Overview (1 minute)
**[Screen: SSH terminal showing system info]**

**Commands to run:**
```bash
ssh root@10.42.0.196

# Show OS info
cat /etc/os-release

# Show kernel and architecture
uname -a

# Show running application
ps aux | grep LeafSense

# Show database exists
ls -la /opt/leafsense/
```

**Script:**
> "Here we have the Raspberry Pi 4 running our custom Buildroot 2025.08 Linux image. The kernel is version 6.12.41 for ARM64 architecture.
>
> The LeafSense application is running, and we can see the database file, the ONNX machine learning model, and the gallery folder for captured plant images."

---

### PART 3: GUI Demonstration (3 minutes)

#### 3.1 Login Screen (30 seconds)
**[Screen: Remote framebuffer capture or physical device]**

**Script:**
> "The application starts with a login screen for secure access. Let me enter the credentials..."

**Action:** Enter username `admin`, password `admin`

---

#### 3.2 Main Dashboard (45 seconds)
**[Screen: Main dashboard view]**

**Script:**
> "This is the main dashboard showing:
> - Current plant health status
> - Real-time sensor readings: temperature, pH, and electrical conductivity
> - Recent alerts and navigation buttons
> 
> The values update automatically from our sensor threads running in the background."

---

#### 3.3 Analytics Window (45 seconds)
**[Screen: Navigate through Analytics tabs]**

**Script:**
> "The Analytics window has three tabs:
> - **Sensor Readings** - showing current environmental data
> - **Trends** - displaying historical graphs over time
> - **Gallery** - where we can see captured plant images"

**Action:** Click through each tab

---

#### 3.4 Logs Window (30 seconds)
**[Screen: Logs window with tabs]**

**Script:**
> "The Logs window categorizes all system events into four tabs: Alerts, Diseases, Deficiencies, and Maintenance. This provides a complete audit trail of plant care activities."

**Action:** Show each tab

---

#### 3.5 Settings and Dark Mode (30 seconds)
**[Screen: Settings window, then toggle dark mode]**

**Script:**
> "In Settings, users can configure sensor thresholds and enable dark mode for low-light environments."

**Action:** Toggle to dark mode, show GUI in dark theme

---

### PART 4: LED Device Driver (2 minutes)
**[Screen: SSH terminal]**

**Script:**
> "Now let's demonstrate our custom Linux kernel module for LED control. This driver uses memory-mapped I/O to control GPIO pin 20."

**Commands to run:**
```bash
# Load the module
insmod /root/led.ko

# Verify loading
lsmod | grep led
dmesg | tail -10

# Show device file
ls -la /dev/led0

# Control the LED
echo '1' > /dev/led0    # Turn ON
# (Pause to show LED is on)
echo '0' > /dev/led0    # Turn OFF

# Check kernel logs
dmesg | grep -E 'led|SetGPIO'

# Unload module
rmmod led
dmesg | tail -5
```

**Script points:**
- Explain module initialization in kernel space
- Show character device creation at /dev/led0
- Demonstrate ON/OFF control
- Explain GPIO function configuration in dmesg output
- Show clean unloading

---

### PART 5: Machine Learning Pipeline (1.5 minutes)
**[Screen: SSH terminal and/or GUI]**

**Script:**
> "The machine learning pipeline uses ONNX Runtime for plant health classification. Let me show the model and a prediction..."

**Commands to run:**
```bash
# Show model files
ls -la /opt/leafsense/leafsense_model.onnx
cat /opt/leafsense/leafsense_model_classes.txt

# Show recent prediction in logs
tail -50 /var/log/leafsense.log | grep -E 'ML|Prediction|Camera'

# Or query the database
sqlite3 /opt/leafsense/leafsense.db "SELECT * FROM ml_predictions ORDER BY predicted_at DESC LIMIT 3;"
```

**Script points:**
- Show 4-class model (healthy, disease, deficiency, pest)
- Demonstrate confidence scores
- Show prediction stored in database

---

### PART 6: Database System (1 minute)
**[Screen: SSH terminal with SQLite]**

**Commands to run:**
```bash
sqlite3 /opt/leafsense/leafsense.db

# In SQLite:
.tables
.schema sensor_readings
SELECT * FROM sensor_readings ORDER BY timestamp DESC LIMIT 5;
SELECT * FROM alerts WHERE is_read = 0;
.quit
```

**Script:**
> "All data is persisted in SQLite3. We have tables for sensor readings, alerts, ML predictions, and system logs. Here we can see recent temperature, pH, and EC readings with timestamps."

---

### PART 7: Hardware Drivers (1 minute)
**[Screen: SSH terminal]**

**Script:**
> "The system uses three hardware drivers for peripherals..."

**Commands to run:**
```bash
# Display driver
lsmod | grep -E 'ili9486|fbtft'
cat /sys/class/graphics/fb1/name

# Touchscreen driver  
lsmod | grep ads7846
cat /proc/bus/input/devices | grep -A 3 "ADS7846"

# Camera driver
lsmod | grep ov5647
ls /dev/video0
```

**Script points:**
- ILI9486 display on SPI → framebuffer fb1
- ADS7846 resistive touch → /dev/input/event0
- OV5647 camera → /dev/video0

---

### PART 8: Conclusion (30 seconds)
**[Screen: Dashboard or title slide]**

**Script:**
> "To summarize, LeafSense demonstrates:
> - A complete embedded Linux system using Buildroot
> - Custom kernel module development for GPIO control
> - Real-time sensor acquisition with POSIX threads
> - Machine learning inference on edge devices
> - A responsive Qt GUI optimized for touchscreen
> 
> Thank you for watching this demonstration of the LeafSense hydroponic monitoring system."

---

## Post-Production Notes

### Video Title Suggestions:
- "LeafSense Demo - Embedded Hydroponic Monitoring System"
- "LeafSense: Real-Time Plant Health Monitoring on Raspberry Pi"

### Recommended Edits:
1. Add title cards between sections
2. Include zooms/callouts for important terminal output
3. Add background music (quiet, non-distracting)
4. Include text annotations for technical terms
5. Add project logo watermark in corner

### Export Settings:
- Resolution: 1920x1080 (1080p)
- Frame Rate: 30 fps
- Format: MP4 (H.264)
- Bitrate: 10-15 Mbps

---

## Quick Reference Commands

```bash
# Connect to Pi
ssh root@10.42.0.196

# Check app running
ps aux | grep LeafSense

# LED demo (quick)
insmod /root/led.ko && echo '1' > /dev/led0
sleep 2
echo '0' > /dev/led0 && rmmod led

# Take screenshot remotely
./deploy/screenshot_pi.sh gui_demo

# Restart application
pkill LeafSense
cd /opt/leafsense && ./start.sh &

# Check sensors
sqlite3 /opt/leafsense/leafsense.db "SELECT * FROM sensor_readings ORDER BY timestamp DESC LIMIT 1;"
```

---

## Fallback Plans

### If GUI freezes:
```bash
pkill LeafSense
export QT_QPA_PLATFORM='linuxfb:fb=/dev/fb1:size=480x320'
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS='/dev/input/event0:rotate=180:invertx'
cd /opt/leafsense && ./LeafSense &
```

### If LED module fails:
```bash
rmmod led 2>/dev/null
dmesg -c > /dev/null
insmod /root/led.ko
```

### If no camera:
- Skip Part 5 live demo
- Show pre-captured ml_inference_result.png instead
