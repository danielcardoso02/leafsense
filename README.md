# LeafSense - Intelligent Hydroponic Monitoring System

**Version:** 1.4.0  
**Status:** ✅ Production Ready  
**Platform:** Raspberry Pi 4B (ARM64)

---

## 🌱 Overview

LeafSense is a complete embedded system for intelligent hydroponic plant monitoring and control. It combines real-time sensor data acquisition, automatic control logic, machine learning-based disease detection, and a touchscreen interface—all running on a Raspberry Pi 4B.

### Key Features

- **Real-Time Monitoring** - Temperature, pH, EC sensors (2-second update cycle)
- **Automatic Control** - pH adjustment, nutrient dosing, temperature regulation
- **ML Disease Detection** - ONNX Runtime with 99.39% accuracy (4 classes)
- **Touchscreen UI** - Qt5-based interface on Waveshare 3.5" LCD
- **Camera Integration** - Periodic image capture with ML analysis
- **Database Logging** - SQLite persistence for all sensor data and events
- **LED Alerts** - Kernel module-based alert indication

---

## 🚀 Quick Start

### Prerequisites
- Raspberry Pi 4B (2GB+ RAM)
- Waveshare 3.5" LCD (C) with touchscreen
- Raspberry Pi Camera Module (optional)
- Development machine with ARM64 cross-compiler

### Deployment

```bash
# 1. Connect to Raspberry Pi
ssh root@10.42.0.196

# 2. Check if LeafSense is running
ps aux | grep LeafSense

# 3. View live logs
tail -f /var/log/leafsense.log

# 4. Access touchscreen interface
# Touch the 480x320 display - login with admin/admin
```

### From Source

```bash
# Build for ARM64
cd build-arm64
cmake ..
make -j4

# Deploy to Pi
scp LeafSense root@10.42.0.196:/opt/leafsense/

# Initialize database
ssh root@10.42.0.196 'cd /opt/leafsense/database && sqlite3 leafsense.db < schema.sql'

# Start application
ssh root@10.42.0.196 '/opt/leafsense/start_leafsense.sh &'
```

---

## 📚 Documentation

Complete documentation available in [`docs/`](docs/) directory:

| Document | Description |
|----------|-------------|
| [01-OVERVIEW.md](docs/01-OVERVIEW.md) | Project overview and objectives |
| [02-ARCHITECTURE.md](docs/02-ARCHITECTURE.md) | System architecture and design |
| [03-MACHINE-LEARNING.md](docs/03-MACHINE-LEARNING.md) | ML model details and training |
| [04-RASPBERRY-PI-DEPLOYMENT.md](docs/04-RASPBERRY-PI-DEPLOYMENT.md) | Deployment procedures |
| [11-SENSOR-ACTUATOR-INTEGRATION.md](docs/11-SENSOR-ACTUATOR-INTEGRATION.md) | Hardware integration guide |
| [12-DEMO-GUIDE.md](docs/12-DEMO-GUIDE.md) | **Comprehensive demonstration guide** |
| [FINAL-STATUS.md](FINAL-STATUS.md) | **Complete system status** |

**Quick Links:**
- 🎯 [Demonstration Guide](docs/12-DEMO-GUIDE.md) - Step-by-step demo procedures
- ⚙️ [Hardware Integration](docs/11-SENSOR-ACTUATOR-INTEGRATION.md) - Connect real sensors
- 📊 [System Status](FINAL-STATUS.md) - Current feature status
- 🐛 [Troubleshooting](docs/08-TROUBLESHOOTING.md) - Common issues

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
- Qt5 GUI with touchscreen (rotate=90 calibration)
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

See [docs/08-TROUBLESHOOTING.md](docs/08-TROUBLESHOOTING.md) for complete guide.

---

## 📦 Project Structure

```
leafsense-project/
├── CMakeLists.txt                 # Build configuration
├── README.md                      # This file
├── FINAL-STATUS.md               # Complete system status
├── build-arm64/                   # ARM64 build directory
├── database/
│   └── schema.sql                # Database schema
├── deploy/                        # Deployment scripts
│   ├── toolchain-rpi4.cmake
│   └── setup-waveshare35c.sh
├── docs/                          # Documentation (17 files)
│   ├── 00-TERMINOLOGY.md         # Technical terms explained
│   ├── 05-DEVICE-DRIVER.md       # Device driver guide
│   └── 13-KERNEL-MODULE.md       # Low-level kernel details
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

1. Check [FINAL-STATUS.md](FINAL-STATUS.md) for system status
2. Review [docs/08-TROUBLESHOOTING.md](docs/08-TROUBLESHOOTING.md)
3. Contact development team (see Authors section)

---

**Last Updated:** January 11, 2026  
**Build Status:** ✅ Passing  
**System Status:** ✅ Production Ready

---

## 📋 Version 1.4.0 Highlights

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
- Complete device driver guide ([docs/13-DEVICE-DRIVER.md](docs/13-DEVICE-DRIVER.md))
- 14 comprehensive documentation files
- Full API reference and integration examples
