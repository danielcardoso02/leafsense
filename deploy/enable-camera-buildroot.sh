#!/bin/bash
#
# LeafSense Camera Enable Script for Buildroot
# This script helps configure Buildroot to enable Raspberry Pi camera support
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║     LeafSense Camera Enable Script for Buildroot             ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Check if Buildroot directory exists
if [ -z "$BUILDROOT_DIR" ]; then
    echo "⚠️  BUILDROOT_DIR environment variable not set"
    echo ""
    echo "Please set the path to your Buildroot directory:"
    echo "  export BUILDROOT_DIR=/path/to/buildroot"
    echo ""
    echo "If you don't have Buildroot yet, download it:"
    echo "  cd ~/Desktop/ESRG/2025-2026/Project/Rasp"
    echo "  wget https://buildroot.org/downloads/buildroot-2025.08.tar.gz"
    echo "  tar xzf buildroot-2025.08.tar.gz"
    echo "  export BUILDROOT_DIR=\$PWD/buildroot-2025.08"
    echo ""
    exit 1
fi

if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "❌ Buildroot directory not found: $BUILDROOT_DIR"
    exit 1
fi

echo "✓ Buildroot directory: $BUILDROOT_DIR"
echo ""

# Create a custom defconfig fragment for camera support
CAMERA_CONFIG="$BUILDROOT_DIR/camera_support.config"

echo "📝 Creating camera configuration fragment..."
cat > "$CAMERA_CONFIG" << 'EOF'
# Camera Support Configuration Fragment

# VideoCore userland libraries (essential for Pi camera)
BR2_PACKAGE_RPI_USERLAND=y

# V4L utilities for video device management
BR2_PACKAGE_V4L_UTILS=y

# Kernel configuration for camera
BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="$(BR2_EXTERNAL_RPI_PATH)/board/raspberrypi/linux-camera.config"

# Additional useful packages
BR2_PACKAGE_OPENCV4=y
BR2_PACKAGE_OPENCV4_LIB_VIDEOIO=y

EOF

echo "✓ Created: $CAMERA_CONFIG"
echo ""

# Create kernel config fragment
KERNEL_CONFIG="$BUILDROOT_DIR/linux-camera.config"

echo "📝 Creating kernel configuration fragment..."
cat > "$KERNEL_CONFIG" << 'EOF'
# Raspberry Pi Camera Kernel Configuration

# Media support
CONFIG_MEDIA_SUPPORT=y
CONFIG_MEDIA_CAMERA_SUPPORT=y
CONFIG_VIDEO_DEV=y
CONFIG_VIDEO_V4L2=y

# Broadcom VideoCore
CONFIG_VIDEO_BCM2835=m
CONFIG_VIDEO_BCM2835_UNICAM=m

# Camera sensors
CONFIG_VIDEO_OV5647=m
CONFIG_VIDEO_IMX219=m

# V4L2 memory-to-memory
CONFIG_V4L2_MEM2MEM_DEV=m

# Media controller
CONFIG_MEDIA_CONTROLLER=y

EOF

echo "✓ Created: $KERNEL_CONFIG"
echo ""

# Instructions for manual configuration
echo "═══════════════════════════════════════════════════════════════"
echo "📋 NEXT STEPS - Manual Configuration Required"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "1️⃣  Navigate to Buildroot directory:"
echo "   cd $BUILDROOT_DIR"
echo ""
echo "2️⃣  Run menuconfig:"
echo "   make menuconfig"
echo ""
echo "3️⃣  Enable these packages:"
echo ""
echo "   Target packages --->"
echo "     Hardware handling --->"
echo "       [*] rpi-userland (VideoCore libraries)"
echo "       [*]   Install test programs"
echo "       [*] v4l-utils"
echo ""
echo "   Kernel --->"
echo "     Kernel configuration --->"
echo "       [*] Use a custom config file"
echo "       (linux-camera.config) Configuration file path"
echo ""
echo "4️⃣  Save and exit menuconfig"
echo ""
echo "5️⃣  Rebuild Buildroot:"
echo "   make clean"
echo "   make"
echo ""
echo "6️⃣  Flash new image to SD card:"
echo "   sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M status=progress"
echo "   (Replace /dev/sdX with your SD card device)"
echo ""
echo "7️⃣  Boot Raspberry Pi and verify:"
echo "   vcgencmd get_camera"
echo "   # Should show: supported=1 detected=1"
echo ""
echo "   raspistill -o /tmp/test.jpg"
echo "   # Should capture a real photo"
echo ""
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "🔧 Alternative: Quick Test with Device Tree Overlay"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Before rebuilding, you can try loading the camera overlay:"
echo ""
echo "   ssh root@10.42.0.196"
echo "   dtoverlay ov5647"
echo "   vcgencmd get_camera"
echo ""
echo "If this works, you may just need to add it to /boot/config.txt:"
echo "   dtoverlay=ov5647"
echo ""
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Configuration files created:"
echo "  - $CAMERA_CONFIG"
echo "  - $KERNEL_CONFIG"
echo ""
echo "Need help? See docs/04-RASPBERRY-PI-DEPLOYMENT.md"
echo ""
