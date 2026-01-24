# LeafSense - Intelligent Hydroponic Monitoring System

**Version:** 1.6.1  
**Date:** January 23, 2026  
**Status:** ✅ Complete system - Display, camera, touchscreen, ML, and GPIO actuators working on Raspberry Pi 4  
**Platform:** Raspberry Pi 4B (ARM64) + Custom Buildroot Linux  
**Architecture:** Cross-compiled C++17 with Qt5, OpenCV, ONNX Runtime  
**Test Pass Rate:** 90% (73/81 test cases)

> 📋 **See [docs/00-PROJECT-STATUS.md](docs/00-PROJECT-STATUS.md) for complete implementation status**

---

## 🌱 Overview

LeafSense is a complete embedded system for intelligent hydroponic plant monitoring and control. It combines real-time sensor data acquisition, automatic control logic, machine learning-based disease detection, and a touchscreen interface—all running on a Raspberry Pi 4B with custom Buildroot Linux.

### Key Features

- **Real-Time Monitoring** - Temperature, pH, EC sensors (2-second update cycle)
- **Automatic Control** - pH adjustment, nutrient dosing, temperature regulation
- **ML Disease Detection** - ONNX Runtime with 99.39% accuracy (4 classes: Healthy, Nutrient Deficiency, Pest Damage, Disease)
- **Touchscreen UI** - Qt5-based interface on Waveshare 3.5" LCD (480×320)
- **Camera Integration** - Raspberry Pi Camera Module v1 with periodic image capture
- **Database Logging** - SQLite persistence for all sensor data, ML detections, and events
- **LED Alerts** - Custom kernel module for hardware alert indication
- **Cross-Platform** - Both PC (x86_64) and ARM64 builds supported

---

## 🏗️ System Architecture

**Hardware Stack:**
- Raspberry Pi 4B (ARM Cortex-A72, 2GB RAM)
- Waveshare 3.5" LCD-C (ILI9486 + ADS7846 touchscreen)
- DS18B20 temperature sensor (1-Wire)
- pH sensor (analog via ADC)
- EC/TDS sensor (analog via ADC)
- Raspberry Pi Camera Module v1 (OV5647, 5MP)
- Peristaltic pumps for dosing
- PWM heater for temperature control
- LED alert indicator

**Software Stack:**
- **OS:** Custom Buildroot Linux 6.12.41-v8 (minimal ~289 MB image)
- **GUI:** Qt5 5.15.14 (Widgets, Charts, SQL, linuxfb platform)
- **ML:** ONNX Runtime 1.16.3 with custom trained model
- **Vision:** OpenCV 4.11.0 (image preprocessing, CLAHE enhancement)
- **Database:** SQLite 3.48.0
- **Build:** CMake 3.22+ with cross-compilation toolchain

---

## 🚀 Quick Start

### Prerequisites

**Hardware:**
- Raspberry Pi 4B (2GB+ RAM recommended)
- Waveshare 3.5" LCD (C) with touchscreen
- MicroSD card (16GB+, camera-enabled Buildroot image)
- USB cable for SSH/deployment
- Raspberry Pi Camera Module (optional, test patterns used if unavailable)

**Development Machine:**
- Ubuntu 22.04+ or similar Linux distribution
- GCC/G++ 11.4+
- CMake 3.16+
- Buildroot toolchain (for ARM64 cross-compilation)

### Installation

#### Option 1: Use Pre-Built Binary (Quick)

```bash
# 1. Flash Buildroot image to SD card (see docs/05-BUILDROOT-IMAGE.md)
sudo dd if=sdcard.img of=/dev/sdX bs=4M status=progress && sync

# 2. Insert SD card into Pi and power on

# 3. Connect via USB-Ethernet (Pi will appear at 10.42.0.196)
ssh root@10.42.0.196
# Password: (none - passwordless root)

# 4. Deploy binary
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/

# 5. Start application
ssh root@10.42.0.196 '/opt/leafsense/LeafSense &'
```

#### Option 2: Build from Source

**PC Build (x86_64 - for development/testing):**
```bash
cd leafsense-project
rm -rf build && mkdir build && cd build
cmake ..
make -j$(nproc)
./src/LeafSense  # Run on PC
```

