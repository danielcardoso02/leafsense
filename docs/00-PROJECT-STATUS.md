# LeafSense Project Status

---

## Overview

LeafSense is a complete embedded hydroponic monitoring system running on Raspberry Pi 4 with custom Buildroot Linux. This document provides a status report of all implemented features.

---

## Implemented Features

### Operating System (Buildroot)

| Component | Status | Details |
|-----------|--------|---------|
| Custom Linux Image | ✅ Complete | Buildroot 2025.08, kernel 6.12.41-v8 |
| Qt5 Framework | ✅ Complete | Qt 5.15.14 with Widgets, Charts, SQL |
| OpenCV | ✅ Complete | OpenCV 4.11.0 for image processing |
| ONNX Runtime | ✅ Complete | Version 1.16.3 for ARM64 |
| SQLite | ✅ Complete | Version 3.48.0 for data persistence |
| BusyBox Init | ✅ Complete | S99leafsense init script for auto-start |
| SSH/Networking | ✅ Complete | Static IP 10.42.0.196, passwordless SSH |

### Display & Touchscreen

| Component | Status | Details |
|-----------|--------|---------|
| Waveshare 3.5" LCD (C) | ✅ Complete | ILI9486 controller, 480×320 resolution |
| Framebuffer | ✅ Complete | `/dev/fb1` at 31fps |
| Touchscreen | ✅ Complete | ADS7846, evdev at `/dev/input/event0` |
| Calibration | ✅ Complete | Qt evdev: `rotate=180:invertx` |
| Qt Platform | ✅ Complete | linuxfb:fb=/dev/fb1:size=480x320 |

### Camera System

| Component | Status | Details |
|-----------|--------|---------|
| Hardware | ✅ Complete | OV5647 (Pi Camera v1), 5MP |
| Driver | ✅ Complete | libcamera with `cam` utility |
| Capture | ✅ Complete | BGR888 format, 640×480 resolution |
| Gallery | ✅ Complete | Photos saved to `/opt/leafsense/gallery/` |
| Integration | ✅ Complete | Photos displayed in GUI Gallery tab |

### Graphical User Interface

| Component | Status | Details |
|-----------|--------|---------|
| Login Screen | ✅ Complete | Username/password authentication |
| Dashboard | ✅ Complete | Real-time sensor display, health status |
| Analytics Tab | ✅ Complete | 30-day trend charts |
| Gallery Tab | ✅ Complete | Photo viewer with navigation |
| Logs Tab | ✅ Complete | Alerts, diseases, deficiencies, maintenance |
| Settings Tab | ✅ Complete | Ideal conditions configuration |
| Light/Dark Theme | ✅ Complete | ThemeManager with toggle |

### Machine Learning

| Component | Status | Details |
|-----------|--------|---------|
| Model | ✅ Complete | MobileNetV3-Small trained on 4 classes |
| Classes | ✅ Complete | Healthy, Nutrient Deficiency, Pest Damage, Disease |
| Accuracy | ✅ Complete | 99.39% validation accuracy |
| Runtime | ✅ Complete | ONNX Runtime C++ inference |
| Integration | ✅ Complete | Automatic photo analysis with confidence scores |

### Database

| Component | Status | Details |
|-----------|--------|---------|
| Schema | ✅ Complete | 10 tables (users, plants, sensors, alerts, ML, etc.) |
| Operations | ✅ Complete | Full CRUD via dbManager |
| Logging | ✅ Complete | All sensor readings, ML predictions, events |
| Views | ✅ Complete | Latest reading, unread alerts, summaries |

### Kernel Module

| Component | Status | Details |
|-----------|--------|---------|
| LED Driver | ✅ Complete | Character device at `/dev/led0` |
| GPIO Control | ✅ Complete | Direct BCM2711 register access |
| Integration | ✅ Complete | AlertLed class wrapper |

### Threading Architecture

| Component | Status | Details |
|-----------|--------|---------|
| POSIX Threads | ✅ Complete | tReadSensors, tCamera, actuator threads |
| Synchronization | ✅ Complete | Mutexes and condition variables |
| Priorities | ✅ Complete | 4-tier priority scheduling |
| IPC | ✅ Complete | Message queues for daemon communication |

---

## Sensor & Actuator Status

### Sensors (Auto-Detect Real vs Mock)

