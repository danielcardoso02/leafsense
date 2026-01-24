# LeafSense Project Status - Final Report

**Date:** January 23, 2026  
**Version:** 1.6.1  
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
| Model | ✅ Complete | MobileNetV3-Small trained on 4 classes |
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

## ⏳ Mock Sensor Drivers (Hardware Not Connected)

The following sensors have **mock implementations** that return simulated values when the physical hardware is not detected. The code automatically uses real values when hardware is connected.

### Sensors (Auto-Detect Real vs Mock)
| Sensor | Current Behavior | Real Mode Trigger |
|--------|------------------|-------------------|
| DS18B20 (Temperature) | Mock: random 15-25°C | 1-Wire device found at `/sys/bus/w1/devices/28-*` |
| pH Sensor (PH-4502C) | Mock: random 6.0-7.0 | ADS1115 ADC returns valid voltage on Channel 0 |
| TDS/EC Sensor | Mock: random 1200-1400ppm | ADS1115 ADC returns valid voltage on Channel 1 |

### Actuators (Real GPIO via libgpiod)
| Actuator | Current Behavior | Status |
|----------|------------------|--------|
| pH Up Pump (GPIO 6) | Real GPIO control | ✅ `[Pump GPIO6] Initialized successfully (libgpiod)` |
| pH Down Pump (GPIO 13) | Real GPIO control | ✅ `[Pump GPIO13] -> HIGH (ON)` verified |
| Nutrient Pump (GPIO 5) | Real GPIO control | ✅ `[Pump GPIO5] Initialized successfully (libgpiod)` |
| Water Heater (GPIO 26) | Real GPIO control | ✅ `[Heater] GPIO 26 initialized successfully (libgpiod)` |
| Alert LED | Real kernel module | ✅ `[LED] Alert LED -> ON/OFF` via `/dev/led0` |

### 3. I2C & GPIO Implementation Status (January 2026)
| Component | Status | Details |
|-----------|--------|---------|
| I2C Bus | ✅ Working | `/dev/i2c-1` enabled |
| i2c-dev Module | ✅ Auto-load | Added to `/etc/modules` |
| ADS1115 ADC | ✅ Driver Ready | Address 0x48, auto-fallback to mock if not connected |
| pH Sensor | ✅ I2C Ready | Channel 0, voltage-to-pH conversion |
| TDS Sensor | ✅ I2C Ready | Channel 1, voltage-to-ppm conversion |
| Heater GPIO 26 | ✅ Working | Real GPIO via libgpiod |
| pH Up GPIO 6 | ✅ Working | Real GPIO via libgpiod |
| pH Down GPIO 13 | ✅ Working | Real GPIO via libgpiod |
| Nutrient GPIO 5 | ✅ Working | Real GPIO via libgpiod |
| Alert LED | ✅ Working | Kernel module `/dev/led0` |

### 4. Additional Modules (Not Integrated)
| Module | Purpose | Interface |
|--------|---------|-----------|
| DS3231 RTC | Persistent timekeeping | I2C (address 0x68) - NTP used instead |

---

## Integration Guide

See [13-SENSOR-ACTUATOR-INTEGRATION.md](13-SENSOR-ACTUATOR-INTEGRATION.md) for complete instructions on replacing mock drivers with real hardware.

### Quick Integration Checklist:
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
- [ ] (Optional) Connect DS3231 RTC for persistent time (NTP works via Ethernet)

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

LeafSense is a complete, functional embedded system with full software implementation. The architecture, GUI, ML inference, database, and GPIO actuators are production-ready. Sensors operate in mock mode until physical hardware is connected—the code automatically detects and uses real sensors when available.

The system has been tested on actual Raspberry Pi 4 hardware with:
- ✅ Waveshare 3.5" LCD working at 480×320
- ✅ Touch input properly calibrated (rotate=180:invertx)
- ✅ Camera capturing real photos via libcamera
- ✅ ML predictions running with OOD detection
- ✅ Database logging all events
- ✅ Full GUI navigation working
- ✅ GPIO actuators controlling via libgpiod (Heater, Pumps)
- ✅ Alert LED via kernel module

**Test Pass Rate: 90% (73/81 tests)**

**The project is ready for physical sensor integration (ADS1115 ADC, DS18B20).**

---

## Evidence Collection (January 2026)

Comprehensive evidence has been collected for the academic report:

### Screenshots Captured
| Category | Count | Location |
|----------|-------|----------|
| GUI Screens | 10 | `docs/latex/images/gui_*.png` |
| LED Driver | 3 | `docs/latex/images/led_*.png` |
| Database | 4 | `docs/latex/images/db_*.png` |
| ML Pipeline | 3 | `docs/latex/images/ml_*.png` |
| Hardware Drivers | 4 | `docs/latex/images/*_evidence_*.png` |
| Test Evidence | 17 | `test_*.png` screenshots from Jan 22 session |
| **Total** | **41** | |

### Documentation Created
- `docs/latex/results-chapter.tex` - Complete LaTeX chapter with all evidence
- `docs/TEST-CASES-STATUS.md` - Analysis of 81 test cases from section 4.6
- `docs/VIDEO-RECORDING-SCRIPT.md` - Step-by-step demo recording guide

### Images Updated (January 22, 2026)
- `ml_captured_plant.jpg` - Real lettuce leaf captured by OV5647 camera
- `gui_analytics_gallery.png` - Remote framebuffer capture of gallery with lettuce images and recommendation panel

### Test Results Summary
Based on section 4.6 of the implementation report:
- **Total Test Cases:** 81
- **Passing:** 73 (90%)
- **Not Tested:** 8 (statistical analysis, bounding box, nutrient pump EC trigger)

**Test Categories at 100% Pass Rate:**
- System Setup & Basic Functionality
- Sensor Reading
- Image Capture
- ML Analysis & Predictions
- Actuator Control - Water Heater
- Actuator Control - pH Pump
- Actuator Control - Shared Features
- GUI Dashboard & Interactions
- GUI Logs & Theme
- GUI Settings & Configuration
- Data Persistence
- Basic Integration

**Latest Testing Session (January 23, 2026):**
- Fixed database query for health score (ml_predictions table)
- Removed ANSI color codes from terminal logs
- Verified all GPIO actuators working via libgpiod
- Pass rate increased from 81% to 90% (73/81 tests)

See `docs/TEST-CASES-STATUS.md` for detailed analysis.

