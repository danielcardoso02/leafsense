# LeafSense - Documentation

**Version:** 1.4.1  
**Last Updated:** January 10, 2026

---

## Index

| # | Document | Description |
|---|----------|-------------|
| 00 | [Terminology & Concepts](00-TERMINOLOGY.md) | ← **Start here!** Key terms and concepts |
| 01 | [Project Overview](01-OVERVIEW.md) | Project summary and goals |
| 02 | [System Architecture](02-ARCHITECTURE.md) | System design and components |
| 03 | [Machine Learning](03-MACHINE-LEARNING.md) | ML model and inference |
| 04 | [Networking Guide](04-NETWORKING.md) | Pi connectivity options |
| 05 | [Buildroot Image](05-BUILDROOT-IMAGE.md) | Custom Linux image creation |
| 06 | [Raspberry Pi Deployment](06-RASPBERRY-PI-DEPLOYMENT.md) | Deploying to the Pi |
| 07 | [Device Driver](07-DEVICE-DRIVER.md) | LED module driver |
| 08 | [Database](08-DATABASE.md) | SQLite schema and usage |
| 09 | [GUI (Qt5)](09-GUI.md) | Graphical interface |
| 10 | [Troubleshooting](10-TROUBLESHOOTING.md) | Common issues and fixes |
| 11 | [Changelog](11-CHANGELOG.md) | Version history |
| 12 | [Implementation Report](12-IMPLEMENTATION-REPORT.md) | Technical report |
| 13 | [Sensor & Actuator Integration](13-SENSOR-ACTUATOR-INTEGRATION.md) | Hardware integration (RTC, Relay) |
| 14 | [Testing Guide](14-TESTING-GUIDE.md) | Testing procedures |
| 15 | [Demo Guide](15-DEMO-GUIDE.md) | Demonstration instructions |
| 16 | [Kernel Module](16-KERNEL-MODULE.md) | Low-level kernel driver |

### Additional Documents
- [STATUS-REPORT-JAN10-2026.md](STATUS-REPORT-JAN10-2026.md) - Current project status

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
ssh root@10.42.0.196 'killall LeafSense; /opt/leafsense/LeafSense &'
```

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
/opt/leafsense/start_leafsense.sh
```

---

## Autores

**Grupo 11 - University of Minho, School of Engineering**

| Nome | Número |
|------|--------|
| Daniel Gonçalo Silva Cardoso | PG53753 |
| Marco Xavier Leite Costa | PG60210 |

**Orientador:** Professor Adriano José Conceição Tavares

**Curso:** Master's in Industrial Electronics and Computers Engineering  
**UC:** Embedded Systems and Computers

---

**Última atualização:** 3 de Dezembro de 2025
