# LeafSense - Intelligent Hydroponic Monitoring System

<p align="center">
  <img src="resources/images/logo.png" alt="LeafSense Logo" width="200">
</p>

<p align="center">
  <strong>An embedded system for intelligent hydroponic plant monitoring and control</strong>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#architecture">Architecture</a> •
  <a href="#authors">Authors</a>
</p>

---

## Overview

LeafSense is a complete embedded system for intelligent hydroponic plant monitoring and control. It combines real-time sensor data acquisition, automatic control logic, machine learning-based disease detection, and a touchscreen interface—all running on a Raspberry Pi 4B with custom Buildroot Linux.

## Features

| Feature | Description |
|---------|-------------|
| **Real-Time Monitoring** | Temperature, pH, EC sensors with 2-second update cycle |
| **Automatic Control** | pH adjustment, nutrient dosing, temperature regulation |
| **ML Disease Detection** | ONNX Runtime with 99.39% accuracy (4 classes) |
| **Touchscreen UI** | Qt5-based interface on Waveshare 3.5" LCD (480×320) |
| **Camera Integration** | Raspberry Pi Camera Module v1 with periodic capture |
| **Database Logging** | SQLite persistence for sensor data, ML detections, events |
| **LED Alerts** | Custom kernel module for hardware alert indication |
| **Cross-Platform** | PC (x86_64) and ARM64 builds supported |

### ML Disease Classification
- Healthy
- Nutrient Deficiency
- Pest Damage
- Disease

---

## Hardware Requirements

| Component | Description |
|-----------|-------------|
| **Board** | Raspberry Pi 4 Model B (2GB+ RAM) |
| **Display** | Waveshare 3.5" LCD (C) - ILI9486 controller |
| **Touch** | ADS7846 resistive touchscreen |
| **Camera** | Raspberry Pi Camera Module v1 (OV5647) - optional |
| **Temperature** | DS18B20 (1-Wire) |
| **pH/EC** | Analog sensors via ADS1115 ADC (I2C) |
| **Actuators** | GPIO-controlled peristaltic pumps and heater |

---

## Quick Start

### Prerequisites

**Development Machine:**
- Ubuntu 22.04+ or similar Linux distribution
- GCC/G++ 11.4+
- CMake 3.16+
- Qt5 development libraries
- Buildroot toolchain (for ARM64 cross-compilation)

### Building

#### PC Build (Development/Testing)

```bash
git clone https://github.com/your-username/leafsense-project.git
cd leafsense-project

mkdir build && cd build
cmake ..
make -j$(nproc)

./src/LeafSense
```

#### ARM64 Build (Raspberry Pi)

```bash
# Setup ONNX Runtime for ARM64 (first time only)
./deploy/setup-onnxruntime-arm64.sh

# Configure cross-compilation build
mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)

# Output: build-arm64/src/LeafSense
```

### Deployment

```bash
# Flash Buildroot image to SD card
sudo dd if=sdcard.img of=/dev/sdX bs=4M status=progress && sync

# Deploy binary to Raspberry Pi
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/

# Start application
ssh root@10.42.0.196 '/opt/leafsense/start_leafsense.sh &'
```

---

## Documentation

Complete documentation is available in the [`docs/`](docs/) directory:

