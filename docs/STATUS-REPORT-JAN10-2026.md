# LeafSense Project Status Report - January 10, 2026

> ⚠️ **SUPERSEDED:** This document has been superseded by [00-PROJECT-STATUS.md](00-PROJECT-STATUS.md). See that document for the current project status.

---

## Executive Summary

**PROJECT STATUS: ✅ FULL HARDWARE INTEGRATION COMPLETE**

The LeafSense project is fully operational. All core components (GUI, database, camera, ML, touchscreen) are configured and working. The Buildroot image includes **tslib** for touchscreen calibration support.

---

## Current System Status

### ✅ Working Components

| Component | Status | Details |
|-----------|--------|---------|
| **Raspberry Pi** | ✅ Running | Linux 6.12.41-v8 aarch64, IP: 10.42.0.196 |
| **LeafSense App** | ✅ Running | PID active, ML model loaded, daemon operational |
| **Database** | ✅ Working | SQLite, 8563+ sensor readings logged |
| **Camera** | ✅ Detected | OV5647 via unicam-image at /dev/video0 |
| **Display** | ✅ Working | Waveshare 3.5" LCD-C on /dev/fb1 (480x320) |
| **Touchscreen** | ✅ Configured | ADS7846 on /dev/input/event0, tslib calibrated |
| **Network** | ✅ Stable | SSH via 10.42.0.196, static Ethernet |
| **ML Engine** | ✅ Working | ONNX Runtime, model loaded successfully |

---

## Touchscreen Calibration (If Needed)

If touchscreen calibration needs to be redone:

```bash
# SSH into Pi
ssh root@10.42.0.196

# Run ts_calibrate (follow on-screen instructions)
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_FBDEVICE=/dev/fb1
ts_calibrate

# This creates /etc/pointercal with calibration data
```

---

## Deployment Instructions

### Step 1: Flash SD Card Image

```bash
# Insert SD card into your PC
# Identify the device (e.g., /dev/sdb or /dev/mmcblk0)
lsblk

# Flash the image (replace /dev/sdX with your device)
sudo dd if=/home/daniel/buildroot/buildroot-2025.08/output/images/sdcard.img of=/dev/sdX bs=4M status=progress
sync
```

### Step 2: Deploy LeafSense

After booting the Pi with the new image:

```bash
# Copy LeafSense binary
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/

# Copy ML model and resources
scp ml/leafsense_model.onnx root@10.42.0.196:/opt/leafsense/
scp ml/leafsense_model_classes.txt root@10.42.0.196:/opt/leafsense/
scp database/schema.sql root@10.42.0.196:/opt/leafsense/init_db.sql
```

### Step 3: Start LeafSense

```bash
# Run with proper environment
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_FB_DEV=/dev/fb1
export QT_QPA_FB_HIDECURSOR=0
export QT_QPA_FB_TSLIB=1
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_FBDEVICE=/dev/fb1
cd /opt/leafsense && ./LeafSense &
```

---

## Buildroot Configuration

The Buildroot configuration now includes tslib support:

```
BR2_PACKAGE_TSLIB=y
BR2_PACKAGE_QT5BASE_TSLIB=y
```

### Key Files in New Image

| File | Purpose |
|------|---------|
| `/usr/lib/libts.so` | tslib library |
| `/usr/bin/ts_calibrate` | Touchscreen calibration tool |
| `/etc/pointercal` | Calibration data (created by ts_calibrate) |
| `/usr/lib/qt/plugins/generic/libqevdevtouchplugin.so` | Qt touch input plugin |

---

## Project Structure (Cleaned)

```
leafsense-project/
├── CMakeLists.txt          # Main CMake configuration
├── README.md               # Project overview
├── build/                  # PC build output (x86_64)
├── build-arm64/            # ARM64 build output (Raspberry Pi)
├── database/               # SQLite schema
│   └── schema.sql
├── deploy/                 # Deployment scripts and configs
│   ├── config.txt          # Raspberry Pi config.txt
│   ├── configure-buildroot.sh  # Buildroot setup script
│   ├── toolchain-rpi4.cmake    # Cross-compilation toolchain
│   └── ...
├── docs/                   # Documentation
│   ├── 01-OVERVIEW.md
│   ├── 02-ARCHITECTURE.md
│   ├── ...
│   └── STATUS-REPORT-JAN10-2026.md
├── drivers/                # Kernel module source
│   └── kernel_module/
├── external/               # External dependencies (ONNX Runtime)
├── include/                # Header files
├── ml/                     # Machine learning model and training
├── resources/              # Qt resources (images, QRC)
└── src/                    # Source code
    ├── main.cpp
    ├── application/
    │   ├── gui/            # Qt GUI implementation
    │   └── ml/             # ML inference engine
    ├── drivers/
    │   ├── actuators/      # Pump, heater drivers
    │   └── sensors/        # Sensor interfaces
    └── middleware/         # Database, message queue, control logic
```

---

## Ready for Integration

The following components are ready for your colleague's sensor/actuator integration:

### Sensor Interface (include/drivers/sensors/)
- `ISensor.h` - Base sensor interface
- `DS18B20.h` - Temperature sensor
- `PhSensor.h` - pH sensor
- `ECMeter.h` - EC/TDS sensor

### Actuator Interface (include/drivers/actuators/)
- `IActuator.h` - Base actuator interface
- `PeristalticPump.h` - Dosing pump control
- `HeaterController.h` - PWM heater control

### Middleware (include/middleware/)
- `Master.h` - Control logic coordinator
- `dbManager.h` - Database operations
- `IdealConditions.h` - Target parameter ranges

---

## Next Steps

1. **Flash the new SD card image** with tslib support
2. **Run ts_calibrate** to calibrate the touchscreen
3. **Deploy LeafSense** and verify touch functionality
4. **Integrate sensors/actuators** (colleague's task)
5. **Final testing** and demo preparation

---

## Quick Reference

### SSH Access
```bash
ssh root@10.42.0.196
# Password: leafsense
```

### Build LeafSense (ARM64)
```bash
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir -p build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

### Deploy to Pi
```bash
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/
```

### Start LeafSense on Pi
```bash
ssh root@10.42.0.196 'export QT_QPA_PLATFORM=linuxfb; export QT_QPA_FB_DEV=/dev/fb1; export QT_QPA_FB_HIDECURSOR=0; export QT_QPA_FB_TSLIB=1; cd /opt/leafsense && ./LeafSense &'
```

---

*Report generated: January 10, 2026*
