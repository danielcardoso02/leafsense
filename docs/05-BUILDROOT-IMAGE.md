# Buildroot Image Configuration and Creation

**Version**: 1.5.1  
**Last Updated**: January 19, 2026

## Table of Contents

1. [Overview](#overview)
2. [What is Buildroot?](#what-is-buildroot)
3. [LeafSense Buildroot Configuration](#leafsense-buildroot-configuration)
4. [Directory Structure](#directory-structure)
5. [Configuration Files Explained](#configuration-files-explained)
6. [Building the Image](#building-the-image)
7. [Image Contents](#image-contents)
8. [Customization](#customization)
9. [Troubleshooting Build Issues](#troubleshooting-build-issues)
10. [Updating the Image](#updating-the-image)

---

## Overview

LeafSense runs on a custom-built Linux system created with **Buildroot**. This document explains:
- What Buildroot is and why we use it
- How the LeafSense image is configured
- What's included in the image
- How to rebuild or modify the image

**Key Concepts:**
- **Buildroot Image** = Complete operating system for Raspberry Pi (kernel + rootfs + bootloader)
- **Configuration File** = `.config` file that defines what packages and features are included
- **Defconfig** = Default configuration template for specific hardware (e.g., `raspberrypi4_64_defconfig`)
- **Rebuild** = Necessary when adding hardware support (like camera) or updating packages

---

## What is Buildroot?

**Buildroot** is a tool that generates complete embedded Linux systems through cross-compilation.

### Why Buildroot for LeafSense?

1. **Minimal Size**: Only includes what's needed (~300MB vs several GB for full Raspberry Pi OS)
2. **Customizable**: Complete control over packages, kernel, and configuration
3. **Reproducible**: Same configuration always produces same image
4. **Fast Boot**: Boots in ~10 seconds vs 30+ seconds for full Linux distros
5. **Embedded-Focused**: Designed for headless, single-application systems like LeafSense

### Alternatives Considered

| System | Size | Boot Time | Customization | Why Not Used |
|--------|------|-----------|---------------|--------------|
| Raspberry Pi OS | 2-4 GB | 30-40s | Low | Too bloated for single-app |
| Yocto/OpenEmbedded | Variable | Fast | Very High | Too complex, steep learning curve |
| Buildroot | 300 MB | ~10s | High | **✅ CHOSEN** - Best balance |
| Alpine Linux | ~200 MB | Fast | Medium | Less Pi-specific support |

---

## LeafSense Buildroot Configuration

### Hardware Target

- **Board**: Raspberry Pi 4 Model B (64-bit ARM)
- **Processor**: Broadcom BCM2711 (Quad-core Cortex-A72 @ 1.5GHz)
- **Architecture**: AArch64 (ARM64)
- **Base Config**: `raspberrypi4_64_defconfig`

### System Specifications

```
Kernel:        Linux 6.12.41-v8 (custom)
C Library:     glibc 2.40
Init System:   BusyBox init
Shell:         BusyBox ash
Package Mgr:   None (static image)
Root Password: leafsense
Hostname:      leafsense-pi
```

### Key Packages Included

#### Core System
- **BusyBox 1.37.0** - Minimal Unix utilities
- **eudev** - Device management (udev fork for non-systemd systems)
- **dropbear** - Lightweight SSH server
- **iptables** - Network filtering

#### Camera & Video
- ✅ **rpi-userland** - VideoCore libraries (vcgencmd, dtoverlay)
- ✅ **bcm2835-v4l2** - Camera kernel module
- ✅ **bcm2835-unicam** - Camera interface driver
- ✅ **ov5647** - Camera sensor driver (Pi Camera v1)

#### Graphics (for Waveshare 3.5" LCD)
- **Qt5 5.15.14** - GUI framework
- **fbdev** - Framebuffer device support
- **libevdev** - Touchscreen via evdev (NOT tslib - see 15-TOUCHSCREEN-CONFIGURATION.md)

#### Libraries
- **OpenCV 4.11.0** - Computer vision (with V4L2 support)
- **SQLite 3.48.0** - Database engine
- **ONNX Runtime 1.16.3** - ML inference (external)
- **glibc 2.40** - Standard C library

#### Development Tools (Host Only)
- **CMake** - Build system
- **GCC 14.3.0** - Cross-compiler toolchain
- **pkg-config** - Library configuration

---

## Directory Structure

### Buildroot Installation

```
/home/daniel/buildroot/buildroot-2025.08/
├── .config                          # Current build configuration
├── configs/                         # Default configurations
│   └── raspberrypi4_64_defconfig   # Base Pi 4 64-bit config
├── board/
│   ├── raspberrypi4-64/
│   │   ├── post-build.sh           # Script run after rootfs built
│   │   ├── post-image.sh           # Script run after image created
│   │   └── genimage-raspberrypi4-64.cfg  # SD card layout
│   ├── raspberrypi/
│   │   └── linux-camera.config     # Kernel camera drivers config
│   └── leafsense/
│       └── post-build.sh           # LeafSense-specific setup
├── output/
│   ├── images/
│   │   ├── sdcard.img              # ✅ FINAL FLASHABLE IMAGE
│   │   ├── rootfs.ext4             # Root filesystem
│   │   ├── Image                   # Linux kernel
│   │   ├── bcm2711-rpi-4-b.dtb     # Device tree blob
│   │   └── boot.vfat               # Boot partition
│   ├── target/                     # Unpacked root filesystem
│   ├── build/                      # Package build directories
│   └── host/                       # Cross-compilation toolchain
├── package/                        # Package definitions
└── linux/                          # Linux kernel source
```

### LeafSense Project Structure

```
leafsense-project/
├── deploy/
│   ├── buildroot-camera.config     # Camera package fragment
│   ├── linux-camera.config         # Kernel camera config
│   ├── enable-camera.sh            # Automated camera enablement
│   ├── toolchain-rpi4.cmake        # CMake cross-compilation config
│   └── CAMERA_SETUP_GUIDE.md       # Camera troubleshooting guide
└── docs/
    └── 05-BUILDROOT-IMAGE.md       # This document
```

---

## Configuration Files Explained

### 1. Main Configuration: `.config`

**Location**: `/home/daniel/buildroot/buildroot-2025.08/.config`

This file contains **every** configuration option for the build (5000+ lines). It's generated from:
- Base defconfig (`raspberrypi4_64_defconfig`)
- Manual changes via `make menuconfig`
- Configuration fragments (e.g., `buildroot-camera.config`)

**Key Settings:**

```bash
# Target Architecture
BR2_aarch64=y                        # 64-bit ARM
BR2_cortex_a72=y                     # Pi 4 CPU type

# System Configuration
BR2_TARGET_GENERIC_HOSTNAME="leafsense-pi"
BR2_TARGET_GENERIC_ROOT_PASSWD="leafsense"
BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_EUDEV=y

# Bootloader
BR2_TARGET_UBOOT=n                   # Pi uses GPU bootloader instead

# Kernel
BR2_LINUX_KERNEL=y
BR2_LINUX_KERNEL_CUSTOM_TARBALL=y
BR2_LINUX_KERNEL_VERSION="6.12.41"
BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="board/raspberrypi/linux-camera.config"

# Camera Support (ADDED FOR v1.4.0)
BR2_PACKAGE_RPI_USERLAND=y           # VideoCore libraries
BR2_PACKAGE_V4L_UTILS=y              # Video4Linux utilities

# Graphics & GUI
BR2_PACKAGE_QT5=y
BR2_PACKAGE_QT5BASE=y
BR2_PACKAGE_QT5BASE_GUI=y
BR2_PACKAGE_QT5BASE_WIDGETS=y

# Computer Vision
BR2_PACKAGE_OPENCV4=y
BR2_PACKAGE_OPENCV4_LIB_VIDEOIO=y    # For camera capture
BR2_PACKAGE_OPENCV4_WITH_V4L=y       # V4L2 support

# Database
BR2_PACKAGE_SQLITE=y

# Networking
BR2_PACKAGE_DROPBEAR=y               # SSH server
BR2_SYSTEM_DHCP="eth0"               # DHCP on Ethernet
```

### 2. Kernel Camera Configuration: `linux-camera.config`

**Location**: `board/raspberrypi/linux-camera.config`

This is a **configuration fragment** that enables camera drivers in the Linux kernel.

```bash
# Media Support Framework
CONFIG_MEDIA_SUPPORT=y               # Enable media subsystem
CONFIG_MEDIA_CAMERA_SUPPORT=y        # Camera support
CONFIG_VIDEO_DEV=y                   # Video device framework
CONFIG_VIDEO_V4L2=y                  # Video4Linux2 API
CONFIG_VIDEO_V4L2_SUBDEV_API=y       # Subdevice API

# Broadcom Camera Drivers
CONFIG_VIDEO_BCM2835=m               # VideoCore camera (module)
CONFIG_VIDEO_BCM2835_UNICAM=m        # Camera interface (module)
CONFIG_VIDEO_BCM2835_ISP=m           # Image Signal Processor (module)

# Camera Sensors
CONFIG_VIDEO_OV5647=m                # OV5647 sensor (Pi Camera v1)
CONFIG_VIDEO_IMX219=m                # IMX219 sensor (Pi Camera v2/HQ)

# Media Controller
CONFIG_MEDIA_CONTROLLER=y            # Media pipeline management
CONFIG_V4L2_MEM2MEM_DEV=m            # Memory-to-memory framework

# DMA Memory
CONFIG_CMA=y                         # Contiguous Memory Allocator
CONFIG_DMA_CMA=y                     # CMA for DMA
```

**Why Modules (`=m`) Instead of Built-in (`=y`)?**
- Modules can be loaded/unloaded dynamically
- Saves kernel memory when camera not in use
- Easier to troubleshoot (can reload module without reboot)
- Standard practice for device drivers

### 3. Defconfig: `raspberrypi4_64_defconfig`

**Location**: `configs/raspberrypi4_64_defconfig`

This is the **base configuration** provided by Buildroot for Pi 4. Our `.config` starts from this and adds:
- Camera support
- Qt5 GUI
- OpenCV
- Custom hostname/password

**To reset to defaults:**
```bash
cd /home/daniel/buildroot/buildroot-2025.08
make raspberrypi4_64_defconfig
# Then add back custom packages
```

### 4. Post-Build Script: `post-build.sh`

**Location**: `board/leafsense/post-build.sh`

This script runs **after** the root filesystem is built but **before** the image is created. Use it to:
- Copy custom configuration files
- Create directories
- Set permissions
- Install LeafSense binary (if automated)

**Example:**
```bash
#!/bin/sh
set -e

# Create LeafSense directories
mkdir -p ${TARGET_DIR}/opt/leafsense
mkdir -p ${TARGET_DIR}/opt/leafsense/gallery
mkdir -p ${TARGET_DIR}/var/log

# Set permissions
chmod 755 ${TARGET_DIR}/opt/leafsense

echo "LeafSense post-build completed"
```

### 5. Image Generation: `genimage-raspberrypi4-64.cfg`

**Location**: `board/raspberrypi4-64/genimage-raspberrypi4-64.cfg`

Defines the SD card partition layout:

```
image boot.vfat {
  vfat {
    files = {
      "bcm2711-rpi-4-b.dtb",
      "rpi-firmware/config.txt",
      "rpi-firmware/cmdline.txt",
      "rpi-firmware/start4.elf",
      "rpi-firmware/fixup4.dat",
      "Image"
    }
  }
  size = 32M
}

image sdcard.img {
  hdimage {
  }

  partition boot {
    partition-type = 0xC
    bootable = "true"
    image = "boot.vfat"
  }

  partition rootfs {
    partition-type = 0x83
    image = "rootfs.ext4"
    size = 256M
  }
}
```

**Result**: 2-partition SD card
- **Partition 1** (32MB, FAT32): Boot files (firmware, kernel, device tree)
- **Partition 2** (256MB, ext4): Root filesystem (Linux system)

---

## Building the Image

### Prerequisites

**System Requirements:**
- Ubuntu 20.04+ or Debian 11+
- 20+ GB free disk space
- 4+ GB RAM
- 2-4 hours for first build
- ~5-10 minutes for incremental rebuilds

**Required Packages:**
```bash
sudo apt update
sudo apt install -y \
  build-essential \
  git \
  wget \
  cpio \
  unzip \
  rsync \
  bc \
  libncurses5-dev \
  python3
```

### Step 1: Download Buildroot

```bash
cd ~/buildroot
wget https://buildroot.org/downloads/buildroot-2025.08.tar.gz
tar xzf buildroot-2025.08.tar.gz
cd buildroot-2025.08
```

### Step 2: Load Base Configuration

```bash
# Start with Raspberry Pi 4 64-bit defaults
make raspberrypi4_64_defconfig

# Or restore existing configuration
cp /path/to/saved/.config .
make olddefconfig
```

### Step 3: Configure (Optional)

```bash
# Open text-based menu
make menuconfig

# Navigate with arrow keys, select with spacebar, save with 'S'
```

**Important Menus:**
- `Target options` - Architecture (aarch64, cortex-a72)
- `Toolchain` - Compiler options
- `System configuration` - Hostname, passwords, init system
- `Kernel` - Linux kernel version and config
- `Target packages` → `Hardware handling` - rpi-userland, v4l-utils
- `Filesystem images` - Output image type (ext4, size)

### Step 4: Enable Camera Support

**Automated (Recommended):**
```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project
./deploy/enable-camera.sh
# Follow prompts
```

**Manual:**
```bash
cd /home/daniel/buildroot/buildroot-2025.08

# Copy kernel camera config
cp ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/deploy/linux-camera.config \
   board/raspberrypi/linux-camera.config

# Edit .config to enable packages
echo "BR2_PACKAGE_RPI_USERLAND=y" >> .config
echo "BR2_PACKAGE_V4L_UTILS=y" >> .config
echo 'BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="board/raspberrypi/linux-camera.config"' >> .config

# Process changes
make olddefconfig
```

### Step 5: Build

```bash
# Full build (first time: 2-4 hours)
make

# Or specify parallel jobs
make -j$(nproc)

# Clean and rebuild everything
make clean && make

# Rebuild just one package
make <package>-rebuild
```

**Build Output:**
```
>>> Finalizing host directory
>>> Finalizing target directory
>>> Sanitizing RPATH in target tree
>>> Generating root filesystems common tables
>>> Generating filesystem image rootfs.ext2
>>> Executing post-image script

✅ Build complete!
Image: output/images/sdcard.img
```

### Step 6: Verify Output

```bash
ls -lh output/images/

# Should see:
# sdcard.img      (289 MB) - ✅ Final flashable image
# rootfs.ext4     (256 MB) - Root filesystem
# Image           (24 MB)  - Linux kernel
# boot.vfat       (32 MB)  - Boot partition
# *.dtb           (50 KB)  - Device tree blobs
```

---

## Image Contents

### What's Inside `sdcard.img`

The final image is a **complete bootable SD card image** containing:

#### Partition 1: Boot (FAT32, 32MB)
```
/boot/
├── bcm2711-rpi-4-b.dtb        # Device tree (hardware description)
├── config.txt                 # Pi firmware config
├── cmdline.txt                # Kernel boot parameters
├── start4.elf                 # GPU firmware (basic)
├── start4x.elf                # GPU firmware with camera support (REQUIRED for camera!)
├── fixup4.dat                 # GPU memory split config
├── fixup4x.dat                # GPU memory split for camera
├── Image                      # Linux kernel (compressed)
└── overlays/                  # Device tree overlays
    ├── ov5647.dtbo           # Camera overlay
    ├── piscreen.dtbo         # 3.5" ILI9486 LCD overlay (built-in)
    ├── vc4-kms-v3d.dtbo      # Graphics driver
    └── ...
```

> **⚠️ Camera Requirement**: To use `start_x=1` in config.txt (required for camera),
> you MUST have `start4x.elf` and `fixup4x.dat`. The basic `start4.elf` does NOT
> support the camera interface. Enable `BR2_PACKAGE_RPI_FIRMWARE_X=y` in Buildroot
> to include these files.

#### Partition 2: Root Filesystem (ext4, 256MB)
```
/
├── bin/           # Essential binaries (busybox symlinks)
├── boot/          # (empty on Pi, firmware is in partition 1)
├── dev/           # Device nodes (dynamically created)
├── etc/           # System configuration
│   ├── inittab               # Init system config
│   ├── passwd                # User accounts
│   ├── shadow                # Encrypted passwords
│   ├── hostname              # System hostname
│   ├── fstab                 # Filesystem mounts
│   └── network/
│       └── interfaces        # Network configuration
├── home/          # User home directories (none by default)
├── lib/           # Shared libraries (glibc, etc.)
├── media/         # Mount points for removable media
├── mnt/           # Temporary mount points
├── opt/           # Optional software (LeafSense here!)
│   └── leafsense/
│       ├── LeafSense         # Main application binary
│       ├── leafsense_model.onnx
│       ├── gallery/          # Captured images
│       └── database/         # SQLite database
├── proc/          # Process information (virtual)
├── root/          # Root user home directory
├── run/           # Runtime data (PID files, sockets)
├── sbin/          # System administration binaries
├── sys/           # System information (virtual)
├── tmp/           # Temporary files (cleared on boot)
├── usr/           # User programs and libraries
│   ├── bin/                  # User binaries
│   │   ├── vcgencmd         # VideoCore command
│   │   ├── v4l2-ctl         # V4L2 control utility
│   │   └── ...
│   ├── lib/                  # User libraries
│   │   ├── libopencv_*.so   # OpenCV libraries
│   │   ├── libQt5*.so       # Qt5 libraries
│   │   └── ...
│   └── share/                # Shared data
└── var/           # Variable data
    ├── log/                  # Log files
    │   └── leafsense.log
    └── tmp/                  # Temporary files
```

### Size Breakdown

```
Component               Size      Description
────────────────────────────────────────────────────────────
Boot partition          32 MB     Firmware, kernel, DTBs
Root filesystem        256 MB     OS + packages
    ├─ Kernel           24 MB     Linux 6.12.41
    ├─ Libraries       120 MB     glibc, Qt5, OpenCV
    ├─ Binaries         40 MB     BusyBox, utilities
    ├─ System files     10 MB     Config, device nodes
    └─ Free space       62 MB     For logs, captures
────────────────────────────────────────────────────────────
Total Image Size       288 MB     (289 MB on disk)
```

### Libraries Included

```bash
# Core system libraries
/usr/lib/
├── libgcc_s.so.1              # GCC runtime
├── libstdc++.so.6             # C++ standard library
├── ld-linux-aarch64.so.1      # Dynamic linker
└── libc.so.6                  # GNU C library

# Graphics & GUI
├── libQt5Core.so.5
├── libQt5Gui.so.5
├── libQt5Widgets.so.5
└── libQt5Sql.so.5

# Computer vision
├── libopencv_core.so.411
├── libopencv_imgproc.so.411
├── libopencv_videoio.so.411
├── libopencv_imgcodecs.so.411
└── libopencv_dnn.so.411

# Database
└── libsqlite3.so.0

# ML inference (external)
/opt/leafsense/external/
└── libonnxruntime.so.1.16.3
```

---

## Customization

### Adding New Packages

**Example: Add Python 3**

1. Open menuconfig:
   ```bash
   cd /home/daniel/buildroot/buildroot-2025.08
   make menuconfig
   ```

2. Navigate to:
   ```
   Target packages
     → Interpreter languages and scripting
       → [*] python3
   ```

3. Save and rebuild:
   ```bash
   make
   ```

### Modifying Kernel Configuration

**Example: Enable Bluetooth**

1. Edit kernel config fragment:
   ```bash
   nano board/raspberrypi/linux-custom.config
   ```

2. Add:
   ```
   CONFIG_BT=y
   CONFIG_BT_HCIUART=y
   CONFIG_BT_HCIUART_BCM=y
   ```

3. Rebuild kernel:
   ```bash
   make linux-rebuild
   make
   ```

### Changing Root Filesystem Size

**Current**: 256 MB  
**To increase** (e.g., for more gallery images):

1. Edit `.config`:
   ```bash
   cd /home/daniel/buildroot/buildroot-2025.08
   nano .config
   ```

2. Find and change:
   ```
   BR2_TARGET_ROOTFS_EXT2_SIZE="256M"
   ```
   To:
   ```
   BR2_TARGET_ROOTFS_EXT2_SIZE="512M"
   ```

3. Rebuild filesystem:
   ```bash
   make
   ```

### Adding Custom Files

**Use post-build script:**

```bash
# Edit board/leafsense/post-build.sh
nano board/leafsense/post-build.sh
```

Add:
```bash
# Copy custom config
cp /path/to/custom.conf ${TARGET_DIR}/etc/

# Create directories
mkdir -p ${TARGET_DIR}/opt/myapp

# Set execute permission
chmod +x ${TARGET_DIR}/opt/myapp/start.sh
```

---

## Troubleshooting Build Issues

### Build Fails with "Permission Denied"

**Symptom**: `tar: Cannot open: Permission denied`

**Solution**:
```bash
# Ensure output directory is writable
chmod -R u+w output/

# Or clean and rebuild
make clean
make
```

### "No rule to make target"

**Symptom**: `make: *** No rule to make target 'menuconfig'. Stop.`

**Solution**: You're not in the Buildroot directory
```bash
cd /home/daniel/buildroot/buildroot-2025.08
make menuconfig
```

### Package Download Fails

**Symptom**: `wget: unable to resolve host address`

**Solution**:
```bash
# Check internet connection
ping google.com

# Or use local mirror
make BR2_DL_DIR=/path/to/downloads
```

### Kernel Module Not Loading

**Symptom**: `modprobe: module ov5647 not found`

**Check**:
1. Module was built:
   ```bash
   find output/target/lib/modules/ -name "ov5647.ko"
   ```

2. Module is installed:
   ```bash
   ssh root@10.42.0.196 'ls /lib/modules/*/kernel/drivers/media/'
   ```

3. Dependencies loaded:
   ```bash
   ssh root@10.42.0.196 'modprobe videodev; modprobe bcm2835-unicam; modprobe ov5647'
   ```

### Image Too Large for SD Card

**Symptom**: `dd: error writing '/dev/sda': No space left on device`

**Solution**: Reduce rootfs size
```bash
# Edit .config
BR2_TARGET_ROOTFS_EXT2_SIZE="200M"

# Or remove unnecessary packages
make menuconfig
# Deselect unused packages
```

---

## Updating the Image

### Incremental Updates (Recommended)

**When to use**: After adding packages or changing config

```bash
cd /home/daniel/buildroot/buildroot-2025.08

# Update config
make menuconfig
# Make changes

# Rebuild (only rebuilds changed packages)
make

# Flash new image
sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress
```

**Time**: 5-10 minutes (vs 2-4 hours for full rebuild)

### Full Rebuild (Nuclear Option)

**When to use**: After major changes or when incremental fails

```bash
cd /home/daniel/buildroot/buildroot-2025.08

# Remove all build artifacts
make clean

# Or completely remove output directory
rm -rf output/

# Rebuild everything from scratch
make
```

**Time**: 2-4 hours

### Updating Packages

**Example: Update OpenCV from 4.11.0 to 4.12.0**

1. Edit package version:
   ```bash
   nano package/opencv4/opencv4.mk
   ```

2. Change version:
   ```
   OPENCV4_VERSION = 4.12.0
   ```

3. Update hash:
   ```bash
   # Download new version
   wget https://github.com/opencv/opencv/archive/4.12.0.tar.gz
   sha256sum 4.12.0.tar.gz
   
   # Update hash in opencv4.hash
   nano package/opencv4/opencv4.hash
   ```

4. Rebuild:
   ```bash
   make opencv4-dirclean
   make opencv4
   make
   ```

### Saving Custom Configuration

**Save current config as defconfig:**
```bash
cd /home/daniel/buildroot/buildroot-2025.08

# Save to configs/
make savedefconfig BR2_DEFCONFIG=configs/leafsense_defconfig

# Copy to project
cp configs/leafsense_defconfig \
   ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/deploy/
```

**Restore from saved defconfig:**
```bash
cd /home/daniel/buildroot/buildroot-2025.08
make leafsense_defconfig
```

---

## Summary

### Key Points

1. **Buildroot** creates a minimal, customized Linux system for LeafSense
2. **`.config` file** (5000+ lines) defines everything in the image
3. **Camera support** added via kernel config fragment and rpi-userland package
4. **Final image** (`sdcard.img`) is 289 MB with 2 partitions
5. **Build time**: 2-4 hours initially, 5-10 minutes for updates
6. **Fully reproducible**: Same config → Same image

### Quick Reference

```bash
# Build from scratch
cd /home/daniel/buildroot/buildroot-2025.08
make raspberrypi4_64_defconfig
make

# Enable camera
~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project/deploy/enable-camera.sh

# Rebuild
make

# Configure
make menuconfig

# Flash
sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress

# Clean
make clean
```

### Files to Backup

```
✅ .config                          # Current configuration
✅ board/leafsense/post-build.sh    # Custom build script
✅ board/raspberrypi/linux-camera.config  # Kernel camera config
✅ output/images/sdcard.img         # Final image
```

### Related Documentation

- [17-TERMINOLOGY.md](17-TERMINOLOGY.md) - Technical term definitions
- [03-MACHINE-LEARNING.md](03-MACHINE-LEARNING.md) - ML model deployment
- [06-RASPBERRY-PI-DEPLOYMENT.md](06-RASPBERRY-PI-DEPLOYMENT.md) - Deploying LeafSense
- [10-TROUBLESHOOTING.md](10-TROUBLESHOOTING.md) - Common issues
- [deploy/CAMERA_SETUP_GUIDE.md](../deploy/CAMERA_SETUP_GUIDE.md) - Camera troubleshooting

---

**Document Version**: 1.4.0  
**Buildroot Version**: 2025.08  
**Last Build**: January 10, 2026  
**Image Size**: 289 MB