| Document | Description |
|----------|-------------|
| [Project Status](docs/00-PROJECT-STATUS.md) | Current implementation status and roadmap |
| [Overview](docs/01-OVERVIEW.md) | Project objectives and technology stack |
| [Architecture](docs/02-ARCHITECTURE.md) | System design and patterns |
| [Machine Learning](docs/03-MACHINE-LEARNING.md) | ML model training and inference |
| [Networking](docs/04-NETWORKING.md) | USB-Ethernet and SSH configuration |
| [Buildroot Image](docs/05-BUILDROOT-IMAGE.md) | Custom Linux image creation |
| [Deployment](docs/06-RASPBERRY-PI-DEPLOYMENT.md) | Deployment procedures |
| [Device Driver](docs/07-DEVICE-DRIVER.md) | LED kernel module development |
| [Database](docs/08-DATABASE.md) | SQLite schema and queries |
| [GUI](docs/09-GUI.md) | Qt5 interface implementation |
| [Troubleshooting](docs/10-TROUBLESHOOTING.md) | Common issues and solutions |
| [Changelog](docs/11-CHANGELOG.md) | Version history |
| [Hardware Integration](docs/12-SENSOR-ACTUATOR-INTEGRATION.md) | Sensor and actuator setup |
| [Testing Guide](docs/13-TESTING-GUIDE.md) | Testing procedures |
| [Kernel Module](docs/14-KERNEL-MODULE.md) | Low-level kernel development |
| [Touchscreen](docs/15-TOUCHSCREEN-CONFIGURATION.md) | Display calibration |
| [Buildroot Packages](docs/16-BUILDROOT-PACKAGES.md) | Package configuration |
| [Terminology](docs/17-TERMINOLOGY.md) | Key terms explained |

---

## Architecture

### Software Stack

| Layer | Technology |
|-------|------------|
| **OS** | Buildroot 2025.08 (Linux 6.12.41-v8) |
| **Language** | C++17 |
| **GUI** | Qt 5.15.14 (Widgets, Charts, SQL) |
| **ML** | ONNX Runtime 1.16.3 |
| **Vision** | OpenCV 4.11.0 |
| **Database** | SQLite 3.48.0 |

### Thread Architecture

```
Master Controller (8 threads)
├── tTime          - Heartbeat timer (2s)
├── tSig           - Signal dispatcher
├── tReadSensors   - Sensor polling & control logic
├── tCamera        - Image capture & ML inference
├── tWaterHeater   - Temperature control
├── tPHU           - pH Up pump
├── tPHD           - pH Down pump
└── tNutrients     - Nutrient dosing
```

### Project Structure

```
leafsense-project/
├── CMakeLists.txt          # Build configuration
├── database/
│   └── schema.sql          # Database schema
├── deploy/                 # Deployment scripts and toolchain
├── docs/                   # Documentation
├── drivers/
│   └── kernel_module/      # LED kernel driver
├── external/
│   └── onnxruntime-arm64/  # Pre-built ONNX Runtime
├── include/                # Header files
│   ├── application/
│   ├── drivers/
│   └── middleware/
├── ml/                     # Machine learning model and training
│   ├── leafsense_model.onnx
│   └── train_model.py
├── resources/              # Qt resources and images
└── src/                    # Source code
    ├── main.cpp
    ├── application/        # GUI and ML
    ├── drivers/            # Sensor and actuator drivers
    └── middleware/         # Business logic
```

---

## Testing

```bash
# Connect to Pi
ssh root@10.42.0.196

# Check system status
ps aux | grep LeafSense

# View sensor readings
tail -f /var/log/leafsense.log | grep SENSOR

# Check gallery
ls -lh /opt/leafsense/gallery/

# Test database
sqlite3 /opt/leafsense/database/leafsense.db ".tables"
```

See [Testing Guide](docs/13-TESTING-GUIDE.md) for comprehensive test procedures.

---

## Authors

**Group 11** - Embedded Systems and Computers

| Name | Student ID |
|------|------------|
| Daniel Gonçalo Silva Cardoso | PG53753 |
| Marco Xavier Leite Costa | PG60210 |

**Advisor:** Professor Adriano José Conceição Tavares

**Program:** Master's in Industrial Electronics and Computers Engineering  
**Institution:** University of Minho, School of Engineering

---

## License

This project was developed as part of an academic course at the University of Minho.

---

## Acknowledgments

- Professor Adriano Tavares for guidance and support
- [Qt Project](https://www.qt.io/) for the GUI framework
- [ONNX Runtime](https://onnxruntime.ai/) for ML inference
- [OpenCV](https://opencv.org/) for computer vision
- [Buildroot](https://buildroot.org/) for embedded Linux
