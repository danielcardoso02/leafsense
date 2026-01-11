# LeafSense - Documentation Index

**Version:** 1.5.1  
**Last Updated:** January 11, 2026

---

## 📋 Project Status

> **Start here:** [00-PROJECT-STATUS.md](00-PROJECT-STATUS.md) - Complete implementation status and what's missing

---

## 📚 Documentation Index

### Core Documentation

| # | Document | Description | Priority |
|---|----------|-------------|----------|
| 00 | [PROJECT-STATUS](00-PROJECT-STATUS.md) | **Complete project status and roadmap** | ⭐ Essential |
| 01 | [Project Overview](01-OVERVIEW.md) | Goals and objectives | Reference |
| 02 | [System Architecture](02-ARCHITECTURE.md) | Software and hardware design | Reference |
| 19 | [Terminology](19-TERMINOLOGY.md) | Key terms and concepts explained | Reference |

### Buildroot & Deployment

| # | Document | Description | Priority |
|---|----------|-------------|----------|
| 05 | [Buildroot Image](05-BUILDROOT-IMAGE.md) | Custom Linux image creation | ⭐ Essential |
| 06 | [Raspberry Pi Deployment](06-RASPBERRY-PI-DEPLOYMENT.md) | Deploying application to Pi | ⭐ Essential |
| 04 | [Networking Guide](04-NETWORKING.md) | USB-Ethernet and SSH setup | Reference |
| 18 | [Buildroot Packages](18-BUILDROOT-PACKAGES.md) | Complete package list | Reference |

### Hardware Integration

| # | Document | Description | Priority |
|---|----------|-------------|----------|
| 13 | [Sensor & Actuator Integration](13-SENSOR-ACTUATOR-INTEGRATION.md) | **How to connect real hardware** | ⭐ Essential |
| 17 | [Touchscreen Configuration](17-TOUCHSCREEN-CONFIGURATION.md) | Waveshare 3.5" LCD setup | Reference |
| 07 | [Device Driver (LED)](07-DEVICE-DRIVER.md) | Character device driver guide | Reference |
| 16 | [Kernel Module](16-KERNEL-MODULE.md) | Low-level GPIO kernel driver | Reference |

### Software Components

| # | Document | Description | Priority |
|---|----------|-------------|----------|
| 03 | [Machine Learning](03-MACHINE-LEARNING.md) | ONNX model training and inference | Reference |
| 08 | [Database](08-DATABASE.md) | SQLite schema and operations | Reference |
| 09 | [GUI (Qt5)](09-GUI.md) | Graphical interface design | Reference |

### Operations

| # | Document | Description | Priority |
|---|----------|-------------|----------|
| 10 | [Troubleshooting](10-TROUBLESHOOTING.md) | Common issues and solutions | Reference |
| 14 | [Testing Guide](14-TESTING-GUIDE.md) | Testing procedures | Reference |
| 15 | [Demo Guide](15-DEMO-GUIDE.md) | Demonstration instructions | Reference |
| 11 | [Changelog](11-CHANGELOG.md) | Version history | Reference |
| 12 | [Implementation Report](12-IMPLEMENTATION-REPORT.md) | Technical report (Portuguese) | Reference |

---

## Quick Start

### Prerequisites

**PC Development Build (x86_64):**
- GCC/G++ 11.4+
- CMake 3.16+
- Qt5 development packages
- SQLite3 development packages
- OpenCV 4.5+
- ONNX Runtime 1.20+

**Raspberry Pi Cross-Compilation (ARM64):**
- Buildroot toolchain (see [04-BUILDROOT-IMAGE.md](04-BUILDROOT-IMAGE.md))
- Cross-compilation toolchain at: `/home/daniel/buildroot/buildroot-2025.08/output/host/`

---

## Build Instructions

### PC Build (Native x86_64)

For development and testing on your desktop/laptop:

```bash
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**Output:** `build/src/LeafSense` (x86_64 ELF executable)

**Run locally:**
```bash
./src/LeafSense
```

---

### Raspberry Pi Build (Cross-Compiled ARM64)

For deployment to Raspberry Pi 4:

```bash
cd /home/daniel/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project

# Setup ONNX Runtime for ARM64 (first time only)
./deploy/setup-onnxruntime-arm64.sh

# Configure and build
mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

**Output:** `build-arm64/src/LeafSense` (ARM64 ELF executable)

**Deploy to Pi:**
```bash
scp src/LeafSense root@10.42.0.196:/opt/leafsense/
```

---

## Running the Application

### PC (Development/Testing)

Run directly on your Linux PC (requires X11 or Wayland):

```bash
cd build/src
./LeafSense
```

**Notes:**
- Uses default Qt platform (xcb on X11, wayland on Wayland)
- Camera uses /dev/video0 by default
- Database created at ./leafsense.db

### Raspberry Pi (Production)

**Option 1: Using startup script (recommended)**
```bash
ssh root@10.42.0.196
/opt/leafsense/start.sh
```

**Option 2: Manual execution with touchscreen**
```bash
ssh root@10.42.0.196
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
cd /opt/leafsense
./LeafSense -platform linuxfb:fb=/dev/fb1
```

**Option 3: Background execution**
```bash
ssh root@10.42.0.196
/opt/leafsense/start.sh &
```

**Stop the application:**
```bash
ssh root@10.42.0.196 'killall LeafSense'
```

### Critical Touch Configuration

The Waveshare 3.5" LCD with `rotate=270` in `/boot/config.txt` requires:

```bash
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
```

**Note:** The `:invertx` parameter corrects horizontal button mapping.

---

## Clean Build

If you encounter build errors or change CMakeLists.txt:

```bash
# PC build
rm -rf build && mkdir build && cd build && cmake .. && make -j$(nproc)

# ARM64 build
rm -rf build-arm64 && mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake .. && make -j$(nproc)
```

---

## Raspberry Pi Access
```bash
ssh root@10.42.0.196
# Password: leafsense
```

### Start Application (on Pi)
```bash
/opt/leafsense/start.sh
```

---

## Authors

**Group 11 - University of Minho, School of Engineering**

| Name | Student ID |
|------|------------|
| Daniel Gonçalo Silva Cardoso | PG53753 |
| Marco Xavier Leite Costa | PG60210 |

**Advisor:** Professor Adriano José Conceição Tavares

**Program:** Master's in Industrial Electronics and Computers Engineering  
**Course:** Embedded Systems and Computers

---

**Last Updated:** January 11, 2026