**ARM64 Build (for Raspberry Pi deployment):**
```bash
cd leafsense-project

# Setup ONNX Runtime for ARM64 (first time only)
./deploy/setup-onnxruntime-arm64.sh

# Configure cross-compilation build
rm -rf build-arm64 && mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..

# Compile (uses all CPU cores)
make -j$(nproc)

# Output: build-arm64/src/LeafSense (ARM64 ELF executable)
```

**Deploy to Raspberry Pi:**
```bash
# Copy binary
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/

# Start application with touchscreen support
ssh root@10.42.0.196 '/opt/leafsense/start_leafsense.sh &'

# Or manual execution with touchscreen:
ssh root@10.42.0.196 'export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1; \
  export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"; \
  cd /opt/leafsense && ./LeafSense &'

# Stop application
ssh root@10.42.0.196 'killall LeafSense'

# View logs
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log'
```

**Run on PC (Development):**
```bash
cd build/src
./LeafSense
```

---

## 📚 Documentation

**Start here:** [docs/00-PROJECT-STATUS.md](docs/00-PROJECT-STATUS.md) - Complete project status

Complete documentation available in [`docs/`](docs/) directory:

| # | Document | Description |
|---|----------|-------------|
| 00 | [00-PROJECT-STATUS.md](docs/00-PROJECT-STATUS.md) | **Complete project status and roadmap** |
| 01 | [01-OVERVIEW.md](docs/01-OVERVIEW.md) | Project overview and objectives |
| 02 | [02-ARCHITECTURE.md](docs/02-ARCHITECTURE.md) | System architecture and design patterns |
| 03 | [03-MACHINE-LEARNING.md](docs/03-MACHINE-LEARNING.md) | ML model training, inference, and dataset |
| 04 | [04-NETWORKING.md](docs/04-NETWORKING.md) | USB-Ethernet and SSH setup |
| 05 | [05-BUILDROOT-IMAGE.md](docs/05-BUILDROOT-IMAGE.md) | **Buildroot configuration and SD card creation** |
| 06 | [06-RASPBERRY-PI-DEPLOYMENT.md](docs/06-RASPBERRY-PI-DEPLOYMENT.md) | Deployment procedures and SSH access |
| 07 | [07-DEVICE-DRIVER.md](docs/07-DEVICE-DRIVER.md) | LED kernel module development |
| 08 | [08-DATABASE.md](docs/08-DATABASE.md) | SQLite schema and data persistence |
| 09 | [09-GUI.md](docs/09-GUI.md) | Qt5 interface and framebuffer rendering |
| 10 | [10-TROUBLESHOOTING.md](docs/10-TROUBLESHOOTING.md) | Common issues and solutions |
| 11 | [11-CHANGELOG.md](docs/11-CHANGELOG.md) | Version history and release notes |
| 12 | [12-IMPLEMENTATION-REPORT.md](docs/12-IMPLEMENTATION-REPORT.md) | Technical implementation report |
| 13 | [13-SENSOR-ACTUATOR-INTEGRATION.md](docs/13-SENSOR-ACTUATOR-INTEGRATION.md) | Hardware integration guide |
| 14 | [14-TESTING-GUIDE.md](docs/14-TESTING-GUIDE.md) | Testing procedures and validation |
| 15 | [15-DEMO-GUIDE.md](docs/15-DEMO-GUIDE.md) | Demonstration and presentation guide |
| 16 | [16-KERNEL-MODULE.md](docs/16-KERNEL-MODULE.md) | Kernel module development (low-level) |
| 17 | [17-TOUCHSCREEN-CONFIGURATION.md](docs/17-TOUCHSCREEN-CONFIGURATION.md) | **Touchscreen setup (evdev, rotate=90)** |
| 18 | [18-BUILDROOT-PACKAGES.md](docs/18-BUILDROOT-PACKAGES.md) | Complete Buildroot package list |
| 19 | [19-TERMINOLOGY.md](docs/19-TERMINOLOGY.md) | Key terms and concepts explained |
| 20 | [TEST-CASES-STATUS.md](docs/TEST-CASES-STATUS.md) | Section 4.6 test case analysis and pass/fail status |
| 21 | [VIDEO-RECORDING-SCRIPT.md](docs/VIDEO-RECORDING-SCRIPT.md) | Step-by-step demo video recording guide |
| 22 | [IMPLEMENTATION-VERIFICATION.md](docs/IMPLEMENTATION-VERIFICATION.md) | Report vs implementation cross-check |

