# LeafSense Raspberry Pi Deployment Guide

## Authors
- Daniel Cardoso
- Marco Costa

## Date
December 2025

---

## Overview

This guide documents the deployment of LeafSense on Raspberry Pi 4 with Waveshare 3.5" LCD (C) touchscreen display.

### Hardware Configuration
- **Board**: Raspberry Pi 4 (4GB/8GB)
- **Display**: Waveshare 3.5" LCD (C) - ILI9486 controller
- **Touch**: ADS7846 resistive touchscreen
- **Resolution**: 480x320 pixels
- **Interface**: SPI (display on CS0, touch on CS1)

---

## Build System

### Buildroot Configuration
The system uses Buildroot 2025.08 with custom configuration for:
- Qt5 with linuxfb platform plugin
- tslib for touchscreen calibration
- SQLite3 for database
- OpenCV for image processing (optional)

### Cross-Compilation
```bash
# Build for ARM64
cd leafsense-project
mkdir build-arm64 && cd build-arm64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-aarch64.cmake
make -j$(nproc)
```

---

## Display Configuration

### Kernel Overlay
The Waveshare 3.5" LCD (C) requires the `waveshare35c.dtbo` overlay from:
- https://files.waveshare.com/wiki/common/Waveshare35c.zip

### config.txt Settings
```ini
# Enable SPI
dtparam=spi=on

# Waveshare 3.5" LCD (C) overlay
# SPI speed reduced to 48MHz for touch stability (default 115MHz causes freeze)
# FPS reduced to 20 for same reason (default 31)
dtoverlay=waveshare35c,speed=48000000,fps=20

# HDMI settings for framebuffer
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=480 320 60 6 0 0 0
hdmi_drive=2
```

### Critical Fix: SPI Speed
**Problem**: At default 115MHz SPI speed, touching the screen causes UI freeze due to SPI bus contention between display and touch controller.

**Solution**: Reduce SPI speed to 48MHz and FPS to 20:
```
dtoverlay=waveshare35c,speed=48000000,fps=20
```

---

## Touch Configuration

### tslib Setup
Touch calibration uses tslib with the following configuration:

**/etc/ts.conf**:
```
module_raw input
module pthres pmin=1
module dejitter delta=100
module linear
```

**/etc/pointercal** (90° rotation calibration):
```
0 -7680 31457280 5120 0 0 65536
```

### ts_uinput
The `ts_uinput` daemon creates a calibrated virtual input device:
- Raw input: `/dev/input/event0` (ADS7846, 0-4095 range)
- Calibrated output: `/dev/input/event1` (ts_uinput, 0-479/0-319 range)

### Environment Variables
```bash
# TSLIB configuration
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_PLUGINDIR=/usr/lib/ts
```

---

## Qt5 Configuration

### Platform Settings
```bash
# Use linuxfb platform on fb1 (LCD)
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1

# Use calibrated touch from ts_uinput
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=/dev/input/event1:rotate=0
export QT_QPA_FONTDIR=/usr/share/fonts
```

### Touch Scrolling
QScroller is enabled for touch-based kinetic scrolling in scroll areas:
```cpp
QScroller::grabGesture(scroll_area->viewport(), QScroller::TouchGesture);
```

### Focus Rectangle Removal
Focus rectangles are disabled for touchscreen use via stylesheet:
```css
*:focus {
    outline: none;
}
```

---

## Application Startup

### Init Script: /etc/init.d/S99leafsense
```bash
#!/bin/sh
case "$1" in
    start)
        echo "Starting LeafSense..."
        
        # Source environment
        . /etc/profile.d/qt-touchscreen.sh
        
        # Start ts_uinput for calibrated touch
        ts_uinput -d &
        sleep 2
        
        # Start application
        cd /opt/leafsense
        ./LeafSense >> /var/log/leafsense.log 2>&1 &
        ;;
    stop)
        echo "Stopping LeafSense..."
        killall LeafSense ts_uinput 2>/dev/null
        ;;
    restart)
        $0 stop
        sleep 2
        $0 start
        ;;
esac
```

---

## Deployment Steps

### 1. Flash SD Card
```bash
# Flash Buildroot image
sudo dd if=sdcard.img of=/dev/sdX bs=4M status=progress
sync
```

### 2. Expand Root Filesystem
After first boot:
```bash
# Resize partition (use fdisk or parted)
fdisk /dev/mmcblk0
# Delete partition 2, recreate with full size
# Reboot, then:
resize2fs /dev/mmcblk0p2
```

### 3. Install Overlay
```bash
# Mount boot partition
mount /dev/mmcblk0p1 /mnt/boot

# Copy overlay
cp waveshare35c.dtbo /mnt/boot/overlays/

# Update config.txt with display settings
```

### 4. Calibrate Touch
```bash
# Set environment
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_PLUGINDIR=/usr/lib/ts

# Run calibration
ts_calibrate

# Test calibration
ts_test
```

### 5. Deploy Application
```bash
# Copy binary
scp LeafSense root@<PI_IP>:/opt/leafsense/

# Copy resources
scp -r resources/* root@<PI_IP>:/opt/leafsense/

# Start application
ssh root@<PI_IP> '/etc/init.d/S99leafsense start'
```

---

## Troubleshooting

### Display Not Working
```bash
# Check framebuffers
ls /dev/fb*
# Should show fb0 (HDMI) and fb1 (LCD)

# Check kernel messages
dmesg | grep -i "fb\|ili9486\|spi"

# Verify overlay loaded
cat /proc/device-tree/soc/spi@*/status
```

### Touch Not Responding
```bash
# Check input devices
cat /proc/bus/input/devices

# Test raw touch
hexdump -C /dev/input/event0  # Touch screen

# Check ts_uinput running
ps aux | grep ts_uinput

# Test calibrated touch
ts_print_raw
```

### UI Freezes on Touch
- **Cause**: SPI bus contention at high speeds
- **Solution**: Reduce SPI speed in config.txt:
  ```
  dtoverlay=waveshare35c,speed=48000000,fps=20
  ```

### Touch Offset/Miscalibrated
```bash
# Recalibrate
ts_calibrate

# Verify calibration file
cat /etc/pointercal
```

---

## File Locations

| File | Location | Description |
|------|----------|-------------|
| LeafSense | /opt/leafsense/LeafSense | Main application |
| Log | /var/log/leafsense.log | Application log |
| Init Script | /etc/init.d/S99leafsense | Startup script |
| Qt Env | /etc/profile.d/qt-touchscreen.sh | Qt environment |
| ts.conf | /etc/ts.conf | tslib config |
| pointercal | /etc/pointercal | Touch calibration |
| config.txt | /boot/config.txt | Pi boot config |
| Overlay | /boot/overlays/waveshare35c.dtbo | Display overlay |

---

## Network Configuration

### USB Ethernet Gadget
The Pi is configured for USB ethernet gadget mode for development:
- Pi IP: 10.42.0.197 (may vary)
- Host IP: 10.42.0.1
- SSH: `ssh root@10.42.0.197` (password: root)

---

## Version History

| Date | Change |
|------|--------|
| Dec 2025 | Initial deployment with Waveshare 3.5" LCD (C) |
| Dec 2025 | Fixed SPI speed for touch stability (115→48MHz) |
| Dec 2025 | Added tslib calibration and ts_uinput |
| Dec 2025 | Enabled touch scrolling with QScroller |
| Dec 2025 | Removed focus rectangles for touchscreen |
