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
# LeafSense required packages

# Qt5 (GUI)
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

# SQLite (Database)
BR2_PACKAGE_SQLITE=y

# OpenCV (Camera/Image Processing)
BR2_PACKAGE_OPENCV4=y
BR2_PACKAGE_OPENCV4_LIB_IMGPROC=y
BR2_PACKAGE_OPENCV4_LIB_IMGCODECS=y
BR2_PACKAGE_OPENCV4_LIB_HIGHGUI=y
BR2_PACKAGE_OPENCV4_LIB_VIDEOIO=y
BR2_PACKAGE_OPENCV4_WITH_JPEG=y
BR2_PACKAGE_OPENCV4_WITH_PNG=y

# Hardware libraries
BR2_PACKAGE_RPI_USERLAND=y
BR2_PACKAGE_I2C_TOOLS=y
BR2_PACKAGE_LIBGPIOD=y
BR2_PACKAGE_LIBGPIOD_TOOLS=y

# System utilities
BR2_PACKAGE_EUDEV=y
BR2_PACKAGE_EUDEV_RULES_GEN=y
BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_EUDEV=y

# Networking (for SSH access)
BR2_PACKAGE_DROPBEAR=y
BR2_PACKAGE_DHCPCD=y

# Debugging tools (optional, can disable for release)
BR2_PACKAGE_HTOP=y
BR2_PACKAGE_NANO=y

# Fonts (for Qt5 GUI)
BR2_PACKAGE_DEJAVU=y
BR2_PACKAGE_FONTCONFIG=y

# Image libraries
BR2_PACKAGE_JPEG=y
BR2_PACKAGE_LIBPNG=y

# Root password (change this!)
BR2_TARGET_GENERIC_ROOT_PASSWD="leafsense"

# Hostname
BR2_TARGET_GENERIC_HOSTNAME="leafsense-pi"

# Timezone
BR2_TARGET_TZ_INFO=y
BR2_TARGET_LOCALTIME="Europe/Lisbon"
EOF

echo "✓ Configuration fragment created"
echo ""

echo "[3/5] Merging configuration..."
# Merge the fragment with current config
support/kconfig/merge_config.sh .config leafsense_config.fragment

echo ""
echo "✓ Configuration merged"
echo ""

echo "[4/5] Creating post-build script for config.txt..."

# Create overlay directory structure
mkdir -p "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/etc"
mkdir -p "$BUILDROOT_DIR/board/leafsense/rootfs_overlay/opt/leafsense"

# Create post-build script
cat > "$BUILDROOT_DIR/board/leafsense/post-build.sh" << 'POSTBUILD'
#!/bin/bash
# LeafSense post-build script

# Add hardware configuration to config.txt
CONFIG_TXT="$BINARIES_DIR/rpi-firmware/config.txt"

if [ -f "$CONFIG_TXT" ]; then
    # Check if our config is already added
    if ! grep -q "# LeafSense Hardware Configuration" "$CONFIG_TXT"; then
        cat >> "$CONFIG_TXT" << 'HWCONFIG'

# LeafSense Hardware Configuration
# ================================

# Enable I2C (for ADC ADS1115 and RTC DS3231)
dtparam=i2c_arm=on
dtparam=i2c1=on

# Enable 1-Wire for DS18B20 temperature sensor (GPIO 19)
dtoverlay=w1-gpio,gpiopin=19

# Enable Camera (CSI)
start_x=1
gpu_mem=128

# Enable SPI (for display if needed)
dtparam=spi=on

# GPU memory for camera operations
gpu_mem=256

# Disable Bluetooth to free up UART (optional)
# dtoverlay=disable-bt
HWCONFIG
        echo "Added LeafSense hardware configuration to config.txt"
    fi
fi

# Create LeafSense directories on target
mkdir -p "$TARGET_DIR/opt/leafsense/models"
mkdir -p "$TARGET_DIR/opt/leafsense/data"
mkdir -p "$TARGET_DIR/opt/leafsense/images"
mkdir -p "$TARGET_DIR/var/log/leafsense"

# Set permissions
chmod 755 "$TARGET_DIR/opt/leafsense"

echo "LeafSense post-build completed"
POSTBUILD

chmod +x "$BUILDROOT_DIR/board/leafsense/post-build.sh"

echo "✓ Post-build script created"
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
echo "  1. Review configuration (optional):"
echo "     cd $BUILDROOT_DIR"
echo "     make menuconfig"
echo ""
echo "  2. Configure kernel for I2C/1-Wire/Camera:"
echo "     make linux-menuconfig"
echo ""
echo "  3. Build the image (takes 1-3 hours):"
echo "     make -j\$(nproc)"
echo ""
echo "  4. Flash to SD card:"
echo "     sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress"
echo ""
echo "  5. SSH access (after boot):"
echo "     ssh root@<PI_IP>  (password: leafsense)"
echo ""
echo "==========================================="
