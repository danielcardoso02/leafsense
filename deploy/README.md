# LeafSense Raspberry Pi Deployment Guide

## Authors
- Daniel Cardoso
- Marco Costa

## Date
January 2026

---

## Overview

This guide documents the deployment of LeafSense on Raspberry Pi 4 with Waveshare 3.5" LCD (C) touchscreen display.

### Hardware Configuration
- **Board**: Raspberry Pi 4 Model B (2GB+ RAM)
- **Display**: Waveshare 3.5" LCD (C) - ILI9486 controller
- **Touch**: ADS7846 resistive touchscreen
- **Resolution**: 480x320 pixels (landscape)
- **Interface**: SPI (display on CS0, touch on CS1)

---

## Quick Start

### 1. Flash SD Card
```bash
sudo dd if=~/buildroot/buildroot-2025.08/output/images/sdcard.img \
       of=/dev/sdX bs=4M status=progress conv=fsync
sudo fatlabel /dev/sdX1 BOOT
sudo e2label /dev/sdX2 ROOTFS
```

### 2. Configure Display Overlay
```bash
sudo mount /dev/sdX1 /mnt/BOOT
# NOTE: piscreen overlay is built into the kernel, no dtbo file copy needed
sudo cp deploy/boot-overlay/config.txt /mnt/BOOT/config.txt
sudo umount /mnt/BOOT
```

**Note:** The config.txt uses the `piscreen` overlay with:
- `speed=16000000` (16MHz) - prevents touch freeze
- `rotate=270` - landscape orientation
- Qt uses `rotate=180:invertx` for touch mapping

### 3. Deploy Application
```bash
sudo mount /dev/sdX2 /mnt/ROOTFS
sudo mkdir -p /mnt/ROOTFS/opt/leafsense

# Copy files
sudo cp build-arm64/src/LeafSense /mnt/ROOTFS/opt/leafsense/
sudo cp ml/leafsense_model.onnx /mnt/ROOTFS/opt/leafsense/
sudo cp ml/leafsense_model_classes.txt /mnt/ROOTFS/opt/leafsense/
sudo cp database/schema.sql /mnt/ROOTFS/opt/leafsense/
sudo cp external/onnxruntime-arm64/lib/libonnxruntime.so.1.16.3 /mnt/ROOTFS/usr/lib/
sudo ln -sf libonnxruntime.so.1.16.3 /mnt/ROOTFS/usr/lib/libonnxruntime.so

sudo umount /mnt/ROOTFS
```

### 4. First Boot
```bash
ssh root@10.42.0.196  # Password: leafsense

# Initialize database
cd /opt/leafsense
sqlite3 leafsense.db < schema.sql

# Run application (or use start.sh)
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
./LeafSense
```

---

## Build System

### PC Build (Development/Testing)
```bash
cd leafsense-project
mkdir -p build && cd build
cmake ..
make -j$(nproc)
# Binary: build/src/LeafSense
```

### ARM64 Build (Raspberry Pi)
```bash
cd leafsense-project
mkdir -p build-arm64 && cd build-arm64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake
make -j$(nproc)
# Binary: build-arm64/src/LeafSense
```

---

## Display Configuration

### Kernel Overlay
The 3.5" LCD uses the built-in `piscreen` overlay (ILI9486 + ADS7846).

### config.txt Settings (Working Configuration)
```ini
# Enable SPI
dtparam=spi=on

# 3.5" LCD with piscreen overlay - landscape mode
# CRITICAL: speed=16000000 prevents touch freeze
dtoverlay=piscreen,speed=16000000,rotate=270

# Framebuffer dimensions
framebuffer_width=480
framebuffer_height=320
```

**Note:** Higher SPI speeds (24MHz+) cause touchscreen freezing issues.

---

## Touch Configuration

### REQUIRED: Qt evdev (Do NOT use tslib!)

The **only working solution** uses Qt's evdev touchscreen handler with rotation parameter:

```bash
# Environment variable to set before running LeafSense
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320

# Run application
./LeafSense
```

**Key insight:** The `rotate=180:invertx` in `QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS` corrects touch mapping for the `piscreen,rotate=270` display configuration.

### ❌ DO NOT USE: tslib (Causes application freezing)

tslib causes application freezing when the touchscreen is touched due to SPI bus contention.

**Never use tslib with this display.** If you see TSLIB_ environment variables, remove them and use evdev instead.

