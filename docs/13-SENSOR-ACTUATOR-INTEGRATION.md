# LeafSense - Sensor & Actuator Integration Guide

## Overview

This guide explains how to integrate real hardware sensors and actuators into the LeafSense system, replacing the current mock implementations.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Sensor Integration](#sensor-integration)
3. [Actuator Integration](#actuator-integration)
4. [Additional Hardware Modules](#additional-hardware-modules)
   - [Real-Time Clock (RTC)](#1-real-time-clock-rtc-module-ds3231)
   - [Relay Module](#2-relay-module-for-power-control)
5. [Testing & Calibration](#testing--calibration)
6. [Troubleshooting](#troubleshooting)

---

## Architecture Overview

### Current Implementation

The system currently uses **mock drivers** that return simulated values. All sensor/actuator code is located in:

```
drivers/
├── sensors/
│   ├── Temp.cpp/.h      # DS18B20 temperature sensor
│   ├── PH.cpp/.h        # pH sensor via ADC
│   ├── TDS.cpp/.h       # TDS/EC sensor via ADC
│   ├── ADC.cpp/.h       # ADS1115 ADC interface
│   └── Cam.cpp/.h       # Camera (OV5647)
└── actuators/
    ├── Pumps.cpp/.h     # Dosing pumps
    ├── Heater.cpp/.h    # Water heater
    └── AlertLed.cpp/.h  # Status LED (kernel module)
```

### Integration Points

The `Master` class (middleware) coordinates all sensors and actuators:
- **Location**: `src/middleware/Master.cpp`
- **Thread**: `tReadSensorsFunc()` reads sensors every 2 seconds
- **Control Logic**: Automatically triggers actuators based on sensor readings

---

## Sensor Integration

### 1. Temperature Sensor (DS18B20)

**Current Mock**: Returns random value between 18-26°C

**Hardware Setup**:
- Interface: 1-Wire (GPIO 19)
- Device path: `/sys/bus/w1/devices/28-*/w1_slave`

**Implementation Steps**:

1. **Enable 1-Wire in Buildroot**:
   ```bash
   # In Buildroot menuconfig:
   # Kernel → Device Drivers → Dallas's 1-wire support → Enable
   # Kernel → Device Drivers → 1-Wire master → GPIO 1-wire bitbang → Enable
   ```

2. **Update `/boot/config.txt`**:
   ```
   dtoverlay=w1-gpio,gpiopin=19
   ```

3. **Modify `src/drivers/sensors/Temp.cpp`**:
   ```cpp
   float Temp::readSensor() {
       // Replace mock code with:
       std::ifstream file("/sys/bus/w1/devices/28-*/w1_slave");
       std::string line;
       
       // Read until t= line
       while (std::getline(file, line)) {
           size_t pos = line.find("t=");
           if (pos != std::string::npos) {
               int raw = std::stoi(line.substr(pos + 2));
               return raw / 1000.0f;  // Convert to Celsius
           }
       }
       
       return -999.0f;  // Error value
   }
   ```

4. **Test**:
   ```bash
   cat /sys/bus/w1/devices/28-*/w1_slave
   # Should output: t=23125 (23.125°C)
   ```

---

### 2. pH Sensor

**Current Mock**: Returns random value between 5.5-7.5

**Hardware Setup**:
- Interface: I2C via ADS1115 ADC (address 0x48)
- ADC Channel: 0
- Calibration: pH 4.0, pH 7.0, pH 10.0 solutions required

**Implementation Steps**:

1. **Enable I2C in Buildroot**:
   ```bash
   # Kernel → Device Drivers → I2C support → Enable
   # Add to /boot/config.txt:
   dtparam=i2c_arm=on
   i2c_arm_baudrate=100000
   ```

2. **Update `src/drivers/sensors/ADC.cpp`**:
   ```cpp
   #include <linux/i2c-dev.h>
   #include <sys/ioctl.h>
   #include <fcntl.h>
   
   int ADC::readChannel(int channel) {
       int fd = open("/dev/i2c-1", O_RDWR);
       if (fd < 0) {
           std::cerr << "[ADC] Cannot open I2C device" << std::endl;
           return -1;
       }
       
       if (ioctl(fd, I2C_SLAVE, address) < 0) {
           std::cerr << "[ADC] Cannot set I2C address" << std::endl;
           close(fd);
           return -1;
       }
       
       // ADS1115 configuration
       uint16_t config = 0xC383;  // Single-shot, channel 0, ±4.096V
       config |= (channel << 12);
       
       uint8_t writeBuf[3] = {0x01, (uint8_t)(config >> 8), (uint8_t)(config & 0xFF)};
       write(fd, writeBuf, 3);
       
       usleep(10000);  // Wait for conversion
       
       // Read result
       uint8_t readBuf[2];
       read(fd, readBuf, 2);
       
       close(fd);
       return (readBuf[0] << 8) | readBuf[1];
   }
   ```

3. **Update `src/drivers/sensors/PH.cpp`**:
   ```cpp
   float PH::readSensor() {
       int adcValue = adc->readChannel(channel);
       
       if (adcValue < 0) return -1.0f;
       
       // Convert ADC to voltage (ADS1115: 16-bit, ±4.096V range)
       float voltage = (adcValue * 4.096) / 32768.0;
       
       // Apply calibration (example values - MUST calibrate!)
       // pH = slope * voltage + offset
       float slope = -5.70;      // From calibration
       float offset = 21.34;     // From calibration
       
       return slope * voltage + offset;
   }
   ```

4. **Calibration Procedure**:
   ```bash
   # 1. Immerse probe in pH 7.0 solution
   # 2. Record voltage: V7
   # 3. Immerse in pH 4.0 solution
   # 4. Record voltage: V4
   # 5. Calculate: slope = (4.0 - 7.0) / (V4 - V7)
   # 6. Calculate: offset = 7.0 - (slope * V7)
   ```

---

### 3. TDS/EC Sensor

**Current Mock**: Returns random value between 800-1500 ppm

**Hardware Setup**:
- Interface: I2C via ADS1115 ADC (address 0x48)
- ADC Channel: 1
- Calibration: TDS calibration solution (e.g., 1413 μS/cm) required

**Implementation Steps**:

1. **Update `src/drivers/sensors/TDS.cpp`**:
   ```cpp
   float TDS::readSensor() {
       int adcValue = adc->readChannel(channel);
       
       if (adcValue < 0) return -1.0f;
       
       float voltage = (adcValue * 4.096) / 32768.0;
       
       // Temperature compensation (assume 25°C for now)
       float compensationCoefficient = 1.0 + 0.02 * (25.0 - 25.0);
       float compensationVoltage = voltage / compensationCoefficient;
       
       // TDS calculation (example - MUST calibrate!)
       float tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
                       - 255.86 * compensationVoltage * compensationVoltage
                       + 857.39 * compensationVoltage) * 0.5;
       
       return tdsValue;
   }
   ```

2. **Calibration Procedure**:
   ```bash
   # 1. Use known TDS solution (e.g., 1413 μS/cm)
   # 2. Measure voltage
   # 3. Calculate calibration factor = known_TDS / calculated_TDS
   # 4. Multiply result by calibration factor
   ```

---

### 4. Camera (Already Implemented)

**Status**: ✅ Hardware detected, driver implemented

**Hardware**: OV5647 Raspberry Pi Camera Module v1
**Device**: `/dev/video0`
**Capture**: Automatic every 30 minutes with ML analysis

**Current Issue**: OpenCV VideoCapture not opening camera
**Workaround Needed**: Use system command to capture

**Alternative Implementation** (if OpenCV fails):
```cpp
std::string Cam::takePhoto() {
    // Generate filename
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    std::ostringstream filename;
    filename << "/opt/leafsense/gallery/plant_"
             << std::put_time(timeinfo, "%Y%m%d_%H%M%S")
             << ".jpg";
    
    // Use libcamera-still (if available) or ffmpeg
    std::string cmd = "libcamera-still -o " + filename.str() + " -t 1000 --width 640 --height 480";
    int result = system(cmd.c_str());
    
    if (result == 0) {
        std::cout << "[Camera] Photo captured: " << filename.str() << std::endl;
        return filename.str();
    } else {
        std::cerr << "[Camera] Capture failed" << std::endl;
        return "";
    }
}
```

---

## Actuator Integration

### 1. Dosing Pumps (pH Up, pH Down, Nutrients)

**Current Mock**: Prints log messages, doesn't control GPIO

**Hardware Setup**:
- Interface: GPIO via relay modules
- Pins:
  - pH Up Pump: GPIO 6
  - pH Down Pump: GPIO 13
  - Nutrient Pump: GPIO 5

**Implementation Steps**:

1. **Enable GPIO in Buildroot**:
   ```bash
   # Add to package selection:
   # Target packages → Hardware handling → libgpiod
   ```

2. **Update `src/drivers/actuators/Pumps.cpp`**:
   ```cpp
   #include <gpiod.h>
   
   class Pumps {
   private:
       int pin;
       struct gpiod_chip *chip;
       struct gpiod_line *line;
       bool isActive;
   
   public:
       Pumps(int gpio_pin) : pin(gpio_pin), isActive(false) {
           chip = gpiod_chip_open("/dev/gpiochip0");
           if (!chip) {
               std::cerr << "[Pump] Cannot open GPIO chip" << std::endl;
               return;
           }
           
           line = gpiod_chip_get_line(chip, pin);
           if (!line) {
               std::cerr << "[Pump] Cannot get GPIO line " << pin << std::endl;
               return;
           }
           
           gpiod_line_request_output(line, "leafsense_pump", 0);
       }
       
       void activate(int durationMs) {
           if (!line) return;
           
           std::cout << "[Pump " << pin << "] Dosing for " << durationMs << "ms" << std::endl;
           gpiod_line_set_value(line, 1);  // Activate relay
           isActive = true;
           
           usleep(durationMs * 1000);
           
           gpiod_line_set_value(line, 0);  // Deactivate relay
           isActive = false;
       }
       
       ~Pumps() {
           if (line) gpiod_line_release(line);
           if (chip) gpiod_chip_close(chip);
       }
   };
   ```

3. **Safety Features** (already in Master.cpp):
   - Maximum dose: 500ms per activation
   - Cooldown period between doses
   - Override via `sensorsCorrecting` flag

---

### 2. Water Heater

**Current Mock**: Prints state changes

**Hardware Setup**:
- Interface: GPIO 26 via SSR (Solid State Relay)
- Load: 100-200W heating element

**Implementation Steps**:

1. **Update `src/drivers/actuators/Heater.cpp`**:
   ```cpp
   #include <gpiod.h>
   
   class Heater {
   private:
       int pin;
       struct gpiod_chip *chip;
       struct gpiod_line *line;
       bool state;
   
   public:
       Heater(int gpio_pin) : pin(gpio_pin), state(false) {
           chip = gpiod_chip_open("/dev/gpiochip0");
           line = gpiod_chip_get_line(chip, pin);
           gpiod_line_request_output(line, "leafsense_heater", 0);
       }
       
       void toggle() {
           state = !state;
           gpiod_line_set_value(line, state ? 1 : 0);
           std::cout << "[Heater] State: " << (state ? "ON" : "OFF") << std::endl;
       }
       
       bool getState() { return state; }
       
       ~Heater() {
           gpiod_line_set_value(line, 0);  // Ensure OFF
           gpiod_line_release(line);
           gpiod_chip_close(chip);
       }
   };
   ```

2. **Control Logic** (already implemented in Master.cpp):
   - Turns ON when temp < ideal_min
   - Turns OFF when temp > ideal_max
   - Hysteresis prevents rapid cycling

---

### 3. Alert LED (Kernel Module)

**Status**: ✅ Kernel module implemented

**Hardware**: LED on GPIO (pin TBD)
**Module**: `drivers/kernel_module/ledmodule.c`

**Current Integration**:

The LED kernel module is already compiled and can be loaded:

```bash
# Load module
insmod /lib/modules/$(uname -r)/extra/ledmodule.ko

# Control LED
echo 1 > /dev/led0  # Turn ON
echo 0 > /dev/led0  # Turn OFF
```

**Integration with Alerts** (add to Master.cpp):

```cpp
void Master::updateAlertLED() {
    // Determine if any alerts are active
    bool hasAlert = false;
    
    float phRange[2], tempRange[2], tdsRange[2];
    idealConditions->getPH(phRange);
    idealConditions->getTemp(tempRange);
    idealConditions->getTDS(tdsRange);
    
    float currentPH = phSensor->readSensor();
    float currentTemp = tempSensor->readSensor();
    float currentTDS = tdsSensor->readSensor();
    
    if (currentPH < phRange[0] || currentPH > phRange[1]) hasAlert = true;
    if (currentTemp < tempRange[0] || currentTemp > tempRange[1]) hasAlert = true;
    if (currentTDS < tdsRange[0] || currentTDS > tdsRange[1]) hasAlert = true;
    
    // Write to LED device
    int fd = open("/dev/led0", O_WRONLY);
    if (fd >= 0) {
        char state = hasAlert ? '1' : '0';
        write(fd, &state, 1);
        close(fd);
    }
}
```

Call `updateAlertLED()` in the `tReadSensorsFunc()` loop after reading sensors.

---

## Testing & Calibration

### 1. Sensor Testing

**Individual Sensor Test**:
```bash
# Temperature
cat /sys/bus/w1/devices/28-*/w1_slave

# I2C devices
i2cdetect -y 1

# ADC raw values
i2cget -y 1 0x48 0x00 w
```

**LeafSense Log Monitoring**:
```bash
tail -f /var/log/leafsense.log | grep SENSOR
```

### 2. Actuator Testing

**GPIO Test**:
```bash
# Export GPIO
echo 13 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio13/direction

# Test
echo 1 > /sys/class/gpio/gpio13/value  # ON
sleep 1
echo 0 > /sys/class/gpio/gpio13/value  # OFF
```

**Pump Test**:
- Manually trigger via GUI or modify `Master.cpp` to activate on demand
- Verify pump runs for specified duration
- Check flow rate and adjust timing

### 3. Calibration Storage

Store calibration constants in configuration file:

**Create** `/opt/leafsense/calibration.conf`:
```ini
[pH]
slope=-5.70
offset=21.34

[TDS]
factor=0.85

[Temperature]
offset=0.0
```

Load in drivers using standard C++ file parsing.

---

## Troubleshooting

### Issue: Sensor reads -999 or NaN

**Cause**: Hardware not connected or driver error

**Solution**:
1. Check physical connections
2. Verify I2C/1-Wire enabled in kernel
3. Check `dmesg` for driver errors
4. Test with command-line tools first

### Issue: Actuator doesn't respond

**Cause**: GPIO not configured or relay not connected

**Solution**:
1. Test GPIO manually (see GPIO Test above)
2. Check relay power supply
3. Verify GPIO pin number matches schematic
4. Measure voltage at GPIO pin

### Issue: Erratic readings

**Cause**: Electrical noise or poor connections

**Solution**:
1. Add decoupling capacitors
2. Use shielded cables
3. Separate power supplies for sensors and actuators
4. Software filtering (moving average)

### Issue: LED module not loading

**Cause**: Module not compiled for correct kernel

**Solution**:
```bash
# Rebuild module
cd drivers/kernel_module
make clean
make

# Check kernel version matches
uname -r
ls /lib/modules/$(uname -r)/extra/
```

---

## Additional Hardware Modules

### 1. Real-Time Clock (RTC) Module (DS3231)

**Purpose**: Maintain accurate time when Pi is powered off

**Hardware Interface**: I2C (GPIO 2 = SDA, GPIO 3 = SCL)

**Setup**:

1. **Enable I2C in Buildroot**:
   ```bash
   # In Buildroot menuconfig:
   # Kernel → Device Drivers → I2C support → Enable
   # Kernel → Device Drivers → I2C Hardware Drivers → Raspberry Pi → Enable
   ```

2. **Update `/boot/config.txt`**:
   ```
   dtparam=i2c_arm=on
   dtparam=i2c1=on
   ```

3. **Verify Device**:
   ```bash
   i2cdetect -y 1
   # Should show device at address 0x68 (DS3231)
   ```

4. **Set System Time from RTC**:
   ```bash
   # Install hwclock
   hwclock -r          # Read RTC
   hwclock -w          # Write system time to RTC
   hwclock -s          # Set system time from RTC
   ```

5. **Automatic Sync (in `/etc/rc.local`)**:
   ```bash
   #!/bin/sh -e
   hwclock -s
   ntpd -S /usr/sbin/ntpdate-sync
   exit 0
   ```

**Integration in LeafSense**:
- Timestamps for all sensor readings
- Photo capture timing
- Scheduled maintenance tasks
- Data synchronization with external servers

---

### 2. Relay Module (for Power Control)

**Purpose**: Control high-power devices (heater, pump backup, etc.)

**Hardware Interface**: GPIO (configurable pin, typically GPIO 26)

**Setup**:

1. **GPIO Configuration**:
   ```bash
   # Identify available GPIO pins
   cat /sys/kernel/debug/gpio
   
   # Export GPIO for control
   echo 26 > /sys/class/gpio/export
   echo out > /sys/class/gpio/gpio26/direction
   ```

2. **Control Relay**:
   ```bash
   # Turn ON
   echo 1 > /sys/class/gpio/gpio26/value
   
   # Turn OFF
   echo 0 > /sys/class/gpio/gpio26/value
   
   # Check status
   cat /sys/class/gpio/gpio26/value
   ```

3. **Create C++ Wrapper Class**:
   ```cpp
   // include/drivers/actuators/Relay.h
   #ifndef RELAY_H
   #define RELAY_H
   
   #include <string>
   
   class Relay {
   private:
       int gpio_pin;
       std::string gpio_path;
       
   public:
       Relay(int pin = 26);
       ~Relay();
       
       bool initialize();
       bool activate();
       bool deactivate();
       bool getStatus() const;
       void cleanup();
   };
   
   #endif
   ```

4. **Implementation Example** (`src/drivers/actuators/Relay.cpp`):
   ```cpp
   #include "../../include/drivers/actuators/Relay.h"
   #include <fstream>
   #include <filesystem>
   
   Relay::Relay(int pin) : gpio_pin(pin) {
       gpio_path = "/sys/class/gpio/gpio" + std::to_string(pin);
   }
   
   bool Relay::initialize() {
       std::ofstream export_file("/sys/class/gpio/export");
       export_file << gpio_pin;
       export_file.close();
       
       std::ofstream direction("/sys/class/gpio/gpio" + std::to_string(gpio_pin) + "/direction");
       direction << "out";
       direction.close();
       
       return std::filesystem::exists(gpio_path);
   }
   
   bool Relay::activate() {
       std::ofstream value(gpio_path + "/value");
       value << "1";
       value.close();
       return true;
   }
   
   bool Relay::deactivate() {
       std::ofstream value(gpio_path + "/value");
       value << "0";
       value.close();
       return true;
   }
   
   bool Relay::getStatus() const {
       std::ifstream value(gpio_path + "/value");
       std::string state;
       value >> state;
       return state == "1";
   }
   
   void Relay::cleanup() {
       std::ofstream unexport("/sys/class/gpio/unexport");
       unexport << gpio_pin;
       unexport.close();
   }
   
   Relay::~Relay() {
       cleanup();
   }
   ```

5. **Integration with Master.cpp**:
   ```cpp
   // In middleware/Master.cpp
   Relay backup_pump_relay(26);
   
   void Master::initializeActuators() {
       backup_pump_relay.initialize();
   }
   
   void Master::controlRelayBasedOnSensors() {
       // Example: activate backup pump if main pump fails
       if (should_activate_backup) {
           backup_pump_relay.activate();
       } else {
           backup_pump_relay.deactivate();
       }
   }
   ```

**Use Cases**:
- Pump backup power control
- Emergency shutdown
- Heater auxiliary power
- Water level pump triggering

---

## Integration Checklist

Before deployment:

- [ ] All sensors return valid readings
- [ ] pH calibrated with 3-point calibration
- [ ] TDS calibrated with standard solution
- [ ] Temperature sensor within ±0.5°C of reference
- [ ] All pumps activate and stop correctly
- [ ] Pump timing matches desired dose volume
- [ ] Heater toggles based on temperature threshold
- [ ] LED responds to alert conditions
- [ ] **RTC module maintaining accurate time**
- [ ] **RTC syncs on system startup**
- [ ] **Relay responds to GPIO commands**
- [ ] **Relay safely controls high-power devices**
- [ ] Database logs all sensor readings
- [ ] ML captures photos every 30 minutes
- [ ] System runs stable for 24+ hours

---

## Next Steps

1. **Hardware Setup**: Connect all sensors and actuators according to schematic
2. **Module Build**: Compile drivers with real hardware code
3. **Individual Testing**: Test each sensor/actuator independently
4. **Integration**: Build and deploy LeafSense with real drivers
5. **Calibration**: Perform calibration procedures
6. **Validation**: Run system for 24 hours and verify data
7. **Optimization**: Tune control parameters (thresholds, timing)

---

**Authors**: Daniel Cardoso, Marco Costa  
**Last Updated**: January 10, 2026
