# LeafSense Touchscreen Configuration Guide

## Critical Information

**Last Verified:** January 11, 2026  
**Hardware:** Waveshare 3.5" LCD (C) with ILI9486 display and ADS7846 touchscreen  
**Driver:** Qt5 evdev touchscreen plugin (NOT tslib)

---

## Working Configuration

### config.txt (Boot Partition)

```ini
# Display and touchscreen overlay
dtoverlay=waveshare35c:rotate=90,speed=16000000,fps=50

# Framebuffer dimensions
framebuffer_width=480
framebuffer_height=320
```

### Environment Variables (Runtime)

```bash
# CRITICAL: The rotation MUST be configured for correct touch mapping
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_FB_HIDECURSOR=1
```

---

## Why evdev Instead of tslib?

**Problem with tslib:**
- tslib causes the application to freeze when the touchscreen is touched
- This is due to SPI bus contention and the way tslib reads the touch device

**Solution with evdev:**
- Qt5's evdev touchscreen plugin reads directly from `/dev/input/event0`
- The `rotate=90` parameter handles coordinate transformation for the 90° rotated display
- No calibration file is needed
- No `ts_calibrate` or `ts_uinput` processes required

---

## Rotation Parameter Mapping

The touch coordinates require transformation to match the rotated display. For the Waveshare 3.5" LCD (C) with `rotate=90` in config.txt, the Qt evdev parameter needs `rotate=90`:

| config.txt dtoverlay | Qt Environment Variable |
|---------------------|------------------------|
| `rotate=0` | `rotate=0` |
| `rotate=90` | `rotate=90` |
| `rotate=180` | `rotate=180` |
| `rotate=270` | `rotate=270` |

**Note:** For the Waveshare 3.5" LCD (C), both display and touch use `rotate=90` - no additional parameters needed.

---

## SPI Speed and FPS Settings

### Why 16MHz Instead of Higher?

| SPI Speed | Result |
|-----------|--------|
| 24MHz+ | Touch causes UI freeze (SPI contention) |
| 16MHz | Touch works reliably, slight screen refresh flicker |
| 12MHz | Very stable but more visible flicker |

### FPS Setting

| FPS | Result |
|-----|--------|
| 20 | Very visible blinking/flicker |
| 30 | Moderate blinking |
| 40 | Acceptable blinking |
| 50 | Minimal blinking (recommended) |
| 60+ | May cause instability |

**Recommended:** `speed=16000000,fps=50`

---

## Files That Must Be Preserved

### Boot Partition Files

1. **config.txt** - Contains dtoverlay with rotation and SPI settings
2. **overlays/waveshare35c.dtbo** - Display driver overlay

### Root Filesystem Files

1. **/etc/profile.d/leafsense-qt.sh** - Qt environment variables
2. **/etc/init.d/S99leafsense** - Auto-start script with correct environment
3. **/opt/leafsense/start_leafsense.sh** - Manual startup script

---

## Buildroot Integration

### Overlay Directory Structure

```
deploy/
├── boot-overlay/
│   ├── config.txt              # Complete boot configuration
│   ├── cmdline.txt             # Kernel command line
│   └── overlays/
│       └── waveshare35c.dtbo   # Display overlay
│
└── rootfs-overlay/
    ├── etc/
    │   ├── init.d/
    │   │   └── S99leafsense    # Auto-start script
    │   └── profile.d/
    │       └── leafsense-qt.sh # Qt environment
    └── opt/
        └── leafsense/
            └── start_leafsense.sh  # Startup script
```

### Post-Build Script

The `board/leafsense/post-build.sh` script:
1. Copies `config.txt` to boot partition
2. Copies `waveshare35c.dtbo` to overlays
3. Creates Qt environment configuration
4. Creates init and startup scripts

This ensures that after reflashing, the touchscreen works immediately without any manual configuration.

---

## Troubleshooting

### Touch Not Working

1. Check that LeafSense is running:
   ```bash
   ps aux | grep LeafSense
   ```

2. Verify environment variable is set:
   ```bash
   cat /proc/$(pgrep LeafSense)/environ | tr '\0' '\n' | grep TOUCH
   ```

3. Check touch device exists:
   ```bash
   cat /proc/bus/input/devices | grep -A5 ADS7846
   ```

### Touch Coordinates Wrong

1. Verify configuration:
   ```bash
   cat /boot/config.txt | grep waveshare  # Should show rotate=90
   echo $QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS  # Should show rotate=90
   ```

2. If they don't match, restart with correct rotation:
   ```bash
   export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
   killall LeafSense
   /opt/leafsense/start_leafsense.sh
   ```

### Application Freezes on Touch

This indicates tslib is being used instead of evdev. Check:
```bash
ps aux | grep ts_uinput  # Should show nothing
env | grep TSLIB  # Should show nothing
```

Solution: Ensure `QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS` is set, not `TSLIB_*` variables.

---

## Verification After Reflash

After flashing a new image, verify:

1. **Display shows LeafSense UI** (not blank or HDMI output)
2. **Touch works immediately** without running any calibration
3. **Touch in top-left corner registers as top-left** (not inverted)
4. **No application freeze when touching screen**

If all these pass, the configuration is correctly preserved.

---

## Summary

**Key Points:**
- Use **evdev**, NOT tslib
- Set `rotate=90` in config.txt dtoverlay
- Set `rotate=90` in Qt environment variable
- Use `speed=16000000` (16MHz) to prevent touch freeze
- Use `fps=50` to minimize screen flicker
- All configuration files are in the Buildroot overlay
- No calibration required after reflash

**Contact:** LeafSense Development Team - University of Minho
