# LeafSense Project Status - Final Report

**Date:** January 11, 2026  
**Version:** 1.5.1  
**Authors:** Daniel Cardoso (PG53753), Marco Costa (PG60210)  
**Course:** Master's in Industrial Electronics and Computers Engineering  
**Institution:** University of Minho, School of Engineering

---

## Executive Summary

LeafSense is a complete embedded hydroponic monitoring system running on Raspberry Pi 4 with custom Buildroot Linux. This document provides a final status report of what is implemented, what is working, and what remains for future development.

---

## ✅ Fully Implemented & Working

### 1. Operating System (Buildroot)
| Component | Status | Details |
|-----------|--------|---------|
| Custom Linux Image | ✅ Complete | Buildroot 2025.08, kernel 6.12.41-v8 |
| Qt5 Framework | ✅ Complete | Qt 5.15.14 with Widgets, Charts, SQL |
| OpenCV | ✅ Complete | OpenCV 4.11.0 for image processing |
| ONNX Runtime | ✅ Complete | Version 1.16.3 for ARM64 |
| SQLite | ✅ Complete | Version 3.48.0 for data persistence |
| BusyBox Init | ✅ Complete | S99leafsense init script for auto-start |
| SSH/Networking | ✅ Complete | Static IP 10.42.0.196, passwordless SSH |

### 2. Display & Touchscreen
| Component | Status | Details |
|-----------|--------|---------|
| Waveshare 3.5" LCD (C) | ✅ Complete | ILI9486 controller, 480×320 resolution |
| Framebuffer | ✅ Complete | `/dev/fb1` at 31fps |
| Touchscreen | ✅ Complete | ADS7846, evdev at `/dev/input/event0` |
| Calibration | ✅ Complete | Qt evdev: `rotate=180:invertx` |
| Qt Platform | ✅ Complete | linuxfb:fb=/dev/fb1:size=480x320 |

### 3. Camera System
| Component | Status | Details |
|-----------|--------|---------|
| Hardware | ✅ Complete | OV5647 (Pi Camera v1), 5MP |
| Driver | ✅ Complete | libcamera with `cam` utility |
| Capture | ✅ Complete | BGR888 format, 640×480 resolution |
| Gallery | ✅ Complete | Photos saved to `/opt/leafsense/gallery/` |
| Integration | ✅ Complete | Photos displayed in GUI Gallery tab |

### 4. Graphical User Interface
| Component | Status | Details |
|-----------|--------|---------|
| Login Screen | ✅ Complete | Username/password authentication |
| Dashboard | ✅ Complete | Real-time sensor display, health status |
| Analytics Tab | ✅ Complete | 30-day trend charts |
| Gallery Tab | ✅ Complete | Photo viewer with navigation |
| Logs Tab | ✅ Complete | Alerts, diseases, deficiencies, maintenance |
| Settings Tab | ✅ Complete | Ideal conditions configuration |
| Light/Dark Theme | ✅ Complete | ThemeManager with toggle |

### 5. Machine Learning
| Component | Status | Details |
|-----------|--------|---------|
| Model | ✅ Complete | ResNet-18 trained on 4 classes |
| Classes | ✅ Complete | Healthy, Nutrient Deficiency, Pest Damage, Disease |
| Accuracy | ✅ Complete | 99.39% validation accuracy |
| Runtime | ✅ Complete | ONNX Runtime C++ inference |
| Integration | ✅ Complete | Automatic photo analysis with confidence scores |

### 6. Database
| Component | Status | Details |
|-----------|--------|---------|
| Schema | ✅ Complete | 10 tables (users, plants, sensors, alerts, ML, etc.) |
| Operations | ✅ Complete | Full CRUD via dbManager |
| Logging | ✅ Complete | All sensor readings, ML predictions, events |
| Views | ✅ Complete | Latest reading, unread alerts, summaries |

### 7. Kernel Module
| Component | Status | Details |
|-----------|--------|---------|
| LED Driver | ✅ Complete | Character device at `/dev/led0` |
| GPIO Control | ✅ Complete | Direct BCM2711 register access |
| Integration | ✅ Complete | AlertLed class wrapper |

### 8. Threading Architecture
| Component | Status | Details |
|-----------|--------|---------|
| POSIX Threads | ✅ Complete | tReadSensors, tCamera, actuator threads |
| Synchronization | ✅ Complete | Mutexes and condition variables |
| Priorities | ✅ Complete | 4-tier priority scheduling |
| IPC | ✅ Complete | Message queues for daemon communication |

---

## ⏳ Not Implemented (Mock Drivers)

The following hardware components have **mock implementations** that return simulated values. The code structure is complete and ready for real hardware integration.

### 1. Sensors (Mock Only)
| Sensor | Current Behavior | Hardware Required |
|--------|------------------|-------------------|
| DS18B20 (Temperature) | Returns random 18-26°C | 1-Wire on GPIO 19 |
| pH Sensor (PH-4502C) | Returns random 5.5-7.5 | Analog via ADS1115 ADC |
| TDS/EC Sensor | Returns random 800-1500 ppm | Analog via ADS1115 ADC |

