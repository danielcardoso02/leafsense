#!/bin/bash
#
# LeafSense - Buildroot Configuration Script
# This script helps configure Buildroot with all required packages
#

set -e

BUILDROOT_DIR="$HOME/buildroot/buildroot-2025.08"
LEAFSENSE_DIR="$(dirname "$(realpath "$0")")/.."

echo "==========================================="
echo "  LeafSense - Buildroot Configuration"
echo "==========================================="
echo ""

# Check if Buildroot exists
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "ERROR: Buildroot not found at $BUILDROOT_DIR"
    echo "Please install Buildroot first."
    exit 1
fi

cd "$BUILDROOT_DIR"

echo "[1/5] Checking current configuration..."
echo ""

# Check if already configured for Raspberry Pi 4
if grep -q "raspberrypi4_64_defconfig" .config 2>/dev/null; then
    echo "✓ Already configured for Raspberry Pi 4 (64-bit)"
else
    echo "Configuring for Raspberry Pi 4 (64-bit)..."
    make raspberrypi4_64_defconfig
fi

echo ""
echo "[2/5] Creating Buildroot fragment for LeafSense packages..."

# Create a config fragment with our required packages
cat > "$BUILDROOT_DIR/leafsense_config.fragment" << 'EOF'
# =========================================================
# LeafSense Buildroot Configuration Fragment
# =========================================================
# Version: 1.5.0 - January 10, 2026
# =========================================================

# ============================================
# SYSTEM CONFIGURATION
# ============================================
BR2_TARGET_GENERIC_ROOT_PASSWD="leafsense"
BR2_TARGET_GENERIC_HOSTNAME="leafsense-pi"
BR2_TARGET_TZ_INFO=y
BR2_TARGET_LOCALTIME="Europe/Lisbon"

# Device management (udev for proper device nodes)
BR2_PACKAGE_EUDEV=y
BR2_PACKAGE_EUDEV_RULES_GEN=y
BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_EUDEV=y

# ============================================
# Qt5 (GUI FRAMEWORK)
# ============================================
BR2_PACKAGE_QT5=y
BR2_PACKAGE_QT5BASE=y
BR2_PACKAGE_QT5BASE_WIDGETS=y
BR2_PACKAGE_QT5BASE_GUI=y
BR2_PACKAGE_QT5BASE_PNG=y
BR2_PACKAGE_QT5BASE_SQLITE_QT=y
BR2_PACKAGE_QT5BASE_FONTCONFIG=y
BR2_PACKAGE_QT5BASE_HARFBUZZ=y
BR2_PACKAGE_QT5BASE_GIF=y
BR2_PACKAGE_QT5BASE_JPEG=y
BR2_PACKAGE_QT5SVG=y
BR2_PACKAGE_QT5CHARTS=y
# NOTE: We use evdev for touchscreen, NOT tslib (tslib causes freezing)
# BR2_PACKAGE_QT5BASE_TSLIB=y  # DISABLED - causes application freeze

# ============================================
# TOUCHSCREEN (evdev - NOT tslib!)
# ============================================
# CRITICAL: tslib causes application freezing on Waveshare 3.5" LCD
# The Qt evdev touchscreen plugin handles rotation via environment variable:
#   QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
# This MUST match the display rotation in config.txt
BR2_PACKAGE_LIBEVDEV=y
BR2_PACKAGE_EVTEST=y
BR2_PACKAGE_INPUT_EVENT_DAEMON=y

# ============================================
# DATABASE (SQLite)
# ============================================
BR2_PACKAGE_SQLITE=y

# ============================================
# CAMERA (Pi Camera v1/v2 + libcamera)
# ============================================
# libcamera is required for Pi Camera hardware
BR2_PACKAGE_LIBCAMERA=y
BR2_PACKAGE_LIBCAMERA_V4L2=y
BR2_PACKAGE_LIBCAMERA_PIPELINE_RPI_VC4=y

# V4L2 camera tools and libraries
BR2_PACKAGE_V4L_UTILS=y
BR2_PACKAGE_LIBV4L=y
BR2_PACKAGE_MEDIA_CTL=y

# OpenCV for image processing
BR2_PACKAGE_OPENCV4=y
BR2_PACKAGE_OPENCV4_LIB_IMGPROC=y
BR2_PACKAGE_OPENCV4_LIB_IMGCODECS=y
BR2_PACKAGE_OPENCV4_LIB_HIGHGUI=y
BR2_PACKAGE_OPENCV4_LIB_VIDEOIO=y
BR2_PACKAGE_OPENCV4_WITH_JPEG=y
BR2_PACKAGE_OPENCV4_WITH_PNG=y
BR2_PACKAGE_OPENCV4_WITH_V4L=y

