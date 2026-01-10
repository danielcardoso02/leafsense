# LeafSense - Documentation

**Version:** 1.4.0  
**Last Updated:** January 10, 2026

---

## Index

0. [**Terminology & Concepts Guide**](00-TERMINOLOGY.md) ← **Start here!**
1. [Overview do Projeto](01-OVERVIEW.md)
2. [Architecture do System](02-ARCHITECTURE.md)
3. [Machine Learning](03-MACHINE-LEARNING.md)
4. [Buildroot Image Configuration](04-BUILDROOT-IMAGE.md)
5. [Deployment - Raspberry Pi](05-RASPBERRY-PI-DEPLOYMENT.md)
6. [Device Driver (LED Module)](06-DEVICE-DRIVER.md)
7. [Base de Data](07-DATABASE.md)
8. [Interface Gráfica (Qt5)](08-GUI.md)
9. [Guide de Troubleshooting](09-TROUBLESHOOTING.md)
10. [Changelog e History](10-CHANGELOG.md)
11. [Secção de Implementation (Report)](11-IMPLEMENTATION-REPORT.md)
12. [Testing Guide](11-TESTING-GUIDE.md)
13. [Sensor & Actuator Integration](11-SENSOR-ACTUATOR-INTEGRATION.md)
14. [Demo Guide](12-DEMO-GUIDE.md)
15. [Kernel Module - Low Level](13-KERNEL-MODULE.md)

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
