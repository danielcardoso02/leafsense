#!/bin/sh
# LeafSense Qt5 and Touchscreen Environment Configuration
# ========================================================
# This file is sourced at login to set up the correct environment
# for the LeafSense application with Waveshare 3.5" LCD (C).
#
# CRITICAL: These settings ensure touchscreen works without calibration.
# The evdev driver with rotate=90 provides correct touch coordinates.
# DO NOT use tslib - it causes application freezing.
#
# Last verified: January 10, 2026

# ============================================
# TOUCHSCREEN CONFIGURATION (CRITICAL)
# ============================================
# The rotate=90 parameter MUST match the display rotation in config.txt
# This ensures touch coordinates are correctly mapped to screen pixels.
#
# Rotation mapping for Waveshare 3.5" LCD (C) with piscreen overlay:
#   The correct configuration is rotate=180:invertx
#   This was verified working on January 22, 2026
#
# DO NOT CHANGE - This is the verified working configuration!
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"

# ============================================
# DISPLAY CONFIGURATION
# ============================================
# Use linuxfb platform with framebuffer fb1 (Waveshare LCD)
# fb0 is HDMI, fb1 is the SPI LCD
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1

# Hide cursor on touchscreen
export QT_QPA_FB_HIDECURSOR=1

# Font directory
export QT_QPA_FONTDIR=/usr/share/fonts

# ============================================
# APPLICATION PATHS
# ============================================
export LEAFSENSE_HOME=/opt/leafsense
export LEAFSENSE_DB=/opt/leafsense/leafsense.db