# ============================================
# HARDWARE INTERFACES (I2C, SPI, GPIO)
# ============================================
BR2_PACKAGE_RPI_USERLAND=y
BR2_PACKAGE_I2C_TOOLS=y
BR2_PACKAGE_LIBGPIOD=y
BR2_PACKAGE_LIBGPIOD_TOOLS=y

# ============================================
# NETWORKING
# ============================================
# SSH server for remote access
BR2_PACKAGE_DROPBEAR=y
BR2_PACKAGE_DROPBEAR_CLIENT=y

# Network configuration
BR2_PACKAGE_DHCPCD=y
BR2_PACKAGE_IPROUTE2=y
BR2_PACKAGE_IPTABLES=y

# Network utilities
BR2_PACKAGE_WGET=y
BR2_PACKAGE_CURL=y
BR2_PACKAGE_NTP=y
BR2_PACKAGE_NTPDATE=y

# WiFi support (future use)
BR2_PACKAGE_WPA_SUPPLICANT=y
BR2_PACKAGE_WIRELESS_TOOLS=y
BR2_PACKAGE_IW=y

# ============================================
# FILESYSTEM TOOLS
# ============================================
BR2_PACKAGE_E2FSPROGS=y
BR2_PACKAGE_DOSFSTOOLS=y
BR2_PACKAGE_COREUTILS=y
BR2_PACKAGE_TAR=y
BR2_PACKAGE_GZIP=y
BR2_PACKAGE_BZIP2=y

# ============================================
# SYSTEM UTILITIES
# ============================================
BR2_PACKAGE_HTOP=y
BR2_PACKAGE_NANO=y
BR2_PACKAGE_FILE=y
BR2_PACKAGE_PROCPS_NG=y
BR2_PACKAGE_UTIL_LINUX=y
BR2_PACKAGE_UTIL_LINUX_BINARIES=y
BR2_PACKAGE_KMOD=y
BR2_PACKAGE_KMOD_TOOLS=y

# ============================================
# DEBUGGING TOOLS (can be disabled for production)
# ============================================
BR2_PACKAGE_STRACE=y
BR2_PACKAGE_LSOF=y
BR2_PACKAGE_TCPDUMP=y
BR2_PACKAGE_GDB=y
BR2_PACKAGE_LTRACE=y

# ============================================
# FONTS (required for Qt5 GUI)
# ============================================
BR2_PACKAGE_DEJAVU=y
BR2_PACKAGE_FONTCONFIG=y

# ============================================
# IMAGE LIBRARIES
# ============================================
BR2_PACKAGE_JPEG=y
BR2_PACKAGE_LIBPNG=y
BR2_PACKAGE_TIFF=y

# ============================================
# SCRIPTING & AUTOMATION
# ============================================
BR2_PACKAGE_BASH=y
BR2_PACKAGE_JQ=y

# ============================================
# FUTURE-PROOFING (commonly needed packages)
# ============================================
# Python (useful for scripts and ML tools)
BR2_PACKAGE_PYTHON3=y
BR2_PACKAGE_PYTHON3_SSL=y
BR2_PACKAGE_PYTHON3_SQLITE=y
BR2_PACKAGE_PYTHON_PIP=y

# USB mass storage support
BR2_PACKAGE_USBUTILS=y

# Serial communication (for sensors)
BR2_PACKAGE_PICOCOM=y

# Real-time clock support
BR2_PACKAGE_HWCLOCK=y

# System logging
BR2_PACKAGE_SYSLOG_NG=y
EOF

echo "✓ Configuration fragment created"
echo ""

echo "[3/5] Merging configuration..."
# Merge the fragment with current config
support/kconfig/merge_config.sh .config leafsense_config.fragment

echo ""
echo "✓ Configuration merged"
echo ""

echo "[4/5] Creating post-build script with touchscreen configuration..."

# Create overlay directory structure
mkdir -p "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/etc/init.d"
mkdir -p "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/etc/profile.d"
mkdir -p "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/opt/leafsense/gallery"

