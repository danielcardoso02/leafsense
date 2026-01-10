#!/bin/bash
# Waveshare 3.5" LCD (C) Setup Script for LeafSense
# ==================================================
# Run this script on the Raspberry Pi to configure the display
# 
# Display: ILI9486 controller, 480x320, SPI @ 48MHz (reduced for touch stability)
# Touch: ADS7846 compatible, SPI CS1, calibrated via tslib
#
# Authors: Daniel Cardoso, Marco Costa
# Date: December 2025

set -e

echo "=== Waveshare 3.5inch LCD (C) Setup ==="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (sudo)"
    exit 1
fi

CONFIG_FILE="/boot/config.txt"
BACKUP_FILE="/boot/config.txt.backup"
OVERLAY_URL="https://files.waveshare.com/wiki/common/Waveshare35c.zip"

# Mount boot partition if needed
mkdir -p /boot
mount /dev/mmcblk0p1 /boot 2>/dev/null || true

# Backup existing config
if [ -f "$CONFIG_FILE" ]; then
    cp "$CONFIG_FILE" "$BACKUP_FILE"
    echo "Backed up config.txt to $BACKUP_FILE"
fi

# Download and install Waveshare overlay if not present
if [ ! -f "/boot/overlays/waveshare35c.dtbo" ]; then
    echo "Downloading Waveshare overlay..."
    cd /tmp
    wget -q "$OVERLAY_URL" -O Waveshare35c.zip || {
        echo "ERROR: Cannot download overlay. Please copy waveshare35c.dtbo manually."
        echo "Download from: $OVERLAY_URL"
        exit 1
    }
    unzip -o Waveshare35c.zip
    cp waveshare35c.dtbo /boot/overlays/
    echo "Installed waveshare35c.dtbo overlay"
fi

# Check if SPI is enabled
if ! grep -q "^dtparam=spi=on" "$CONFIG_FILE"; then
    echo "dtparam=spi=on" >> "$CONFIG_FILE"
    echo "Enabled SPI"
fi

# Remove old display overlays if present
sed -i '/dtoverlay=pitft35-resistive/d' "$CONFIG_FILE"
sed -i '/dtoverlay=ads7846/d' "$CONFIG_FILE"

# Add Waveshare 3.5" LCD (C) overlay configuration
cat >> "$CONFIG_FILE" << 'EOF'

# ========================================
# Waveshare 3.5" LCD (C) Configuration
# Using official Waveshare overlay (ILI9486 + ADS7846)
# SPI speed reduced to 48MHz and FPS to 20 for touch stability
# ========================================
dtoverlay=waveshare35c,speed=48000000,fps=20

# HDMI to LCD framebuffer settings
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=480 320 60 6 0 0 0
hdmi_drive=2

# Framebuffer size
framebuffer_width=480
framebuffer_height=320
EOF

echo "Added display configuration to config.txt"

# Create udev rule for touchscreen permissions
cat > /etc/udev/rules.d/99-touchscreen.rules << 'EOF'
# Touchscreen permissions for Qt applications
SUBSYSTEM=="input", KERNEL=="event*", ATTRS{name}=="ADS7846*", MODE="0666", ENV{LIBINPUT_CALIBRATION_MATRIX}="1 0 0 0 1 0"
SUBSYSTEM=="input", KERNEL=="event*", ATTRS{name}=="*touch*", MODE="0666"
EOF

echo "Created udev rules for touchscreen"

# Configure tslib for touch calibration
mkdir -p /etc/ts.conf.d
cat > /etc/ts.conf << 'EOF'
module_raw input
module pthres pmin=1
module variance delta=30
module dejitter delta=100
module linear
EOF

echo "Configured tslib"

# Set Qt environment variables for touch support
cat > /etc/profile.d/qt-touchscreen.sh << 'EOF'
# Qt5 Framebuffer settings for Waveshare 3.5" LCD
# TSLIB environment for ts_uinput calibrated touch
export TSLIB_TSDEVICE=/dev/input/event0
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_PLUGINDIR=/usr/lib/ts

# Qt5 platform settings
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS=/dev/input/event0:rotate=90
export QT_QPA_FONTDIR=/usr/share/fonts
EOF

chmod +x /etc/profile.d/qt-touchscreen.sh
echo "Configured Qt environment variables"

# Create LeafSense startup script
cat > /etc/init.d/S99leafsense << 'INITSCRIPT'
#!/bin/sh
### LeafSense Application Startup

case "$1" in
    start)
        echo "Starting LeafSense..."
        
        # Start application with Qt touchscreen plugin
        # Touchscreen requires rotate=90 for Waveshare 3.5" LCD (C)
        cd /opt/leafsense
        exec env \
            QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1 \
            QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins \
            QT_QPA_FONTDIR=/usr/share/fonts \
            QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90" \
            ./LeafSense -platform linuxfb:fb=/dev/fb1 >> /var/log/leafsense.log 2>&1 &
        ;;
    stop)
        echo "Stopping LeafSense..."
        killall LeafSense 2>/dev/null
        ;;
    restart)
        $0 stop
        sleep 2
        $0 start
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
INITSCRIPT
chmod +x /etc/init.d/S99leafsense
echo "Created LeafSense startup script"

# Create symlink rule for touchscreen device
cat >> /etc/udev/rules.d/99-touchscreen.rules << 'EOF'
# Create consistent symlink for touchscreen
SUBSYSTEM=="input", KERNEL=="event*", ATTRS{name}=="ADS7846*", SYMLINK+="input/touchscreen0"
EOF

sync
echo ""
echo "=== Setup Complete ==="
echo ""
echo "Next steps:"
echo "1. Reboot the Raspberry Pi: reboot"
echo "2. After reboot, calibrate touch: ts_calibrate"
echo "3. Test touch: ts_test"
echo "4. Run LeafSense with: /opt/leafsense/LeafSense"
echo ""
echo "If the display doesn't work, check:"
echo "  - dmesg | grep -i fb"
echo "  - dmesg | grep -i spi"
echo "  - ls /dev/fb*"
echo ""
