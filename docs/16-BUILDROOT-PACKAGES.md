<h1 align="center">LeafSense - Buildroot Package Reference</h1>

<p align="center">
<strong>Version:</strong> 1.5.1 &nbsp;|&nbsp; <strong>Last Updated:</strong> January 19, 2026
</p>

---

<p align="center"><em>Complete reference of all packages included in the LeafSense Buildroot image</em></p>

### SD Card Partitions

| Partition | Label | Format | Size | Contents |
|-----------|-------|--------|------|----------|
| 1 | BOOT | FAT32 | 64 MB | Kernel, DTB, firmware |
| 2 | ROOTFS | ext4 | 512 MB | Root filesystem |

---

## Package Categories

### 1. Qt5 GUI Framework

| Package | Purpose |
|---------|---------|
| `qt5` | Qt5 framework base |
| `qt5base` | Core Qt5 libraries |
| `qt5base-widgets` | GUI widgets |
| `qt5base-gui` | Graphics support |
| `qt5base-png` | PNG image support |
| `qt5base-gif` | GIF image support |
| `qt5base-jpeg` | JPEG image support |
| `qt5base-sqlite` | SQLite integration |
| `qt5base-fontconfig` | Font configuration |
| `qt5base-harfbuzz` | Text shaping engine |
| `qt5svg` | SVG support |
| `qt5charts` | Charts and graphs |

**Note:** `qt5base-tslib` is intentionally **NOT included** - it causes application freezing with the Waveshare 3.5" LCD. Use evdev instead.

---

### 2. Touchscreen (evdev)

| Package | Purpose |
|---------|---------|
| `libevdev` | Event device library |
| `evtest` | Touchscreen testing tool |
| `input-event-daemon` | Input event handling |

**Critical Configuration:**
```bash
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
```

The rotation parameters configure correct touch mapping for the display (works with piscreen,rotate=270).

---

### 3. Database

| Package | Purpose |
|---------|---------|
| `sqlite` | SQLite database engine |

**Usage:** LeafSense stores all data in `/opt/leafsense/leafsense.db`.

---

### 4. Camera System

| Package | Purpose |
|---------|---------|
| `libcamera` | Modern camera support framework |
| `libcamera-v4l2` | V4L2 compatibility layer |
| `libcamera-pipeline-rpi-vc4` | Raspberry Pi camera pipeline |
| `v4l-utils` | Video4Linux utilities |
| `libv4l` | V4L2 library |
| `media-ctl` | Media device control |
| `opencv4` | Computer vision library |
| `opencv4-lib-imgproc` | Image processing |
| `opencv4-lib-imgcodecs` | Image encoding/decoding |
| `opencv4-lib-highgui` | GUI for display |
| `opencv4-lib-videoio` | Video I/O |
| `opencv4-with-jpeg` | JPEG support in OpenCV |
| `opencv4-with-png` | PNG support in OpenCV |
| `opencv4-with-v4l` | V4L2 support in OpenCV |

**Camera Status:**
- Hardware: Pi Camera v1 (OV5647)
- Interface: libcamera (via vc4 pipeline)
- Integration: Pending final testing

---

### 5. Hardware Interfaces

| Package | Purpose |
|---------|---------|
| `rpi-userland` | VideoCore libraries (vcgencmd, etc.) |
| `i2c-tools` | I2C debugging and control |
| `libgpiod` | GPIO library |
| `libgpiod-tools` | GPIO utilities (gpioset, gpioget) |

**Hardware Used:**
- I2C: ADS1115 ADC, DS3231 RTC
- GPIO: LED control, relay control
- SPI: Display, touchscreen

---

### 6. LeafSense LED Driver (Kernel Module)

| Package | Purpose |
|---------|--------|
| `leafsense-led` | Custom kernel module for LED status indicator |

**Features:**
- Character device at `/dev/led0`
- Automatic permission setup via udev
- Auto-loads at boot via `/etc/modules`

**Usage:**
```bash
# Check module is loaded
lsmod | grep led

# Control LED (from userspace application)
echo "1" > /dev/led0  # Turn on
echo "0" > /dev/led0  # Turn off
```

**Files Installed:**
- `/lib/modules/*/updates/led.ko.xz` - Kernel module
- `/etc/udev/rules.d/99-leddev.rules` - Permission rules
- `/etc/modules` - Auto-load configuration

---

### 7. Networking

| Package | Purpose |
|---------|---------|
| `dropbear` | Lightweight SSH server |
| `dropbear-client` | SSH client |
| `dhcpcd` | DHCP client |
| `iproute2` | Network configuration |
| `iptables` | Firewall |
| `wget` | HTTP downloads |
| `curl` | URL transfers |
| `ntp` | Network time protocol daemon |
| `ntpdate` | NTP date sync |
| `wpa_supplicant` | WiFi WPA authentication |
| `wireless-tools` | WiFi utilities |
| `iw` | WiFi configuration |

**Default Access:**
```bash
ssh root@<PI_IP>
# Password: leafsense
```

---

### 8. Filesystem Tools

| Package | Purpose |
|---------|--------|
| `e2fsprogs` | ext2/3/4 filesystem tools |
| `dosfstools` | FAT filesystem tools |
| `coreutils` | Standard Unix utilities |
| `tar` | Archive utility |
| `gzip` | Compression |
| `bzip2` | Compression |