# Copy overlay files from project
if [ -d "$LEAFSENSE_DIR/deploy/rootfs-overlay" ]; then
    cp -r "$LEAFSENSE_DIR/deploy/rootfs-overlay/"* "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/"
    echo "✓ Copied rootfs overlay files (including touchscreen configuration)"
fi

# Create post-build script
cat > "$BUILDROOT_DIR/board/leafsense/post-build.sh" << 'POSTBUILD'
#!/bin/bash
# LeafSense post-build script
# ===========================
# This script configures the boot partition and root filesystem
# for LeafSense with Waveshare 3.5" LCD (C) touchscreen.
#
# CRITICAL: Touchscreen configuration is handled via evdev, not tslib.
# No calibration is required - the rotate parameter handles orientation.

LEAFSENSE_DEPLOY="${BR2_EXTERNAL:-$BUILD_DIR/../..}/deploy"

# ============================================
# BOOT PARTITION CONFIGURATION (config.txt)
# ============================================
CONFIG_TXT="$BINARIES_DIR/rpi-firmware/config.txt"

if [ -f "$CONFIG_TXT" ]; then
    # Replace config.txt with our complete configuration
    if [ -f "$LEAFSENSE_DEPLOY/boot-overlay/config.txt" ]; then
        cp "$LEAFSENSE_DEPLOY/boot-overlay/config.txt" "$CONFIG_TXT"
        echo "Replaced config.txt with LeafSense configuration"
    else
        # Fallback: append our configuration if overlay not found
        if ! grep -q "# LeafSense Hardware Configuration" "$CONFIG_TXT"; then
            cat >> "$CONFIG_TXT" << 'HWCONFIG'

# LeafSense Hardware Configuration
# ================================

# Enable I2C (for ADC ADS1115 and RTC DS3231)
dtparam=i2c_arm=on
dtparam=i2c1=on

# Enable 1-Wire for DS18B20 temperature sensor (GPIO 19)
dtoverlay=w1-gpio,gpiopin=19

# Enable SPI (required for display and touchscreen)
dtparam=spi=on

# USB Gadget Mode
dtoverlay=dwc2

# ============================================
# WAVESHARE 3.5" LCD (C) - CRITICAL SETTINGS
# ============================================
# speed=16000000 (16MHz) prevents touch freeze
# fps=50 reduces screen blinking
# rotate=90 sets landscape orientation
dtoverlay=waveshare35c:rotate=90,speed=16000000,fps=50

hdmi_force_hotplug=1
hdmi_cvt=480 320 60 6 0 0 0
hdmi_group=2
hdmi_mode=87
hdmi_drive=2

framebuffer_width=480
framebuffer_height=320

# Camera CSI
start_x=1
gpu_mem=256
camera_auto_detect=1
dtoverlay=vc4-kms-v3d
HWCONFIG
            echo "Added LeafSense hardware configuration to config.txt"
        fi
    fi
fi

# ============================================
# COPY WAVESHARE DISPLAY OVERLAY
# ============================================
if [ -f "$LEAFSENSE_DEPLOY/boot-overlay/overlays/waveshare35c.dtbo" ]; then
    mkdir -p "$BINARIES_DIR/rpi-firmware/overlays"
    cp "$LEAFSENSE_DEPLOY/boot-overlay/overlays/waveshare35c.dtbo" "$BINARIES_DIR/rpi-firmware/overlays/"
    echo "Copied waveshare35c.dtbo overlay"
elif [ -f "$LEAFSENSE_DEPLOY/waveshare35c.dtbo" ]; then
    mkdir -p "$BINARIES_DIR/rpi-firmware/overlays"
    cp "$LEAFSENSE_DEPLOY/waveshare35c.dtbo" "$BINARIES_DIR/rpi-firmware/overlays/"
    echo "Copied waveshare35c.dtbo overlay from deploy directory"
fi

# ============================================
# ROOT FILESYSTEM CONFIGURATION
# ============================================

# Create LeafSense directories on target
mkdir -p "$TARGET_DIR/opt/leafsense/gallery"
mkdir -p "$TARGET_DIR/opt/leafsense/models"
mkdir -p "$TARGET_DIR/opt/leafsense/data"
mkdir -p "$TARGET_DIR/opt/leafsense/images"
mkdir -p "$TARGET_DIR/var/log"