---

## Qt5 Configuration

### Platform Settings
```bash
# Core Qt settings for LeafSense on 3.5" LCD
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FB_NO_LIBINPUT=1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
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

### Startup Script: /opt/leafsense/start.sh
```bash
#!/bin/sh
# LeafSense Startup Script - 3.5" LCD with piscreen overlay

# Qt Platform Configuration
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FB_NO_LIBINPUT=1
export QT_QPA_FONTDIR=/usr/share/fonts

# Touchscreen Configuration (CRITICAL: rotate=180:invertx for piscreen,rotate=270)
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"

# Run application
cd /opt/leafsense
./LeafSense
```

### Init Script: /etc/init.d/S99leafsense
```bash
#!/bin/sh
PIDFILE=/var/run/leafsense.pid
LOGFILE=/var/log/leafsense.log

case "$1" in
    start)
        echo "Starting LeafSense..."
        /opt/leafsense/start_leafsense.sh >> $LOGFILE 2>&1 &
        echo $! > $PIDFILE
        ;;
    stop)
        echo "Stopping LeafSense..."
        [ -f $PIDFILE ] && kill $(cat $PIDFILE) 2>/dev/null
        killall LeafSense 2>/dev/null
        rm -f $PIDFILE
        ;;
    restart)
        $0 stop
        sleep 2
        $0 start
        ;;
    status)
        if [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE) 2>/dev/null; then
            echo "LeafSense is running (PID: $(cat $PIDFILE))"
        else
            echo "LeafSense is not running"
        fi
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
# Check framebuffers (should show fb0 for HDMI, fb1 for LCD)
ls /dev/fb*

# Check kernel messages
dmesg | grep -i "fb\|ili9486\|spi"

# Verify overlay loaded
cat /proc/device-tree/soc/spi@*/status

# Test display
cat /dev/urandom > /dev/fb1  # Should show static
```

### Touch Not Responding
```bash
# Check input devices
cat /proc/bus/input/devices

# Test raw touch (should show hex output on touch)
hexdump -C /dev/input/event0

# Verify device permissions
ls -la /dev/input/event0
```

### Touch Coordinates Inverted/Wrong
This is the most common issue. The solution is to add `rotate=90` to the evdev parameters:

```bash
# Correct configuration - rotate=90 for proper touch mapping
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
```

| Display Rotation | Touch Parameter |
|------------------|------------------|
| rotate=0 | rotate=0 |
| rotate=90 | rotate=90 |
| rotate=180 | rotate=90 |
| rotate=270 | rotate=270 |

### UI Freezes on Touch (tslib issue)
- **Cause**: tslib can cause SPI bus contention
- **Solution**: Use evdev instead of tslib:
  ```bash
  export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
  ```

---

## File Locations

| File | Location | Description |
|------|----------|-------------|
| LeafSense | /opt/leafsense/LeafSense | Main application |
| Database | /opt/leafsense/leafsense.db | SQLite database |
| ML Model | /opt/leafsense/leafsense_model.onnx | ONNX model |
| Startup | /opt/leafsense/start_leafsense.sh | Startup script |
| Gallery | /opt/leafsense/gallery/ | Camera captures |
| Log | /var/log/leafsense.log | Application log |
| Init | /etc/init.d/S99leafsense | Auto-start script |
| ONNX | /usr/lib/libonnxruntime.so.1.16.3 | ONNX Runtime |
| config.txt | /boot/config.txt | Pi boot config |
| Overlay | /boot/overlays/waveshare35c.dtbo | Display overlay |

---

## Network Configuration

### USB Ethernet Gadget
The Pi is configured for USB ethernet gadget mode for development:
- **Pi IP**: 10.42.0.196
- **Host IP**: 10.42.0.1
- **SSH**: `ssh root@10.42.0.196`
- **Password**: leafsense

---

## Version History

| Date | Change |
|------|--------|
| Jan 2026 | Updated to evdev touchscreen handling (more reliable than tslib) |
| Jan 2026 | Added rotate=90 parameter for correct touch coordinates |
| Jan 2026 | Improved startup scripts and documentation |
| Dec 2025 | Initial deployment with Waveshare 3.5" LCD (C) |
| Dec 2025 | Fixed SPI speed for touch stability |
| Dec 2025 | Enabled touch scrolling with QScroller |