The following sensors have **mock implementations** that return simulated values when the physical hardware is not detected. The code automatically uses real values when hardware is connected.

| Sensor | Mock Behavior | Real Mode Trigger |
|--------|---------------|-------------------|
| DS18B20 (Temperature) | Random 15-25°C | 1-Wire device found at `/sys/bus/w1/devices/28-*` |
| pH Sensor (PH-4502C) | Random 6.0-7.0 | ADS1115 ADC returns valid voltage on Channel 2 |
| TDS/EC Sensor | Random 1200-1400ppm | ADS1115 ADC returns valid voltage on Channel 3 |

### Actuators (Real GPIO via libgpiod)

| Actuator | GPIO | Status |
|----------|------|--------|
| pH Up Pump | GPIO 6 | ✅ Working |
| pH Down Pump | GPIO 13 | ✅ Working |
| Nutrient Pump | GPIO 5 | ✅ Working |
| Water Heater | GPIO 26 | ✅ Working |
| Alert LED | Kernel Module | ✅ Working via `/dev/led0` |

### I2C & GPIO Implementation

| Component | Status | Details |
|-----------|--------|---------|
| I2C Bus | ✅ Working | `/dev/i2c-1` enabled |
| i2c-dev Module | ✅ Auto-load | Added to `/etc/modules` |
| ADS1115 ADC | ✅ Driver Ready | Address 0x48, auto-fallback to mock if not connected |
| pH Sensor | ✅ I2C Ready | Channel 2, voltage-to-pH conversion |
| TDS Sensor | ✅ I2C Ready | Channel 3, voltage-to-ppm conversion |

---

## Hardware Integration

See [12-SENSOR-ACTUATOR-INTEGRATION.md](12-SENSOR-ACTUATOR-INTEGRATION.md) for complete instructions on connecting physical hardware.

### Integration Checklist

- [ ] Enable 1-Wire overlay for DS18B20 temperature sensor
- [x] Configure I2C for analog sensors (i2c-dev module auto-loads)
- [x] ADS1115 ADC driver implemented with I2C support
- [x] Heater GPIO control via libgpiod (GPIO 26 working)
- [x] pH Up Pump GPIO control via libgpiod (GPIO 6 working)
- [x] pH Down Pump GPIO control via libgpiod (GPIO 13 working)
- [x] Nutrient Pump GPIO control via libgpiod (GPIO 5 working)
- [x] Alert LED via kernel module (/dev/led0 working)
- [ ] Connect physical ADS1115 and calibrate pH/TDS sensors
- [ ] Connect DS18B20 for real temperature readings

---

## Known Issues

### Touchscreen Calibration

Qt evdev parameters must be set before each launch. This is handled in `/opt/leafsense/start_leafsense.sh`:

```bash
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
```

### Camera Strategy

The camera driver tries multiple capture methods:
1. libcamera `cam` utility (preferred, works on Pi)
2. libcamera-still fallback
3. OpenCV V4L2 (for USB webcams)
4. Test pattern (if no camera available)

### Time Synchronization

The Pi has no RTC, so time resets on boot. NTP synchronization works when network is available.

---

## File Locations

| Path | Contents |
|------|----------|
| `/opt/leafsense/LeafSense` | Main application binary |
| `/opt/leafsense/start_leafsense.sh` | Startup script |
| `/opt/leafsense/leafsense.db` | SQLite database |
| `/opt/leafsense/leafsense_model.onnx` | ML model |
| `/opt/leafsense/leafsense_model_classes.txt` | Class labels |
| `/opt/leafsense/gallery/` | Captured plant photos |
| `/usr/lib/libonnxruntime.so.1.16.3` | ONNX Runtime library |
| `/boot/config.txt` | Boot configuration (LCD overlay) |

---

## Future Development

### Phase 1: Hardware Integration
1. Connect and calibrate DS18B20 temperature sensor
2. Install ADS1115 ADC and connect pH/TDS sensors
3. Wire peristaltic pumps to GPIO with relay modules
4. Add DS3231 RTC for time persistence
5. Test actuator control loop with real sensors

### Phase 2: System Improvements
1. Add NTP sync when network is available
2. Implement sensor calibration UI
3. Add data export functionality
4. Create system health monitoring dashboard

### Phase 3: Advanced Features
1. Remote monitoring via web interface
2. Historical trend analysis and predictions
3. Multi-plant support
4. Integration with external services