# ============================================
# TOUCHSCREEN ENVIRONMENT CONFIGURATION
# ============================================
# Create Qt/evdev touchscreen environment file
# This is CRITICAL for touchscreen to work without calibration
cat > "$TARGET_DIR/etc/profile.d/leafsense-qt.sh" << 'QTENV'
#!/bin/sh
# LeafSense Qt5 and Touchscreen Environment
# CRITICAL: rotate=90 must match config.txt display rotation
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FONTDIR=/usr/share/fonts
export LEAFSENSE_HOME=/opt/leafsense
export LEAFSENSE_DB=/opt/leafsense/leafsense.db
QTENV
chmod 644 "$TARGET_DIR/etc/profile.d/leafsense-qt.sh"

# ============================================
# AUTO-START INIT SCRIPT
# ============================================
cat > "$TARGET_DIR/etc/init.d/S99leafsense" << 'INITSCRIPT'
#!/bin/sh
DAEMON=/opt/leafsense/LeafSense
PIDFILE=/var/run/leafsense.pid
LOGFILE=/var/log/leafsense.log

case "$1" in
    start)
        echo "Starting LeafSense..."
        [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE) 2>/dev/null && exit 0
        sleep 2
        mkdir -p /opt/leafsense/gallery
        export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
        export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
        export QT_QPA_FB_HIDECURSOR=1
        cd /opt/leafsense
        ./LeafSense -platform linuxfb:fb=/dev/fb1 >> $LOGFILE 2>&1 &
        echo $! > $PIDFILE
        ;;
    stop)
        [ -f $PIDFILE ] && kill $(cat $PIDFILE) 2>/dev/null && rm -f $PIDFILE
        killall LeafSense 2>/dev/null
        ;;
    restart) $0 stop; sleep 2; $0 start ;;
    *) echo "Usage: $0 {start|stop|restart}" ;;
esac
INITSCRIPT
chmod 755 "$TARGET_DIR/etc/init.d/S99leafsense"

# ============================================
# STARTUP SCRIPT
# ============================================
cat > "$TARGET_DIR/opt/leafsense/start_leafsense.sh" << 'STARTUP'
#!/bin/sh
# LeafSense Startup Script
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
export QT_QPA_FB_HIDECURSOR=1
export LEAFSENSE_DB="/opt/leafsense/leafsense.db"
cd /opt/leafsense
mkdir -p /opt/leafsense/gallery
exec ./LeafSense -platform linuxfb:fb=/dev/fb1
STARTUP
chmod 755 "$TARGET_DIR/opt/leafsense/start_leafsense.sh"

# Set permissions
chmod 755 "$TARGET_DIR/opt/leafsense"

echo ""
echo "============================================"
echo "LeafSense post-build completed successfully"
echo "============================================"
echo "Touchscreen: evdev with rotate=90 (no calibration needed)"
echo "Display: Waveshare 3.5 LCD (C) at 16MHz, 50fps"
echo "============================================"
POSTBUILD

chmod +x "$BUILDROOT_DIR/board/leafsense/post-build.sh"

echo "✓ Post-build script created with touchscreen configuration"
echo ""

echo "[5/5] Updating main configuration with post-build script..."

# Add post-build script to config if not already there
if ! grep -q "board/leafsense/post-build.sh" .config; then
    sed -i 's|^BR2_ROOTFS_POST_BUILD_SCRIPT=.*|BR2_ROOTFS_POST_BUILD_SCRIPT="board/raspberrypi4-64/post-build.sh board/leafsense/post-build.sh"|' .config
fi

echo ""
echo "==========================================="
echo "  Configuration Complete!"
echo "==========================================="
echo ""
echo "Next steps:"
echo ""
echo "  1. (OPTIONAL) Add LeafSense LED driver package:"
echo "     cd $BUILDROOT_DIR"
echo "     # Set up external packages"
echo "     export BR2_EXTERNAL=$LEAFSENSE_DIR/deploy/buildroot-packages"
echo "     make menuconfig"
echo "     # Navigate: External options -> LeafSense packages -> leafsense-led"
echo ""
echo "  2. Review configuration (optional):"
echo "     cd $BUILDROOT_DIR"
echo "     make menuconfig"
echo ""
echo "  3. Configure kernel for I2C/1-Wire/Camera:"
echo "     make linux-menuconfig"
echo ""
echo "  4. Build the image (takes 1-3 hours):"
echo "     make -j\$(nproc)"
echo ""
echo "  5. Flash to SD card:"
echo "     sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress"
echo ""
echo "  6. SSH access (after boot):"
echo "     ssh root@<PI_IP>  (password: leafsense)"
echo ""
echo "==========================================="
