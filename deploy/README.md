# LeafSense Deployment Guide

Guide for deploying LeafSense on Raspberry Pi 4 with Waveshare 3.5" LCD (C) touchscreen.

---

## Hardware Configuration

| Component | Model | Details |
|-----------|-------|---------|
| Board | Raspberry Pi 4 Model B | 2GB+ RAM |
| Display | Waveshare 3.5" LCD (C) | ILI9486 controller, 480x320 |
| Touch | ADS7846 | Resistive touchscreen |
| Interface | SPI | Display CS0, Touch CS1 |

---

## Quick Start

### 1. Flash SD Card

```bash
sudo dd if=sdcard.img of=/dev/sdX bs=4M status=progress && sync
```

### 2. Deploy Application

```bash
# Mount root partition
sudo mount /dev/sdX2 /mnt

# Copy files
sudo mkdir -p /mnt/opt/leafsense
sudo cp build-arm64/src/LeafSense /mnt/opt/leafsense/
sudo cp ml/leafsense_model.onnx /mnt/opt/leafsense/
sudo cp ml/leafsense_model_classes.txt /mnt/opt/leafsense/
sudo cp database/schema.sql /mnt/opt/leafsense/
sudo cp external/onnxruntime-arm64/lib/libonnxruntime.so.1.16.3 /mnt/usr/lib/
sudo ln -sf libonnxruntime.so.1.16.3 /mnt/usr/lib/libonnxruntime.so

sudo umount /mnt
```

### 3. First Boot

```bash
ssh root@10.42.0.196

# Initialize database
cd /opt/leafsense
sqlite3 leafsense.db < schema.sql

# Run application
./start_leafsense.sh
```

---

## Build System

### PC Build (Development)

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
# Output: build/src/LeafSense
```

### ARM64 Build (Raspberry Pi)

```bash
mkdir -p build-arm64 && cd build-arm64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake
make -j$(nproc)
# Output: build-arm64/src/LeafSense
```

---

## Display Configuration

### config.txt Settings

```ini
dtparam=spi=on
dtoverlay=piscreen,speed=16000000,rotate=270
framebuffer_width=480
framebuffer_height=320
```

> **Note:** SPI speed of 16MHz prevents touchscreen freeze issues.

---

## Touch Configuration

Qt evdev touchscreen handler with rotation parameter:

```bash
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
./LeafSense
```

> **Important:** Do not use tslib - it causes application freezing due to SPI bus contention.

---

## Startup Script

`/opt/leafsense/start_leafsense.sh`:

```bash
#!/bin/sh
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FB_NO_LIBINPUT=1
export QT_QPA_FONTDIR=/usr/share/fonts
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"

cd /opt/leafsense
./LeafSense
```

---

## File Locations

| Path | Description |
|------|-------------|
| `/opt/leafsense/LeafSense` | Main application binary |
| `/opt/leafsense/start_leafsense.sh` | Startup script |
| `/opt/leafsense/leafsense.db` | SQLite database |
| `/opt/leafsense/leafsense_model.onnx` | ML model |
| `/opt/leafsense/gallery/` | Captured photos |
| `/var/log/leafsense.log` | Application log |
| `/etc/init.d/S99leafsense` | Auto-start init script |
| `/boot/config.txt` | Boot configuration |

---

## Network Access

| Setting | Value |
|---------|-------|
| Pi IP | 10.42.0.196 |
| SSH | `ssh root@10.42.0.196` |
| Protocol | USB Ethernet Gadget |

---

## Troubleshooting

### Display Not Working

```bash
ls /dev/fb*                          # Should show fb0 and fb1
dmesg | grep -i "fb\|ili9486\|spi"   # Check kernel messages
cat /dev/urandom > /dev/fb1          # Test display (shows static)
```

### Touch Not Responding

```bash
hexdump -C /dev/input/event0         # Should show data on touch
ls -la /dev/input/event0             # Check permissions
```

### Application Won't Start

```bash
ldd /opt/leafsense/LeafSense | grep "not found"  # Check dependencies
cat /var/log/leafsense.log | grep ERROR          # Check logs
```

---

## Scripts

| Script | Purpose |
|--------|---------|
| `setup-onnxruntime-arm64.sh` | Download ONNX Runtime for ARM64 |
| `setup-waveshare35c.sh` | Configure Waveshare display |
| `configure-buildroot.sh` | Setup Buildroot configuration |
| `toolchain-rpi4.cmake` | CMake cross-compilation toolchain |
| `screenshot_pi.sh` | Capture framebuffer screenshot |


