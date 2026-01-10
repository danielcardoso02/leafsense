# LeafSense - Raspberry Pi Deployment Guide

## Overview

This document provides comprehensive step-by-step instructions for deploying the LeafSense hydroponic monitoring system on a Raspberry Pi 4 Model B. The deployment process involves configuring a custom Linux distribution using Buildroot, cross-compiling the application, and setting up all required dependencies.

---

## Requirements

### Hardware
- Raspberry Pi 4 Model B (2GB+ RAM)
- microSD card (16GB+, recommended 32GB)
- USB-Ethernet adapter or WiFi connection
- 5V 3A USB-C power supply
- Waveshare 3.5" LCD (C) touchscreen (optional)

### Software (Development Host)
- Ubuntu 22.04+ or similar Linux distribution
- Buildroot 2025.08
- CMake 3.22+
- Git

---

## Step 1: Configure Buildroot

### 1.1 Download Buildroot
```bash
cd ~/buildroot
wget https://buildroot.org/downloads/buildroot-2025.08.tar.xz
tar xf buildroot-2025.08.tar.xz
cd buildroot-2025.08
```

### 1.2 Base Configuration
```bash
make raspberrypi4_64_defconfig
make menuconfig
```

### 1.3 Required Configuration Options

#### Target Options
- Target Architecture: AArch64 (little endian)
- Target Architecture Variant: cortex-A72

#### Toolchain
- C library: glibc
- Enable C++ support: YES

#### System Configuration
- Root password: `leafsense`
- System hostname: `leafsense-pi`

#### Target packages → Graphic libraries and applications → Qt5
- [x] qt5base
- [x] gui module
- [x] widgets module
- [x] sqlite2 support
- [x] qt5charts
- [x] qt5svg

#### Target packages → Libraries → Graphics
- [x] opencv4
- [x] opencv4 python support

#### Target packages → Networking applications
- [x] dropbear (SSH)
- [x] dhcp client

#### Target packages → Hardware handling
- [x] i2c-tools
- [x] Enable 1-Wire support

#### Filesystem images
- [x] ext2/3/4 root filesystem
- Size: 256M (BR2_TARGET_ROOTFS_EXT2_SIZE="256M")

### 1.4 Build Buildroot
```bash
make -j4
# Takes approximately 1-2 hours on first build
```

The image will be generated at:
```
output/images/sdcard.img (288MB with 256MB rootfs)
```

---

## Step 2: ONNX Runtime ARM64 Setup

### 2.1 Download
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir -p external/onnxruntime-arm64
cd external/onnxruntime-arm64

wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-aarch64-1.16.3.tgz
tar xzf onnxruntime-linux-aarch64-1.16.3.tgz
mv onnxruntime-linux-aarch64-1.16.3/* .
rm -rf onnxruntime-linux-aarch64-1.16.3*
```

### 2.2 Expected Directory Structure
```
external/onnxruntime-arm64/
├── include/
│   └── onnxruntime/
│       └── core/
│           └── session/
│               └── onnxruntime_cxx_api.h
└── lib/
    ├── libonnxruntime.so -> libonnxruntime.so.1.16.3
    ├── libonnxruntime.so.1 -> libonnxruntime.so.1.16.3
    └── libonnxruntime.so.1.16.3
```

---

## Step 3: Cross-Compilation

### 3.1 Toolchain File
The file `deploy/toolchain-rpi4.cmake`:
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Buildroot toolchain
set(TOOLCHAIN_PREFIX "/home/daniel/buildroot/buildroot-2025.08/output/host")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-g++")
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")

# ONNX Runtime
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/external/onnxruntime-arm64")
```

### 3.2 Compile LeafSense
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
mkdir -p build-arm64 && cd build-arm64

cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

### 3.3 Verify Binary
```bash
file LeafSense
# LeafSense: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV)...

ls -lh LeafSense
# -rwxrwxr-x 1 daniel daniel 380K ... LeafSense
```

---

## Step 4: Kernel Module Compilation

### 4.1 Compile the LED Module
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/drivers/kernel_module

export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
export CROSS_COMPILE=~/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-
export ARCH=arm64

make
```

### 4.2 Verify Module
```bash
file led.ko
# led.ko: ELF 64-bit LSB relocatable, ARM aarch64...

ls -lh led.ko
# -rw-rw-r-- 1 daniel daniel 13K ... led.ko
```

---

## Step 5: Flash SD Card

### 5.1 Identify Device
```bash
lsblk
# Identify SD card (e.g., /dev/sda)
```

### 5.2 Flash Image
```bash
sudo dd if=~/buildroot/buildroot-2025.08/output/images/sdcard.img \
       of=/dev/sdX \
       bs=4M \
       status=progress \
       conv=fsync

sync
```

### 5.3 Set Partition Labels
```bash
# Set labels for easier identification
sudo partprobe /dev/sdX
sudo fatlabel /dev/sdX1 BOOT
sudo e2label /dev/sdX2 ROOTFS

# Verify
lsblk -o NAME,SIZE,LABEL,FSTYPE /dev/sdX
```

---

## Step 6: Deploy Files to SD Card

### 6.1 Mount SD Card
```bash
sudo mkdir -p /mnt/sd-rootfs
sudo mount /dev/sdX2 /mnt/sd-rootfs
```

### 6.2 Deploy ONNX Runtime
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project

# Copy library
sudo cp external/onnxruntime-arm64/lib/libonnxruntime.so.1.16.3 \
        /mnt/sd-rootfs/usr/lib/

# Create symlinks
sudo ln -sf libonnxruntime.so.1.16.3 /mnt/sd-rootfs/usr/lib/libonnxruntime.so.1
sudo ln -sf libonnxruntime.so.1 /mnt/sd-rootfs/usr/lib/libonnxruntime.so
```

### 6.3 Deploy LeafSense Application
```bash
# Create directory structure
sudo mkdir -p /mnt/sd-rootfs/opt/leafsense/database

# Copy LeafSense binary
sudo cp build-arm64/LeafSense /mnt/sd-rootfs/opt/leafsense/
sudo chmod +x /mnt/sd-rootfs/opt/leafsense/LeafSense

# Copy ML model and class labels
sudo cp ml/leafsense_model.onnx /mnt/sd-rootfs/opt/leafsense/
sudo cp ml/leafsense_model_classes.txt /mnt/sd-rootfs/opt/leafsense/

# Copy database schema
sudo cp database/schema.sql /mnt/sd-rootfs/opt/leafsense/database/
```

### 6.4 Verify Deployment
```bash
# Check all files are in place
ls -lh /mnt/sd-rootfs/opt/leafsense/
ls -lh /mnt/sd-rootfs/usr/lib/libonnxruntime*

# Unmount
sudo umount /mnt/sd-rootfs
sync
```

---

## Step 7: First Boot and Testing

### 7.1 Boot the Raspberry Pi
```bash
# Insert SD card and power on
# Login: root
# Password: leafsense
```

### 7.2 Manual Test (HDMI Display)
```bash
# Set Qt platform for HDMI framebuffer
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0

# Navigate to application directory
cd /opt/leafsense

# Initialize database
sqlite3 leafsense.db < database/schema.sql

# Run LeafSense
./LeafSense -platform linuxfb:fb=/dev/fb0
```

### 7.3 Manual Test (Waveshare 3.5" Touchscreen)
```bash
# Verify framebuffer devices
cat /proc/fb
# Expected: 0 BCM2708 FB, 1 fb_ili9486

# Navigate to application directory
cd /opt/leafsense

# Initialize database
sqlite3 leafsense.db < database/schema.sql

# Run LeafSense on Waveshare display (fb1)
env QT_QPA_PLATFORM=linuxfb \
    QT_QPA_MOUSEDRIVER=linuxinput \
    ./LeafSense -platform linuxfb:fb=/dev/fb1
```

### 7.4 Create LeafSense Startup Script
```bash
cat > /opt/leafsense/start_leafsense.sh << 'EOF'
#!/bin/sh
# LeafSense Display Startup Script

cd /opt/leafsense
killall LeafSense 2>/dev/null || true
sleep 1

exec env \
    QT_QPA_PLATFORM=linuxfb \
    QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins \
    QT_QPA_FONTDIR=/usr/share/fonts \
    QT_QPA_MOUSEDRIVER=linuxinput \
    ./LeafSense -platform linuxfb:fb=/dev/fb1
EOF

chmod +x /opt/leafsense/start_leafsense.sh
```

### 7.5 Create Auto-Start Service (Optional)
```bash
cat > /etc/init.d/S98leafsense << 'EOF'
#!/bin/sh

case "$1" in
    start)
        echo "Starting LeafSense..."
        cd /opt/leafsense
        # Use startup script for proper display configuration
        ./start_leafsense.sh > /var/log/leafsense.log 2>&1 &
        echo "LeafSense started with PID $!"
        ;;
    stop)
        echo "Stopping LeafSense..."
        killall LeafSense 2>/dev/null || true
        ;;
    restart)
        $0 stop
        sleep 1
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
exit 0
EOF

chmod +x /etc/init.d/S98leafsense
```

### 7.6 Test Auto-Start
```bash
/etc/init.d/S98leafsense start
ps aux | grep LeafSense
tail -f /var/log/leafsense.log
```

---

## Files on the Raspberry Pi

| Path | Description |
|------|-------------|
| `/opt/leafsense/LeafSense` | Application binary |
| `/opt/leafsense/leafsense_model.onnx` | ML model |
| `/opt/leafsense/leafsense.db` | SQLite database |
| `/opt/leafsense/start_leafsense.sh` | Startup script |
| `/usr/lib/libonnxruntime.so*` | ONNX Runtime libraries |
| `/lib/modules/6.12.41-v8/led.ko` | LED kernel module |
| `/etc/init.d/S98leafsense` | Init script |
| `/var/log/leafsense.log` | Application log |

---

## Troubleshooting

### Problem: Pi not found on network
```bash
# Verify network interfaces
ip link show
ip addr show

# Scan for Pi on the network
arp-scan --interface=enx00e04c3601a6 --localnet
```

### Problem: Library not found
```bash
# Verify dependencies
ldd /opt/leafsense/LeafSense

# Verify LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib
```

### Problem: Qt platform plugin not found
```bash
# Use offscreen plugin (no display)
export QT_QPA_PLATFORM=offscreen

# OR with HDMI display
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0

# OR with Waveshare 3.5" touchscreen
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
```

### Problem: ML model fails to load
```bash
# Verify file exists
ls -la /opt/leafsense/leafsense_model.onnx

# Check file size (should be ~6MB)
ls -lh /opt/leafsense/leafsense_model.onnx
```

---

*Document last updated: January 9, 2026*