**Quick Links:**
- 🎯 [Demonstration Guide](docs/15-DEMO-GUIDE.md) - Step-by-step demo procedures
- ⚙️ [Hardware Integration](docs/13-SENSOR-ACTUATOR-INTEGRATION.md) - Connect real sensors
- 📊 [Project Status](docs/00-PROJECT-STATUS.md) - Current feature status
- 🐛 [Troubleshooting](docs/10-TROUBLESHOOTING.md) - Common issues

---

## 🏗️ Architecture

### Hardware
- **Raspberry Pi 4B** - 2GB RAM, ARMv8 @ 1.5GHz
- **Display** - Waveshare 3.5" LCD (ILI9486, 480x320)
- **Touch** - ADS7846 resistive touchscreen
- **Camera** - OV5647 5MP (optional)
- **Sensors** - DS18B20 (temp), ADS1115 (pH/EC)
- **Actuators** - GPIO-controlled pumps and heater

### Software Stack
- **OS** - Buildroot 2025.08 (Linux 6.12.41-v8)
- **Language** - C++17
- **GUI** - Qt 5.15.14 (linuxfb platform)
- **ML** - ONNX Runtime 1.16.3
- **Vision** - OpenCV 4.11.0
- **Database** - SQLite 3.48.0

### Thread Architecture
```
Master Controller (7 threads)
├── tTime          - Heartbeat timer (2s)
├── tSig           - Signal dispatcher
├── tReadSensors   - Sensor polling & control logic
├── tWaterHeater   - Temperature control
├── tPHU           - pH Up pump
├── tPHD           - pH Down pump
└── tNutrients     - Nutrient dosing
```

---

## 🎯 Current Status

### ✅ Fully Operational
- Multi-threaded control system (7 threads)
- Qt5 GUI with touchscreen (rotate=90)
- ONNX ML inference (99.39% accuracy)
- Camera capture with fallback (640x480 JPEG)
- Image gallery with ML predictions
- LED alert system (kernel module)
- Database logging (13 tables)
- Comprehensive documentation (13 guides)

### 🔄 Ready for Integration
- Real sensor hardware (mock mode working, guide available)
- pH/EC calibration (procedures documented)
- Real camera capture (test pattern fallback working)

---

## 📸 Screenshots

### Dashboard
Real-time sensor monitoring with automatic control status.

### Analytics
Historical data charts and image gallery with ML predictions.

### Gallery
Captured plant images with disease detection results.

---

## 🧪 Testing

### Manual Test
```bash
# Connect to Pi
ssh root@10.42.0.196

# Check system status
ps aux | grep LeafSense
# Expected: LeafSense process running

# View sensor readings
tail -f /var/log/leafsense.log | grep SENSOR
# Expected: SENSOR|23.4|6.78|1267 every 2 seconds

# Check gallery
ls -lh /opt/leafsense/gallery/
# Expected: plant_*.jpg files with ML analysis

# Test database
sqlite3 /opt/leafsense/database/leafsense.db ".tables"
# Expected: 13 tables listed
```

### Automated Tests
```bash
# Run from build directory
cd build-arm64
make test
```

---

## 🔧 Configuration

### Environment Variables
```bash
# Qt5 Platform (framebuffer)
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins
export QT_QPA_FONTDIR=/usr/share/fonts

# Touchscreen calibration (critical!)
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
```

### Camera Intervals
Edit `src/middleware/Master.cpp`:
```cpp
if (cameraCaptureCounter >= 900) {  // 900 * 2s = 30 minutes
    // Adjust 900 to change capture frequency
}
```

