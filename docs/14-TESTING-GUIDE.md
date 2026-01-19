# LeafSense - Complete Testing Guide

This document provides step-by-step commands for testing every component of the LeafSense system on Raspberry Pi 4B with Waveshare 3.5" touchscreen display.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Network Configuration](#network-configuration)
3. [SD Card Preparation](#sd-card-preparation)
4. [First Boot](#first-boot)
5. [SSH Connection](#ssh-connection)
6. [Touchscreen Configuration](#touchscreen-configuration)
7. [Application Deployment](#application-deployment)
8. [Database Initialization](#database-initialization)
9. [Application Testing](#application-testing)
10. [Quick Reference](#quick-reference)

---

## Prerequisites

### Hardware Required
- Raspberry Pi 4 Model B (2GB+ RAM)
- microSD card (16GB+, recommended 32GB)
- USB-Ethernet adapter or WiFi
- 5V 3A USB-C power supply
- Waveshare 3.5" LCD (C) display (optional)

### Host PC Requirements
- Ubuntu 22.04+ or similar Linux distribution
- Buildroot 2025.08 compiled with `raspberrypi4_64_defconfig`
- USB-Ethernet adapter for direct Pi connection

### Verify Buildroot Image
```bash
# Check image exists
ls -lh ~/buildroot/buildroot-2025.08/output/images/sdcard.img
# Expected: ~288MB file

# Check ONNX Runtime is present
ls -lh ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/external/onnxruntime-arm64/lib/
# Expected: libonnxruntime.so.1.16.3 (~16MB)

# Check LeafSense binary is compiled
ls -lh ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/build-arm64/LeafSense
# Expected: ~441KB ARM64 ELF binary
```

---

## Network Configuration

### Setup USB-Ethernet Adapter on Host PC
```bash
# Identify your USB-Ethernet adapter
ip link show
# Look for interface like enx00e04c3601a6

# Configure static IP on host PC
sudo ip link set enx00e04c3601a6 up
sudo ip addr flush dev enx00e04c3601a6
sudo ip addr add 10.42.0.1/24 dev enx00e04c3601a6

# Verify configuration
ip addr show enx00e04c3601a6
# Expected: inet 10.42.0.1/24
```

---

## SD Card Preparation

### Step 1: Identify SD Card
```bash
# Insert SD card and identify device
lsblk
# Look for new device (e.g., /dev/sda or /dev/mmcblk0)

# CRITICAL: Verify you have the correct device!
lsblk -o NAME,SIZE,MODEL /dev/sdX
```

### Step 2: Flash Buildroot Image
```bash
# Flash the image (replace /dev/sdX with your device)
sudo dd if=~/buildroot/buildroot-2025.08/output/images/sdcard.img \
       of=/dev/sdX \
       bs=4M \
       status=progress \
       conv=fsync

# Sync and wait
sync
echo "Flash complete!"
```

### Step 3: Set Partition Labels
```bash
# Refresh partition table
sudo partprobe /dev/sdX

# Set labels
sudo fatlabel /dev/sdX1 BOOT
sudo e2label /dev/sdX2 ROOTFS

# Verify
lsblk -o NAME,SIZE,LABEL,FSTYPE /dev/sdX
# Expected:
# sdX1   32M  BOOT   vfat
# sdX2  256M  ROOTFS ext4
```

### Step 4: Configure Static IP on Pi
```bash
# Mount ROOTFS partition
sudo mkdir -p /media/daniel/ROOTFS
sudo mount /dev/sdX2 /media/daniel/ROOTFS

# Edit network interfaces for static IP
sudo tee /media/daniel/ROOTFS/etc/network/interfaces << 'EOF'
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
    address 10.42.0.196
    netmask 255.255.255.0
    gateway 10.42.0.1
EOF

# Verify
cat /media/daniel/ROOTFS/etc/network/interfaces
```

### Step 5: Deploy Application Files
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project

# Create directory structure
sudo mkdir -p /media/daniel/ROOTFS/opt/leafsense/database

# Deploy ONNX Runtime library
sudo cp external/onnxruntime-arm64/lib/libonnxruntime.so.1.16.3 /media/daniel/ROOTFS/usr/lib/
sudo ln -sf libonnxruntime.so.1.16.3 /media/daniel/ROOTFS/usr/lib/libonnxruntime.so.1
sudo ln -sf libonnxruntime.so.1 /media/daniel/ROOTFS/usr/lib/libonnxruntime.so

# Deploy LeafSense binary
sudo cp build-arm64/LeafSense /media/daniel/ROOTFS/opt/leafsense/
sudo chmod +x /media/daniel/ROOTFS/opt/leafsense/LeafSense

# Deploy ML model and classes
sudo cp ml/leafsense_model.onnx /media/daniel/ROOTFS/opt/leafsense/
sudo cp ml/leafsense_model_classes.txt /media/daniel/ROOTFS/opt/leafsense/

# Deploy database schema
sudo cp database/schema.sql /media/daniel/ROOTFS/opt/leafsense/database/

# Verify deployment
ls -lh /media/daniel/ROOTFS/opt/leafsense/
ls -lh /media/daniel/ROOTFS/usr/lib/libonnxruntime*
```

### Step 6: Deploy Touchscreen Overlay (if using Waveshare 3.5")
```bash
# Mount BOOT partition
sudo mkdir -p /media/daniel/BOOT
sudo mount /dev/sdX1 /media/daniel/BOOT

# Copy Waveshare overlay
sudo cp deploy/waveshare35c.dtbo /media/daniel/BOOT/overlays/

# Append display configuration to config.txt
sudo tee -a /media/daniel/BOOT/config.txt << 'EOF'

# ========================================
# Waveshare 3.5" LCD (C) Configuration
# ========================================
dtoverlay=waveshare35c:rotate=90,speed=16000000,fps=50

# HDMI to LCD framebuffer settings
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=480 320 60 6 0 0 0
hdmi_drive=2

# Framebuffer size
framebuffer_width=480
framebuffer_height=320

# Disable Bluetooth for stability
dtoverlay=disable-bt
EOF

# Verify config
tail -20 /media/daniel/BOOT/config.txt
```

### Step 7: Unmount and Eject
```bash
# Unmount both partitions
sudo umount /media/daniel/ROOTFS
sudo umount /media/daniel/BOOT

# Sync
sync

# Safe to remove SD card
echo "SD card ready! Insert into Pi."
```

---

## First Boot

### Step 1: Connect Hardware
1. Insert SD card into Raspberry Pi 4
2. Connect Ethernet cable between Pi and USB-Ethernet adapter on host PC
3. (Optional) Connect Waveshare 3.5" display to GPIO header
4. Connect 5V 3A power supply

### Step 2: Monitor Boot on Host PC
```bash
# Configure host network interface
sudo ip link set enx00e04c3601a6 up
sudo ip addr flush dev enx00e04c3601a6
sudo ip addr add 10.42.0.1/24 dev enx00e04c3601a6

# Wait for Pi to boot and ping
for i in $(seq 1 60); do
    if ping -c 1 -W 1 10.42.0.196 >/dev/null 2>&1; then
        echo "✓ Pi online after ${i} seconds"
        break
    fi
    printf "."
    sleep 1
done
```

---

## SSH Connection

### Connect via SSH
```bash
# SSH into the Pi (password: leafsense)
ssh root@10.42.0.196

# If host key changed, clear it first
ssh-keygen -R 10.42.0.196
ssh root@10.42.0.196
```

### Verify System Status
```bash
# Check system info
ssh root@10.42.0.196 'uname -a && uptime'

# Check network
ssh root@10.42.0.196 'ip addr show eth0'

# Check memory
ssh root@10.42.0.196 'free -h'

# Check disk space
ssh root@10.42.0.196 'df -h'
```

---

## Touchscreen Configuration

### Check Display Initialization
```bash
ssh root@10.42.0.196 'dmesg | grep -E "fb|ILI|ads7846|waveshare" | head -15'
# Expected output:
# bcm2708_fb soc:fb: Registered framebuffer for display 0, size 480x320
# ads7846 spi0.1: touchscreen, irq 43
# fb_ili9486 spi0.0: fbtft_property_value: rotate = 90
# graphics fb1: fb_ili9486 frame buffer, 480x320
```

### Verify Framebuffer Devices
```bash
ssh root@10.42.0.196 'cat /proc/fb'
# Expected:
# 0 BCM2708 FB       <-- GPU framebuffer (HDMI)
# 1 fb_ili9486       <-- Waveshare display
```

### Verify Touch Input
```bash
ssh root@10.42.0.196 'ls -la /dev/input/event*'
# Expected: /dev/input/event0 (ADS7846 Touchscreen)
```

---

## Application Deployment

### Verify Files on Pi
```bash
ssh root@10.42.0.196 'ls -lh /opt/leafsense/'
# Expected:
# LeafSense              (441K)
# leafsense_model.onnx   (5.8M)
# leafsense_model_classes.txt
# database/

# Verify library dependencies
ssh root@10.42.0.196 'ldd /opt/leafsense/LeafSense | head -15'
# All libraries should show "=> /usr/lib/..."
```

### Create Startup Script
```bash
ssh root@10.42.0.196 'cat > /opt/leafsense/start_leafsense.sh << '\''EOF'\''
#!/bin/sh
# LeafSense Display Startup Script

cd /opt/leafsense
killall LeafSense 2>/dev/null || true
sleep 1

exec env \
    QT_QPA_PLATFORM=linuxfb \
    QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins \
    QT_QPA_FONTDIR=/usr/share/fonts \
    QT_QPA_MOUSEDRIVER=linuxinput \
    ./LeafSense -platform linuxfb:fb=/dev/fb1
EOF
chmod +x /opt/leafsense/start_leafsense.sh'
```

---

## Database Initialization

### Initialize SQLite Database
```bash
ssh root@10.42.0.196 'cd /opt/leafsense && sqlite3 leafsense.db < database/schema.sql'

# Verify tables created
ssh root@10.42.0.196 'cd /opt/leafsense && sqlite3 leafsense.db ".tables"'
# Expected output:
# alerts                      plant_images
# health_assessments          sensor_readings
# logs                        user
# ml_detections               vw_daily_sensor_summary
# ml_predictions              vw_latest_sensor_reading
# ml_recommendations          vw_pending_recommendations
# plant                       vw_unread_alerts
```

---

## Application Testing

### Test 1: Headless Mode (SSH only, no display)
```bash
ssh root@10.42.0.196 'cd /opt/leafsense && \
    nohup ./LeafSense -platform offscreen > /tmp/leafsense.log 2>&1 &'

sleep 5

# Check if running
ssh root@10.42.0.196 'ps aux | grep LeafSense | grep -v grep'

# Check logs
ssh root@10.42.0.196 'cat /tmp/leafsense.log | head -20'
# Expected:
# [Daemon] Database Service Started.
# [ML] Model loaded successfully: ./leafsense_model.onnx
```

### Test 2: Framebuffer Mode (GPU display - HDMI)
```bash
ssh root@10.42.0.196 'killall LeafSense 2>/dev/null; cd /opt/leafsense && \
    nohup ./LeafSense -platform linuxfb:fb=/dev/fb0 > /tmp/leafsense.log 2>&1 &'

sleep 4
ssh root@10.42.0.196 'ps aux | grep LeafSense | grep -v grep'
```

### Test 3: Waveshare 3.5" Touchscreen (fb1)
```bash
ssh root@10.42.0.196 'killall LeafSense 2>/dev/null; cd /opt/leafsense && \
    nohup env \
        QT_QPA_PLATFORM=linuxfb \
        QT_QPA_MOUSEDRIVER=linuxinput \
        ./LeafSense -platform linuxfb:fb=/dev/fb1 > /tmp/leafsense.log 2>&1 &'

sleep 4
ssh root@10.42.0.196 'ps aux | grep LeafSense | grep -v grep && echo "✓ Running on touchscreen"'
```

### Test 4: Using Startup Script
```bash
ssh root@10.42.0.196 'cd /opt/leafsense && ./start.sh &'

sleep 4
ssh root@10.42.0.196 'ps aux | grep LeafSense | grep -v grep'
```

### Verify Application Logs
```bash
# Check for successful startup
ssh root@10.42.0.196 'tail -50 /tmp/leafsense.log | grep -E "Daemon|ML|Error|Pump"'

# Expected successful output:
# [Daemon] Database Service Started.
# [ML] Model loaded successfully: ./leafsense_model.onnx
# [Daemon] Processed: SENSOR|23.3|6.86|1377
# [Pump 13] Dosing for 500ms
```

### Stop Application
```bash
ssh root@10.42.0.196 'killall LeafSense 2>/dev/null && echo "LeafSense stopped"'
```

---

## Quick Reference

### Essential Commands

| Task | Command |
|------|---------|
| SSH to Pi | `ssh root@10.42.0.196` (password: leafsense) |
| Start app (touchscreen) | `ssh root@10.42.0.196 'cd /opt/leafsense && ./start.sh &'` |
| Stop app | `ssh root@10.42.0.196 'killall LeafSense'` |
| Check running | `ssh root@10.42.0.196 'ps aux \| grep LeafSense'` |
| View logs | `ssh root@10.42.0.196 'tail -f /tmp/leafsense.log'` |
| Reboot Pi | `ssh root@10.42.0.196 'reboot'` |

### Network Configuration

| Device | IP Address |
|--------|------------|
| Host PC (USB-Ethernet) | 10.42.0.1/24 |
| Raspberry Pi 4 | 10.42.0.196/24 |

### File Locations on Pi

| File | Path |
|------|------|
| Application binary | `/opt/leafsense/LeafSense` |
| ML model | `/opt/leafsense/leafsense_model.onnx` |
| Class labels | `/opt/leafsense/leafsense_model_classes.txt` |
| Database | `/opt/leafsense/leafsense.db` |
| Startup script | `/opt/leafsense/start.sh` |
| Application logs | `/tmp/leafsense.log` |
| ONNX Runtime | `/usr/lib/libonnxruntime.so.1.16.3` |

### Framebuffer Devices

| Device | Description | Resolution |
|--------|-------------|------------|
| `/dev/fb0` | GPU/HDMI output | Varies |
| `/dev/fb1` | Waveshare ILI9486 | 480x320 |

### Environment Variables for Qt5

```bash
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins
export QT_QPA_FONTDIR=/usr/share/fonts
export QT_QPA_MOUSEDRIVER=linuxinput
```

---

## Troubleshooting

### Pi not responding to ping
```bash
# Verify host network interface is up
sudo ip link set enx00e04c3601a6 up
sudo ip addr add 10.42.0.1/24 dev enx00e04c3601a6

# Try ARP scan
sudo arp-scan --interface=enx00e04c3601a6 10.42.0.0/24
```

### Display is black
```bash
# Check which framebuffer to use
ssh root@10.42.0.196 'cat /proc/fb'

# For Waveshare 3.5", use fb1 not fb0
./LeafSense -platform linuxfb:fb=/dev/fb1
```

### Library not found
```bash
# Verify ONNX Runtime is deployed
ssh root@10.42.0.196 'ls -la /usr/lib/libonnxruntime*'

# Check all dependencies
ssh root@10.42.0.196 'ldd /opt/leafsense/LeafSense'
```

### Database errors
```bash
# Reinitialize database
ssh root@10.42.0.196 'cd /opt/leafsense && rm -f leafsense.db && sqlite3 leafsense.db < database/schema.sql'
```

### Touch not working
```bash
# Verify touchscreen device exists
ssh root@10.42.0.196 'cat /proc/bus/input/devices | grep -A 4 ADS7846'

# Check udev rules
ssh root@10.42.0.196 'cat /etc/udev/rules.d/99-touchscreen.rules'
```

---

## Formal Test Cases

This section contains the formal test cases defined for the LeafSense system validation. These test cases are designed to verify all functional requirements and ensure system reliability.

### System Setup & Basic Functionality Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCSSBF1 | System Setup | RPi Boots and Initializes | RPi boots; OS loads; Libraries load; GUI starts; DB accessible |
| TCSSBF2 | System Setup | Database Tables Created | All 10 tables exist; Correct fields; PKs defined; FKs defined; Indexes created |

### Sensor Reading Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCSR1 | Sensor Reading | Reading Collection | Reading in DB within 30 min; Temp/pH/EC recorded; Valid ranges; Timestamp recorded |
| TCSR2 | Sensor Reading | Multiple Readings Over Time | At least 6 readings in 180 min; Spaced 30 min apart; Valid data; Nothing missing |
| TCSR3 | Sensor Reading | Out-of-Range Alert | High temp reading stored; Alert generated; Alert type correct; Message mentions temp; is_read=0 |
| TCSR4 | Sensor Reading | GUI Displays Readings | GUI loads; Temp/pH/EC displayed; Values match latest DB entry |

### Image Capture Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCIC1 | Image Capture | Scheduled Image Capture | Image created at scheduled time; Metadata stored in DB; Filename correct; captured_at recorded |
| TCIC2 | Image Capture | 2 Images Per Day | 2 images captured on schedule; Each at correct time; Metadata for each recorded |

### ML Analysis & Predictions Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCMLAP1 | ML Analysis | ONNX Models Load | Models load without errors; No FileNotFoundError; GUI continues to run |
| TCMLAP2 | ML Analysis | Image Analysis Generates Predictions | Predictions in ML_PREDICTIONS table; At least 3 per image; Has: image_id, label, confidence, timestamp |
| TCMLAP3 | ML Analysis | Health Score Generated | Assessment in DB; health_score 0–100; health_status valid (Excellent/Healthy/Warning/Critical); assessment_date recorded |
| TCMLAP4 | ML Analysis | Recommendations Generated | Recommendations in DB; At least 1 per prediction; recommendation_text not empty; Valid type; confidence 0–1; user_acknowledged=0 |

### Disease Detection Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCDIS1 | Disease Detection | Model Loads | Disease detection ONNX model loads without errors; Model ready for inference |
| TCDIS2 | Disease Detection | Diseased Plant Detection | Image with plant disease analyzed; Disease detected with high confidence; Correct disease label returned |
| TCDIS3 | Disease Detection | Healthy Plant Detection | Image of healthy plant analyzed; Disease model returns healthy status with high confidence |
| TCDIS4 | Disease Detection | Bounding Box for Disease | Disease detected with bounding box; Box coordinates valid; Box covers affected area |
| TCDIS5 | Disease Detection | Confidence Score | Disease detection returns confidence value between 0.0–1.0; Confidence reflects detection certainty |
| TCDIS6 | Disease Detection | Recommendation Generated | After disease detected; Appropriate treatment recommendation generated automatically |
| TCDIS7 | Disease Detection | Multiple Diseases Detection | Image with multiple diseases analyzed; All diseases detected; Each with separate confidence score |
| TCDIS8 | Disease Detection | Alert Threshold | Disease detected above confidence threshold; Alert generated with appropriate severity level |
| TCDIS9 | Disease Detection | Logging | Disease detection logged with: disease name, confidence, timestamp, image reference |
| TCDIS10 | Disease Detection | False Positive Rate | Multiple healthy images analyzed; False positive rate acceptable; No false disease alerts |

### Deficiency Detection Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCDEF1 | Deficiency Detection | Model Loads | Deficiency detection ONNX model loads without errors; Model ready for inference |
| TCDEF2 | Deficiency Detection | Nutrient Deficiency Detection | Image with nutrient deficiency analyzed; Deficiency detected with high confidence; Correct nutrient label returned |
| TCDEF3 | Deficiency Detection | Healthy Plant Detection | Image of healthy plant analyzed; Deficiency model returns healthy status with high confidence |
| TCDEF4 | Deficiency Detection | Confidence Score | Deficiency detection returns confidence value between 0.0–1.0; Confidence reflects detection certainty |
| TCDEF5 | Deficiency Detection | Recommendation Generated | After deficiency detected; Nutrient adjustment recommendation generated automatically |
| TCDEF6 | Deficiency Detection | Specific Nutrient Recommendation | Recommendation specifies which nutrient to adjust; Includes adjustment guidance (dosage/percentage) |
| TCDEF7 | Deficiency Detection | Multiple Deficiencies Detection | Image with multiple deficiencies analyzed; All deficiencies detected; Each with separate score |
| TCDEF8 | Deficiency Detection | Alert Threshold | Deficiency detected above confidence threshold; Alert generated with appropriate severity level |
| TCDEF9 | Deficiency Detection | Logging | Deficiency detection logged with: nutrient type, confidence, timestamp, image reference |
| TCDEF10 | Deficiency Detection | Sensor Data Correlation | Deficiency detected correlated with sensor readings; Matches environmental conditions |
| TCDEF11 | Deficiency Detection | False Positive Rate | Multiple healthy images analyzed; False positive rate acceptable; No false deficiency alerts |
| TCDEF12 | Deficiency Detection | Nutrient Pump Trigger | After deficiency detected; Nutrient dosing pump activates automatically if EC below threshold |

### Actuator Control Test Cases - Water Heater

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCACT1 | Actuator Control | Water Heater Initialization | Water heater GPIO pin initialized; Ready to activate when temp out of range |
| TCACT2 | Actuator Control | Water Heater Activation on Low Temp | Sensor reads temp below min threshold (e.g., 18°C); Water heater activates automatically |
| TCACT3 | Actuator Control | Water Heater Deactivation on Target Temp | Water heater running; Sensor reads temp at target (e.g., 24°C); Heater deactivates automatically |
| TCACT4 | Actuator Control | Water Heater No Activation in Range | Sensor reads temp within ideal range; Water heater remains OFF; No activation without trigger |

### Actuator Control Test Cases - Nutrient Dosing Pump

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCACT5 | Actuator Control | Nutrient Pump Initialization | Nutrient pump GPIO pin initialized; Ready to activate when nutrient level out of range |
| TCACT6 | Actuator Control | Nutrient Pump Activation on Deficiency | Sensor/ML detects nutrient deficiency; Nutrient pump activates automatically; Doses for configured duration |
| TCACT7 | Actuator Control | Nutrient Pump Deactivation After Dose | Nutrient pump runs for configured duration; Auto-stops after specified time; System logs activation |
| TCACT8 | Actuator Control | Nutrient Pump No Activation in Range | System detects nutrients adequate; Nutrient pump remains OFF; No activation without trigger |

### Actuator Control Test Cases - pH Dosing Pump

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCACT9 | Actuator Control | pH Pump Initialization | pH pump GPIO pin initialized; Ready to activate when pH out of range |
| TCACT10 | Actuator Control | pH Pump Activation on Low pH | Sensor reads pH below min threshold (e.g., <6.5); pH pump (pH up) activates automatically |
| TCACT11 | Actuator Control | pH Pump Activation on High pH | Sensor reads pH above max threshold (e.g., >7.5); pH pump (pH down) activates automatically |
| TCACT12 | Actuator Control | pH Pump Deactivation on Target pH | pH pump running; Sensor reads pH at target (e.g., 7.0); Pump deactivates automatically |
| TCACT13 | Actuator Control | pH Pump No Activation in Range | Sensor reads pH within ideal range; pH pump remains OFF; No activation without trigger |

### Actuator Control Test Cases - Shared Features & Logging

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCACT14 | Actuator Control | Actuator Logs | All actuator activations logged (time, type, duration, reason); Logs accessible from UI; Useful for troubleshooting |
| TCACT15 | Actuator Control | Multiple Actuators Simultaneous | Multiple sensors out of range simultaneously; All relevant actuators activate together; Logged separately |

### GUI Display Test Cases - Dashboard & Interactions

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCGUID1 | GUI Display | User Login | Login page displays; Credentials accepted; Redirected to dashboard; Dashboard loads |
| TCGUID2 | GUI Display | User Logout | Logout button visible; Button clickable; User session terminated; Redirected to login page |
| TCGUID3 | GUI Display | Health Card Display | Health card visible; Score displays; Status displays; Details visible; Color matches status |
| TCGUID4 | GUI Display | Predictions Display | Predictions section visible; Each shows label and confidence; Multiple shown if detected |
| TCGUID5 | GUI Display | Recommendations Count | Badge shows new recommendations count; Updates after acknowledgment |
| TCGUID6 | GUI Display | Alerts Display | Alerts section visible; Unread count displayed; Recent alerts listed; Type shown |
| TCGUID7 | GUI Display | Auto-Refresh Data | Dashboard updates every 5–10 sec; New alerts appear automatically; New recommendations appear automatically |
| TCGUID8 | GUI Display | Acknowledge Recommendation | Button clickable; Marked as acknowledged in DB; No longer shows as New; Count decreases |
| TCGUID9 | GUI Display | Record Action Taken | Action form appears; Text saved to DB; action_date recorded; Dashboard shows recorded |
| TCGUID10 | GUI Display | Record Outcome | Outcome form appears; Saved to DB; outcome_date recorded; Dashboard shows recorded |
| TCGUID11 | GUI Display | Mark Alert as Read | is_read updated to 1; No longer shows unread; Unread count decreases |

### GUI Display Test Cases - Logs & Theme

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCGUID12 | GUI Display | Logs Page Load | Logs page accessible from GUI; Tab navigation visible (Alerts/Diseases/Deficiencies/Maintenance) |
| TCGUID13 | GUI Display | View Alerts in Logs | Alerts tab displays all alerts; Shows: type, message, timestamp; Ordered by date (newest first) |
| TCGUID14 | GUI Display | View Diseases in Logs | Diseases tab displays all disease logs; Shows: disease name, confidence, date; Filterable by date |
| TCGUID15 | GUI Display | View Deficiencies in Logs | Deficiencies tab displays all deficiency logs; Shows: nutrient type, confidence, date; Filterable by date |
| TCGUID16 | GUI Display | View Maintenance in Logs | Maintenance tab displays all maintenance logs; Shows: action taken, date, details; Filterable by date |
| TCGUID17 | GUI Display | Light Mode Display | GUI toggles to light mode; All text readable; Colors appropriate for light theme |
| TCGUID18 | GUI Display | Dark Mode Display | GUI toggles to dark mode; All text readable; Colors appropriate for dark theme; Eye-friendly |
| TCGUID19 | GUI Display | Theme Persistence | Selected theme saved in system; Theme applies on next login; User preference maintained |

### GUI Display Test Cases - Settings & Configuration

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCGUID20 | GUI Display | Settings Page Load | Settings page accessible from GUI; All configuration options display; Form elements functional |
| TCGUID21 | GUI Display | Change Temperature Threshold | Settings page allows input; Temperature min/max values editable; Changes saved to DB |
| TCGUID22 | GUI Display | Change pH Threshold | Settings page allows input; pH min/max values editable; Changes saved to DB |
| TCGUID23 | GUI Display | Change EC Threshold | Settings page allows input; EC min/max values editable; Changes saved to DB |
| TCGUID24 | GUI Display | Reset Thresholds to Default | Reset button available; Clicking resets all thresholds to factory defaults; Changes saved |
| TCGUID25 | GUI Display | Alert Generation with New Thresholds | After changing thresholds; System uses new values for alert generation; Old thresholds no longer used |
| TCGUID26 | GUI Display | Display Current Threshold Values | Settings page shows current threshold values; Values match what's saved in DB |

### Data Persistence Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCDP1 | Data Persistence | Survives System Restart | DB intact after reboot; All data visible; Same health score/alerts; No data loss |
| TCDP2 | Data Persistence | Image Files Saved | File at correct path; Valid image (can open); Not corrupted; Timestamp matches DB |

### Basic Integration Test Cases

| TC ID | Category | Test Name | Expected Result |
|-------|----------|-----------|-----------------|
| TCBI1 | Basic Integration | Half Daily Cycle (12 hours) | 72 sensor readings; 1 image captured on schedule; 1 assessment; 1 recommendation; User can interact; No errors |
| TCBI2 | Basic Integration | Problem Detection Response | ML detects problem; Health score reflects it; Alert generated; Relevant recommendations appear; User can act; Data flows |
| TCBI3 | Basic Integration | 24-Hour Continuous Operation | No crashes; GUI responsive; 144 readings; 2 images; 2 assessments; 2 recommendations; Consistent data; No errors |

---

*Document last updated: January 9, 2026*
