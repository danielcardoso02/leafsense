#!/bin/sh
# LeafSense Startup Script
# ========================
# This script launches LeafSense with correct display and touchscreen configuration.
#
# CRITICAL TOUCHSCREEN CONFIGURATION:
# The rotate=180:invertx parameter in QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS
# works with dtoverlay=piscreen,rotate=270 in config.txt.
#
# This ensures touch coordinates are correctly mapped without calibration.
# DO NOT use tslib - it causes the application to freeze.
#
# Last verified: January 19, 2026
# Hardware: 3.5" LCD with ADS7846 touchscreen (piscreen overlay)

# ============================================
# DISPLAY CONFIGURATION
# ============================================
# fb1 = SPI LCD (480x320)
# fb0 = HDMI output
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FB_NO_LIBINPUT=1
export QT_QPA_FONTDIR=/usr/share/fonts

# ============================================
# TOUCHSCREEN CONFIGURATION (CRITICAL)
# ============================================
# Uses evdev driver directly - NO tslib needed
# rotate=180:invertx matches display rotation (piscreen,rotate=270)
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"

# ============================================
# APPLICATION PATHS
# ============================================
export LEAFSENSE_DB="/opt/leafsense/leafsense.db"
export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH

# Ensure we're in the application directory
cd /opt/leafsense

# Create gallery directory if it doesn't exist
mkdir -p /opt/leafsense/gallery

# Start LeafSense
exec ./LeafSense
