# LeafSense Touchscreen Configuration Guide

## Critical Information

**Last Verified:** January 19, 2026  
**Hardware:** 3.5" LCD with ILI9486 display and ADS7846 touchscreen  
**Driver:** Qt5 evdev touchscreen plugin (NOT tslib)

---

## Working Configuration

### config.txt (Boot Partition)

```ini
# Display and touchscreen overlay
dtoverlay=piscreen,speed=16000000,rotate=270

# Framebuffer dimensions
framebuffer_width=480
framebuffer_height=320
```

### Environment Variables (Runtime)

```bash
# CRITICAL: The rotation MUST be configured for correct touch mapping
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1:size=480x320
export QT_QPA_FB_HIDECURSOR=1
export QT_QPA_FB_NO_LIBINPUT=1
```

---

## Why evdev Instead of tslib?

**Problem with tslib:**
- tslib causes the application to freeze when the touchscreen is touched
- This is due to SPI bus contention and the way tslib reads the touch device

**Solution with evdev:**
- Qt5's evdev touchscreen plugin reads directly from `/dev/input/event0`
- The `rotate=180:invertx` parameter handles coordinate transformation for the rotated display
- No calibration file is needed
- No `ts_calibrate` or `ts_uinput` processes required

---

## Rotation Parameter Mapping

The touch coordinates require transformation to match the rotated display. For the 3.5" LCD with `rotate=270` in config.txt (piscreen overlay), the Qt evdev parameter needs `rotate=180:invertx`:

| config.txt dtoverlay | Qt Environment Variable |
|---------------------|------------------------|
| `piscreen,rotate=270` | `rotate=180:invertx` |
| `waveshare35c,rotate=90` | `rotate=90` |

**Note:** Different overlays (piscreen vs waveshare35c) may require different touch rotation parameters.

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
3. **/opt/leafsense/start.sh** - Manual startup script

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
            └── start.sh  # Startup script
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
   mount /dev/mmcblk0p1 /boot 2>/dev/null
   cat /boot/config.txt | grep piscreen  # Should show rotate=270
   echo $QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS  # Should show rotate=180:invertx
   ```

2. If they don't match, restart with correct rotation:
   ```bash
   export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
   killall LeafSense
   cd /opt/leafsense && ./start.sh
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
- Current display overlay: `dtoverlay=piscreen,rotate=270`
- Set `rotate=180:invertx` in Qt environment variable
- Use `speed=16000000` (16MHz) to prevent touch freeze
- All configuration files are in the Buildroot overlay
- No calibration required after reflash

**Contact:** LeafSense Development Team - University of Minho
