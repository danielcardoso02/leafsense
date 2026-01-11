#!/bin/sh
# LeafSense Startup Script
# ========================
# This script launches LeafSense with correct display and touchscreen configuration.
#
# CRITICAL TOUCHSCREEN CONFIGURATION:
# The rotate=90 parameter in QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS
# MUST match the rotate=90 in config.txt dtoverlay=waveshare35c.
#
# This ensures touch coordinates are correctly mapped without calibration.
# DO NOT use tslib - it causes the application to freeze.
#
# Last verified: January 10, 2026
# Hardware: Waveshare 3.5" LCD (C) with ADS7846 touchscreen

# ============================================
# DISPLAY CONFIGURATION
# ============================================
# fb1 = Waveshare SPI LCD (480x320)
# fb0 = HDMI output
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FONTDIR=/usr/share/fonts

# ============================================
# TOUCHSCREEN CONFIGURATION (CRITICAL)
# ============================================
# Uses evdev driver directly - NO tslib needed
# rotate=90 must match display rotation
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"

# ============================================
# APPLICATION PATHS
# ============================================
export LEAFSENSE_DB="/opt/leafsense/leafsense.db"

# Ensure we're in the application directory
cd /opt/leafsense

# Create gallery directory if it doesn't exist
mkdir -p /opt/leafsense/gallery

# Start LeafSense
exec ./LeafSense -platform linuxfb:fb=/dev/fb1