### Ideal Ranges
Edit `src/middleware/IdealConditions.cpp`:
```cpp
// Default values for Lettuce
tds_min = 560; tds_max = 840;  // ppm
ph_min = 5.5; ph_max = 6.5;
temp_min = 18; temp_max = 24;  // °C
```

---

## 🐛 Troubleshooting

### LeafSense won't start
```bash
# Check log for errors
ssh root@10.42.0.196 'cat /var/log/leafsense.log | grep ERROR'

# Verify dependencies
ssh root@10.42.0.196 'ldd /opt/leafsense/LeafSense | grep "not found"'
```

### Touchscreen not responding
```bash
# Verify device exists
ssh root@10.42.0.196 'ls -l /dev/input/event0'

# Test touch events
ssh root@10.42.0.196 'hexdump -C /dev/input/event0' &
# Touch screen - should see data
```

### Database locked
```bash
# Kill all instances
ssh root@10.42.0.196 'killall -9 LeafSense'

# Remove lock files
ssh root@10.42.0.196 'rm -f /opt/leafsense/database/*.db-wal'

# Restart
ssh root@10.42.0.196 '/opt/leafsense/start_leafsense.sh &'
```

See [docs/10-TROUBLESHOOTING.md](docs/10-TROUBLESHOOTING.md) for complete guide.

---

## 📦 Project Structure

```
leafsense-project/
├── CMakeLists.txt                 # Build configuration
├── README.md                      # This file
├── build-arm64/                   # ARM64 build directory
├── database/
│   └── schema.sql                # Database schema
├── deploy/                        # Deployment scripts
│   ├── toolchain-rpi4.cmake
│   └── setup-waveshare35c.sh
├── docs/                          # Documentation (21 files)
│   ├── 00-PROJECT-STATUS.md      # Project status and roadmap
│   ├── 07-DEVICE-DRIVER.md       # Device driver guide
│   └── 16-KERNEL-MODULE.md       # Low-level kernel details
├── drivers/
│   └── kernel_module/            # LED driver (device driver)
├── external/
│   └── onnxruntime-arm64/        # Pre-built ONNX Runtime
├── include/                       # Header files
│   ├── application/
│   ├── drivers/
│   └── middleware/
├── ml/                            # Machine learning
│   ├── leafsense_model.onnx      # Trained model (5.9MB)
│   └── train_model.py
├── resources/                     # Qt resources
│   └── images/
└── src/                           # Source code
    ├── main.cpp
    ├── application/
    ├── drivers/
    └── middleware/
```

---

## 👥 Authors

**Group 11**

- **Daniel Gonçalo Silva Cardoso** - PG53753
- **Marco Xavier Leite Costa** - PG60210

**Advisor:** Professor Adriano José Conceição Tavares

**Course:** Embedded Systems and Computers  
**Program:** Master's in Industrial Electronics and Computers Engineering  
**Institution:** University of Minho

---

## 📄 License

This project was developed as part of an academic course. All rights reserved.

---

## 🙏 Acknowledgments

- Professor Adriano Tavares for guidance and support
- Qt Project for the GUI framework
- ONNX Runtime team for ML inference engine
- OpenCV community for computer vision tools
- Buildroot project for embedded Linux system

---

## 📞 Support

For issues, questions, or contributions:

1. Check [docs/00-PROJECT-STATUS.md](docs/00-PROJECT-STATUS.md) for system status
2. Review [docs/10-TROUBLESHOOTING.md](docs/10-TROUBLESHOOTING.md)
3. Contact development team (see Authors section)

---

**Last Updated:** January 19, 2026  
**Build Status:** ✅ Passing  
**System Status:** ✅ Production Ready

---

## 📋 Version 1.5.1 Highlights

### Computer Vision Enhancements
- Auto white balance correction
- CLAHE contrast enhancement
- Unsharp masking for improved sharpness
- Gaussian blur for noise reduction

### Database Improvements
- Enhanced logging with SUCCESS/FAILED status
- SQL command output on errors
- Real-time verification of data persistence

### Documentation
- Complete device driver guide ([docs/07-DEVICE-DRIVER.md](docs/07-DEVICE-DRIVER.md))
- 21 comprehensive documentation files
- Full API reference and integration examples