---

### 9. System Utilities

| Package | Purpose |
|---------|---------|
| `htop` | Interactive process viewer |
| `nano` | Text editor |
| `file` | File type identification |
| `procps-ng` | Process utilities (ps, top, etc.) |
| `util-linux` | System utilities |
| `kmod` | Kernel module tools |
| `bash` | Bash shell |
| `jq` | JSON processor |

---

### 10. Debugging Tools

| Package | Purpose |
|---------|--------|
| `strace` | System call tracer |
| `lsof` | List open files |
| `tcpdump` | Network packet analyzer |
| `gdb` | GNU debugger |
| `ltrace` | Library call tracer |

**Note:** These can be removed for production builds to reduce image size.

---

### 11. Fonts

| Package | Purpose |
|---------|---------|
| `dejavu` | DejaVu font family |
| `fontconfig` | Font configuration |

---

### 12. Image Libraries

| Package | Purpose |
|---------|--------|
| `jpeg` | JPEG library |
| `libpng` | PNG library |
| `tiff` | TIFF library |

---

### 13. Future-Proofing Packages

| Package | Purpose |
|---------|---------|
| `python3` | Python interpreter |
| `python3-ssl` | SSL support for Python |
| `python3-sqlite` | SQLite support for Python |
| `python-pip` | Python package manager |
| `usbutils` | USB utilities (lsusb) |
| `picocom` | Serial terminal |
| `hwclock` | Hardware clock utilities |
| `syslog-ng` | System logging |

---

## Device Management

| Package | Purpose |
|---------|---------|
| `eudev` | Device manager (udev) |
| `eudev-rules-gen` | Automatic rule generation |

**Configuration:** `BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_EUDEV=y`

---

## System Configuration

| Setting | Value |
|---------|-------|
| Root Password | `leafsense` |
| Hostname | `leafsense-pi` |
| Timezone | `Europe/Lisbon` |

---

## Not Included (By Design)

| Package | Reason |
|---------|--------|
| `tslib` | Causes application freezing with Waveshare 3.5" LCD |
| `systemd` | Too heavy for embedded system; BusyBox init used |
| `xorg` | Not needed; using framebuffer directly |
| `wayland` | Not needed; using framebuffer directly |

---

## Adding Custom Packages

To add packages to future builds:

1. Edit `deploy/configure-buildroot.sh`
2. Add package to the fragment section
3. Run the configuration script
4. Rebuild the image

```bash
cd ~/buildroot/buildroot-2025.08
./path/to/leafsense-project/deploy/configure-buildroot.sh
make -j$(nproc)
```

---

## Image Size Considerations

**Current image size:** ~300-400 MB

To reduce image size for production:
1. Remove debugging tools (strace, gdb, ltrace, tcpdump, lsof)
2. Remove Python if not needed
3. Use `BR2_ENABLE_DEBUG=n`
4. Strip binaries: `BR2_STRIP_strip=y`

---

## Configuration Script

The complete Buildroot configuration is managed by:

```
deploy/configure-buildroot.sh
```

This script:
1. Creates the configuration fragment
2. Merges with base Raspberry Pi 4 config
3. Sets up post-build hooks for touchscreen
4. Creates overlay directories

---

## Overlay Structure

Files automatically installed to the image:

```
deploy/
├── boot-overlay/
│   ├── config.txt              # Boot configuration (includes piscreen overlay)
│   └── cmdline.txt             # Kernel command line
│
└── rootfs-overlay/
    ├── etc/
    │   ├── init.d/
    │   │   └── S99leafsense    # Auto-start script
    │   └── profile.d/
    │       └── leafsense-qt.sh # Qt environment (evdev touchscreen config)
    └── opt/
        └── leafsense/
            └── start.sh
```

**Note:** The piscreen overlay is built into the Linux kernel, so no separate .dtbo file needs to be deployed.

---

## Building the Complete Image

```bash
# 1. Configure Buildroot
cd ~/buildroot/buildroot-2025.08
/path/to/leafsense-project/deploy/configure-buildroot.sh

# 2. Build (1-3 hours first time)
make -j$(nproc)

# 3. Flash to SD card
sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress
sync

# Partitions will be labeled:
#   - BOOT (FAT32, 64MB)
#   - ROOTFS (ext4, 512MB)
```

---

## Verification

After flashing, verify packages:

```bash
# SSH to Pi
ssh root@<PI_IP>

# Check Qt5
qmake --version

# Check SQLite
sqlite3 --version

# Check Python
python3 --version

# Check camera tools
libcamera-hello --list-cameras

# Check I2C
i2cdetect -y 1

# Check GPIO
gpioinfo
```

---

## Related Documentation

- [05-BUILDROOT-IMAGE.md](05-BUILDROOT-IMAGE.md) - Image creation details
- [15-TOUCHSCREEN-CONFIGURATION.md](15-TOUCHSCREEN-CONFIGURATION.md) - Touchscreen setup
- [06-RASPBERRY-PI-DEPLOYMENT.md](06-RASPBERRY-PI-DEPLOYMENT.md) - Deployment guide