### 2. Actuators (Mock Only)
| Actuator | Current Behavior | Hardware Required |
|----------|------------------|-------------------|
| pH Up Pump | Logs "Dosing for Xms" | GPIO 6 via relay |
| pH Down Pump | Logs "Dosing for Xms" | GPIO 13 via relay |
| Nutrient Pump | Logs "Dosing for Xms" | GPIO 5 via relay |
| Water Heater | Logs "ON/OFF" | GPIO 26 via SSR relay |

### 3. Additional Modules (Not Integrated)
| Module | Purpose | Interface |
|--------|---------|-----------|
| DS3231 RTC | Persistent timekeeping | I2C (address 0x68) |
| Relay Module | High-power device control | GPIO 26 |
| ADS1115 ADC | Analog-to-digital conversion | I2C (address 0x48) |

---

## Integration Guide

See [13-SENSOR-ACTUATOR-INTEGRATION.md](13-SENSOR-ACTUATOR-INTEGRATION.md) for complete instructions on replacing mock drivers with real hardware.

### Quick Integration Checklist:
- [ ] Enable 1-Wire overlay for DS18B20 temperature sensor
- [ ] Configure I2C and install ADS1115 for analog sensors
- [ ] Install libgpiod and update actuator drivers
- [ ] Connect DS3231 RTC for persistent time
- [ ] Add relay module for heater control
- [ ] Calibrate pH and TDS sensors with standard solutions

---

## Known Issues & Constraints

### 1. Touchscreen Calibration Persistence
- **Issue:** Qt evdev parameters must be set before each launch
- **Solution:** Parameters are set in `/opt/leafsense/start.sh`:
  ```bash
  export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=180:invertx"
  ```
- **Note:** This configuration works with dtoverlay=piscreen,rotate=270

### 2. Camera Strategy
- **Behavior:** The camera driver tries multiple capture methods:
  1. libcamera `cam` utility (preferred, works on Pi)
  2. libcamera-still fallback
  3. OpenCV V4L2 (for USB webcams)
  4. Test pattern (if no camera available)
- **Note:** The default OpenCV `VideoCapture` times out on Pi Camera; libcamera `cam` is required

### 3. Time Synchronization
- **Issue:** Pi has no RTC, time resets to 1970-01-01 on boot
- **Current:** Application uses whatever system time is available
- **Solution:** Integrate DS3231 RTC module and sync on boot

---

## File Locations on Raspberry Pi

| Path | Contents |
|------|----------|
| `/opt/leafsense/LeafSense` | Main application binary |
| `/opt/leafsense/start.sh` | Startup script with Qt environment |
| `/opt/leafsense/leafsense.db` | SQLite database |
| `/opt/leafsense/leafsense_model.onnx` | ML model |
| `/opt/leafsense/leafsense_model_classes.txt` | Class labels |
| `/opt/leafsense/gallery/` | Captured plant photos |
| `/usr/lib/libonnxruntime.so.1.16.3` | ONNX Runtime library |
| `/boot/config.txt` | Boot configuration (LCD overlay) |

---

## Buildroot Preservation

The Buildroot environment is fully configured and ready for future image rebuilds:

**Location:** `~/buildroot/buildroot-2025.08/`

**Key Files:**
- `.config` - Full menuconfig settings
- `output/images/sdcard.img` - Final flashable image
- `board/raspberrypi4-64/` - Custom configurations

**Rebuild Command:**
```bash
cd ~/buildroot/buildroot-2025.08
make clean  # Only if needed
make -j$(nproc)
# Output: output/images/sdcard.img
```

---

## Deployment Commands

### Start Application on Pi
```bash
ssh root@10.42.0.196 'cd /opt/leafsense && ./start.sh'
```

### Redeploy Binary
```bash
cd leafsense-project/build-arm64
make -j$(nproc)
sshpass -p 'leafsense' scp src/LeafSense root@10.42.0.196:/opt/leafsense/
```

### Access Pi
```bash
ssh root@10.42.0.196  # Password: leafsense
```

---

## Future Development Roadmap

### Phase 1: Hardware Integration (Priority)
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

---

## Conclusion

LeafSense is a complete, functional embedded system with full software implementation. The architecture, GUI, ML inference, and database are production-ready. The remaining work involves connecting real hardware sensors and actuators to replace the mock drivers—the code structure and control logic are already in place.

The system has been tested on actual Raspberry Pi 4 hardware with:
- ✅ Waveshare 3.5" LCD working at 480×320
- ✅ Touch input properly calibrated
- ✅ Camera capturing real photos
- ✅ ML predictions running
- ✅ Database logging all events
- ✅ Full GUI navigation working

**The project is ready for sensor/actuator hardware integration.**
