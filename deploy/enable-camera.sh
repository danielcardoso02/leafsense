#!/bin/bash
#
# LeafSense - Enable Raspberry Pi Camera in Buildroot
# This script configures and rebuilds Buildroot with camera support
#

set -e

BUILDROOT_DIR="/home/daniel/buildroot/buildroot-2025.08"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  LeafSense - Enable Raspberry Pi Camera in Buildroot      ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Verify Buildroot directory
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "❌ Error: Buildroot directory not found at: $BUILDROOT_DIR"
    echo ""
    echo "Please update BUILDROOT_DIR in this script if your path is different."
    exit 1
fi

echo "✓ Buildroot directory: $BUILDROOT_DIR"
echo ""

# Check if already configured
if [ ! -f "$BUILDROOT_DIR/.config" ]; then
    echo "⚠️  No .config found. Loading raspberrypi4_64_defconfig..."
    cd "$BUILDROOT_DIR"
    make raspberrypi4_64_defconfig
    echo "✓ Base configuration loaded"
    echo ""
fi

# Copy kernel config fragment to Buildroot
KERNEL_FRAGMENT="$BUILDROOT_DIR/board/raspberrypi/linux-camera.config"
mkdir -p "$(dirname "$KERNEL_FRAGMENT")"
cp "$SCRIPT_DIR/linux-camera.config" "$KERNEL_FRAGMENT"
echo "✓ Copied kernel camera configuration to: $KERNEL_FRAGMENT"
echo ""

# Enable camera packages using sed
echo "📝 Enabling camera packages in .config..."
cd "$BUILDROOT_DIR"

# Backup current config
cp .config .config.backup
echo "✓ Backed up current config to .config.backup"

# Enable rpi-userland
if grep -q "^# BR2_PACKAGE_RPI_USERLAND is not set" .config; then
    sed -i 's/^# BR2_PACKAGE_RPI_USERLAND is not set/BR2_PACKAGE_RPI_USERLAND=y/' .config
    echo "✓ Enabled BR2_PACKAGE_RPI_USERLAND"
else
    echo "BR2_PACKAGE_RPI_USERLAND=y" >> .config
    echo "✓ Added BR2_PACKAGE_RPI_USERLAND"
fi

# Enable v4l-utils
if grep -q "^# BR2_PACKAGE_V4L_UTILS is not set" .config; then
    sed -i 's/^# BR2_PACKAGE_V4L_UTILS is not set/BR2_PACKAGE_V4L_UTILS=y/' .config
    echo "✓ Enabled BR2_PACKAGE_V4L_UTILS"
else
    echo "BR2_PACKAGE_V4L_UTILS=y" >> .config
    echo "✓ Added BR2_PACKAGE_V4L_UTILS"
fi

# Add kernel config fragment
if ! grep -q "BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES" .config; then
    echo 'BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="board/raspberrypi/linux-camera.config"' >> .config
    echo "✓ Added kernel camera configuration fragment"
fi

# Run olddefconfig to process new options
echo ""
echo "⚙️  Processing configuration changes..."
make olddefconfig
echo "✓ Configuration updated"
echo ""

# Show what was enabled
echo "════════════════════════════════════════════════════════════"
echo "📋 Camera Support Enabled:"
echo "════════════════════════════════════════════════════════════"
grep "BR2_PACKAGE_RPI_USERLAND\|BR2_PACKAGE_V4L_UTILS\|BR2_LINUX_KERNEL_CONFIG_FRAGMENT" .config | grep -v "^#"
echo ""

# Ask user if they want to rebuild now
echo "════════════════════════════════════════════════════════════"
echo "⚠️  IMPORTANT: Rebuild Required"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "The configuration has been updated. To enable the camera,"
echo "you need to rebuild Buildroot. This will take 2-4 hours."
echo ""
echo "Options:"
echo "  1. Rebuild now (recommended)"
echo "  2. Review config in menuconfig first"
echo "  3. Exit (rebuild manually later)"
echo ""
read -p "Enter your choice (1/2/3): " choice

case $choice in
    1)
        echo ""
        echo "🔨 Starting Buildroot rebuild..."
        echo "This will take 2-4 hours. You can monitor progress here."
        echo ""
        sleep 2
        make
        echo ""
        echo "✅ Build complete!"
        echo ""
        echo "Output image: $BUILDROOT_DIR/output/images/sdcard.img"
        echo ""
        echo "Next steps:"
        echo "  1. Flash SD card: sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress"
        echo "  2. Boot Raspberry Pi"
        echo "  3. Verify camera: ssh root@10.42.0.196 'vcgencmd get_camera'"
        echo "  4. Test capture: ssh root@10.42.0.196 'raspistill -o /tmp/test.jpg'"
        echo ""
        ;;
    2)
        echo ""
        echo "Opening menuconfig..."
        echo ""
        echo "Navigate to:"
        echo "  Target packages → Hardware handling → rpi-userland"
        echo "  Target packages → Hardware handling → v4l-utils"
        echo ""
        echo "After saving, run: make"
        echo ""
        make menuconfig
        ;;
    3)
        echo ""
        echo "Configuration saved. To rebuild later, run:"
        echo "  cd $BUILDROOT_DIR"
        echo "  make"
        echo ""
        ;;
    *)
        echo ""
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac

echo "════════════════════════════════════════════════════════════"
echo ""
