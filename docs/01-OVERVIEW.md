# LeafSense - Project Overview

## Description

**LeafSense** is an intelligent monitoring and control system for hydroponic cultivation, developed for Raspberry Pi 4B. The system combines environmental sensors, automatic actuators, and Machine Learning for plant disease detection.

## Objectives

1. **Real-time monitoring** of environmental parameters (temperature, pH, EC)
2. **Automatic control** of dosing pumps for pH and nutrient maintenance
3. **Disease detection** in plants using computer vision and ML
4. **Graphical interface** for visualization and manual control
5. **Logging and analytics** for historical data and trend analysis

## Hardware Used

### Raspberry Pi 4 Model B
- **RAM:** 2GB
- **CPU:** Cortex-A72 (ARM64/aarch64) @ 1.5GHz
- **Connectivity:** WiFi, Ethernet, USB 3.0
- **GPIO:** 40 pins for sensors and actuators

### Display
| Component | Model | Resolution |
|-----------|-------|------------|
| Touchscreen | Waveshare 3.5" LCD (C) | 480x320 |
| Controller | ILI9486 (SPI) | 16-bit color |
| Touch | ADS7846 (SPI) | Resistive |

### Sensors
| Sensor | Interface | Function |
|--------|-----------|----------|
| DS18B20 | 1-Wire (GPIO19) | Water temperature |
| pH Sensor | I2C | pH measurement |
| EC Sensor | I2C | Electrical conductivity |
| Camera | CSI/USB | Image capture for ML |

### Actuators
| Actuator | Interface | Function |
|----------|-----------|----------|
| pH Up Pump | GPIO (Relay) | Increase pH |
| pH Down Pump | GPIO (Relay) | Decrease pH |
| Nutrient Pump | GPIO (Relay) | Add nutrients |
| LED Indicator | GPIO (Kernel Module) | System status |

## Technology Stack

### Software
- **Language:** C++17
- **GUI Framework:** Qt 5.15.14 (Widgets, Charts, Sql)
- **ML Runtime:** ONNX Runtime 1.16.3
- **Computer Vision:** OpenCV 4.11.0
- **Database:** SQLite 3.48.0
- **Build System:** CMake 3.22+

### Operating System
- **Buildroot 2025.08** - Custom embedded Linux system
- **Kernel:** Linux 6.12.41-v8 (64-bit ARM)
- **Init System:** BusyBox init

## Project Structure

```
leafsense-project/
├── CMakeLists.txt                  # Main build configuration
├── database/
│   └── schema.sql                  # Database schema
├── deploy/
│   ├── toolchain-rpi4.cmake        # Toolchain for cross-compilation
│   ├── configure-buildroot.sh      # Buildroot configuration script
│   └── setup-onnxruntime-arm64.sh
├── docs/                           # Documentation
├── drivers/
│   ├── kernel_module/              # Kernel module for LED
│   ├── actuators/                  # Actuator drivers
│   └── sensors/                    # Sensor drivers
├── external/
│   └── onnxruntime-arm64/          # Pre-compiled ONNX Runtime
├── include/
│   ├── application/
│   │   ├── gui/                    # GUI headers
│   │   └── ml/                     # ML system headers
│   ├── drivers/                    # Driver headers
│   └── middleware/                 # Middleware headers
├── ml/
│   ├── leafsense_model.onnx        # Trained model (5.9MB)
│   └── training/                   # Python training scripts
├── resources/
│   ├── resources.qrc               # Qt resources
│   └── images/                     # GUI images
└── src/
    ├── main.cpp                    # Entry point
    ├── application/
    │   ├── gui/                    # GUI implementation
    │   └── ml/                     # ML implementation
    ├── drivers/                    # Driver implementation
    └── middleware/                 # Middleware implementation
```

## Implemented Features

### ✅ Completed
- [x] Qt5 graphical interface with dashboard, charts, and settings
- [x] SQLite database system with complete schema
- [x] ONNX Runtime integration for ML inference
- [x] Trained ML model (4 classes, 99.39% accuracy)
- [x] Cross-compilation for ARM64
- [x] Kernel module for LED control
- [x] LED alert system integration (automatic control based on sensor thresholds)
- [x] Logging and alerting system
- [x] Auto-start on Raspberry Pi boot
- [x] Waveshare 3.5" touchscreen integration (ILI9486 + ADS7846)
- [x] Touch calibration for display (evdev with rotate=180:invertx)
- [x] Static IP network configuration
- [x] SSH remote access via Dropbear
- [x] Camera driver implementation (OV5647 Pi Camera Module v1)
- [x] Camera capture system with multi-device fallback and test pattern generation
- [x] Periodic camera capture system (30-minute intervals)
- [x] ML-based plant disease detection with captured images
- [x] Gallery interface for viewing captured images
- [x] Database integration for sensor readings, ML predictions, and logs
- [x] Complete system integration and testing

### 🔄 In Progress
- [ ] Real sensor hardware integration (mock mode working - guide available)
- [ ] pH and EC sensor calibration procedures (documented and ready)

### 📋 Planned
- [ ] Web server for remote access
- [ ] Push notifications
- [ ] Cloud service integration

## Documentation

Complete documentation available in `docs/` directory:

| Document | Description |
|----------|-------------|
| [00-PROJECT-STATUS.md](00-PROJECT-STATUS.md) | Implementation status |
| [01-OVERVIEW.md](01-OVERVIEW.md) | This file |
| [02-ARCHITECTURE.md](02-ARCHITECTURE.md) | System architecture |
| [03-MACHINE-LEARNING.md](03-MACHINE-LEARNING.md) | ML model training |
| [04-NETWORKING.md](04-NETWORKING.md) | Network configuration |
| [05-BUILDROOT-IMAGE.md](05-BUILDROOT-IMAGE.md) | Linux image creation |
| [06-RASPBERRY-PI-DEPLOYMENT.md](06-RASPBERRY-PI-DEPLOYMENT.md) | Deployment procedures |
| [07-DEVICE-DRIVER.md](07-DEVICE-DRIVER.md) | Kernel module guide |
| [08-DATABASE.md](08-DATABASE.md) | Database schema |
| [09-GUI.md](09-GUI.md) | GUI implementation |
| [10-TROUBLESHOOTING.md](10-TROUBLESHOOTING.md) | Common issues |
| [11-CHANGELOG.md](11-CHANGELOG.md) | Version history |
| [13-SENSOR-ACTUATOR-INTEGRATION.md](13-SENSOR-ACTUATOR-INTEGRATION.md) | Hardware integration |
| [14-TESTING-GUIDE.md](14-TESTING-GUIDE.md) | Testing procedures |
| [16-KERNEL-MODULE.md](16-KERNEL-MODULE.md) | Kernel driver details |
| [17-TOUCHSCREEN-CONFIGURATION.md](17-TOUCHSCREEN-CONFIGURATION.md) | LCD setup |
| [18-BUILDROOT-PACKAGES.md](18-BUILDROOT-PACKAGES.md) | Package reference |
| [19-TERMINOLOGY.md](19-TERMINOLOGY.md) | Technical terms |
