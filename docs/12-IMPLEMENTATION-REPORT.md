# LeafSense - Implementation Report

**Group 11**

**Authors:**
- Daniel Gonçalo Silva Cardoso, PG53753
- Marco Xavier Leite Costa, PG60210

**Specialization:** Embedded Systems and Computers  
**Program:** Master's in Industrial Electronics and Computers Engineering  
**Advisor:** Professor Adriano José Conceição Tavares  
**Date:** December 2025

---

## 4. Implementation

### 4.1 Development Environment

The development of the LeafSense project was carried out using the following tools and technologies:

| Component | Tool/Version |
|------------|-------------------|
| Operating System (Host) | Ubuntu 22.04 LTS |
| IDE | Visual Studio Code |
| Main Language | C++17 |
| Build System | CMake 3.22+ |
| Version Control | Git / GitHub |
| GUI Framework | Qt 5.15.14 |
| ML Framework (Training) | PyTorch 2.0 |
| ML Runtime (Inference) | ONNX Runtime 1.16.3 |
| Computer Vision | OpenCV 4.11.0 |
| Database | SQLite 3.48.0 |

For embedded development, **Buildroot 2025.08** was used as the build system to create a customized Linux distribution for the Raspberry Pi 4.

### 4.2 Cross-Compilation for Raspberry Pi

#### 4.2.1 Buildroot Configuration

Buildroot was configured from the base configuration `raspberrypi4_64_defconfig`, with the following customizations:

```bash
# Configuration base
make raspberrypi4_64_defconfig

# Customizations via menuconfig
make menuconfig
```

**Enabled options:**

| Categoria | Packages |
|-----------|---------|
| Toolchain | glibc, C++ support, GCC 14.3.0 |
| Qt5 | qt5base, qt5charts, qt5svg, qt5sql |
| Graphics | OpenCV 4.11.0 |
| Database | SQLite 3 |
| Networking | Dropbear SSH, dhcp client |
| Hardware | i2c-tools, 1-Wire support |
| Filesystem | ext4, 512MB root partition |

#### 4.2.2 Cross-Compilation Toolchain

A CMake toolchain file (`deploy/toolchain-rpi4.cmake`) was created to enable cross-compilation:

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN_PREFIX "~/buildroot/buildroot-2025.08/output/host")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/aarch64-linux-g++")
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")
```

Compilation is performed with:

```bash
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```

The resulting binary is approximately 380KB and is a 64-bit ELF executable for ARM aarch64.

### 4.3 Drivers Layer

#### 4.3.1 Kernel Module (LED Driver)

A Linux kernel module was developed for controlling an indicator LED via GPIO. The module implements a character device driver that exposes the device `/dev/led0`.

**Main characteristics:**

- Direct access to BCM2711 GPIO registers via `ioremap`
- GPIO base address: `0xFE200000`
- GPIO used: Pin 20
- Supported operations: `open`, `close`, `read`, `write`

**Code structure:**

```c
// GPIO Registers
#define GPIO_BASE   0xFE200000
#define GPFSEL2     0x08    // Function Select (GPIO 20-29)
#define GPSET0      0x1C    // Set Output High
#define GPCLR0      0x28    // Clear Output Low

// Map registers to virtual memory
gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);

// Configure GPIO as output
SetGPIOOutput(LED_PIN);

// Control LED
SetGPIOOutputValue(LED_PIN, value);  // 1=ON, 0=OFF
```

**Technical note:** The `ioremap_nocache` function was replaced with `ioremap` due to changes in the Linux kernel 5.6+ API.

Module compilation requires the kernel headers:

```bash
export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
export CROSS_COMPILE=~/buildroot/.../bin/aarch64-linux-
export ARCH=arm64
make
```

#### 4.3.2 Sensor Drivers

The sensor drivers were implemented following a common interface:

```cpp
class SensorInterface {
public:
    virtual double read() = 0;
    virtual bool calibrate() = 0;
    virtual std::string getName() = 0;
};
```

**Implemented sensors:**

| Sensor | Interface | Description |
|--------|-----------|-----------|
| DS18B20 | 1-Wire (GPIO4) | Solution temperature |
| pH Sensor | I2C | pH measurement (0-14) |
| EC Sensor | I2C | Electrical conductivity |

For testing and development, a mock mode was implemented that simulates realistic sensor readings.

#### 4.3.3 Actuator Drivers

The actuators (dosing pumps) are controlled through relays connected to GPIOs:

```cpp
class ActuatorInterface {
public:
    virtual void activate(int duration_ms) = 0;
    virtual void deactivate() = 0;
    virtual bool isActive() = 0;
};
```

**Implemented actuators:**

| Actuator | GPIO | Function |
|---------|------|--------|
| pH Up Pump | 12 | Increase pH |
| pH Down Pump | 13 | Decrease pH |
| Nutrient Pump | 16 | Add nutrients |

### 4.4 Middleware Layer

#### 4.4.1 Database (SQLite)

Data persistence is managed through SQLite, with the following schema:

**Main tables:**

| Table | Description |
|--------|-----------|
| `sensor_readings` | Temperature, pH, EC readings |
| `logs` | System events and actions |
| `alerts` | Alerts with severity |
| `ml_predictions` | ML inference results |
| `ml_detections` | Disease detections |
| `ml_recommendations` | Action recommendations |
| `plant` | Plant registry |
| `health_assessments` | Health assessments |

**Views for optimized queries:**

- `vw_latest_sensor_reading` - Latest reading
- `vw_daily_sensor_summary` - Daily summary
- `vw_unread_alerts` - Pending alerts
- `vw_pending_recommendations` - Active recommendations

#### 4.4.2 Main Controller (Master)

The main controller implements the business logic:

1. **Reading loop** - Periodic data acquisition from sensors
2. **Decision logic** - Pump activation based on thresholds
3. **Event management** - Alert and log generation
4. **Communication** - Bridge with the graphical interface via signals/slots

### 4.5 Application Layer

#### 4.5.1 Graphical Interface (Qt5)

The interface was developed in Qt5 with the following components:

| Component | Class | Function |
|------------|--------|--------|
| Main Window | `MainWindow` | Container with tabs |
| Sensors | `SensorsDisplay` | Real-time visualization |
| Health | `HealthDisplay` | ML results |
| Alerts | `AlertsDisplay` | Notification list |
| Analytics | `AnalyticsWindow` | Historical charts |
| Settings | `SettingsWindow` | System parameters |

**Visual characteristics:**

- Light/Dark mode themes
- Consistent colors (green #4CAF50 as primary color)
- Charts with Qt5Charts
- Responsive design

**Supported Qt platforms:**

| Platform | Usage |
|------------|-----|
| `xcb` | Desktop Linux (X11) |
| `linuxfb` | Raspberry Pi with HDMI |
| `offscreen` | Headless/testing mode |
| `vnc` | Remote access |

#### 4.5.2 Machine Learning System

##### Model

A model based on MobileNetV3-Small was trained for disease classification in tomato plants:

| Parameter | Value |
|-----------|-------|
| Architecture | MobileNetV3-Small (modified) |
| Input | 224×224×3 (RGB) |
| Output | 4 classes |
| Size | 5.9 MB (ONNX) |

**Classification classes:**

| Class | Description |
|--------|-----------|
| Healthy | Healthy plant |
| Bacterial_Spot | Bacterial spot |
| Early_Blight | Early Blight |
| Late_Blight | Late Blight |

##### Training

| Parameter | Value |
|-----------|-------|
| Dataset | PlantVillage (10,000 images) |
| Split | 80% training, 10% validation, 10% test |
| Epochs | 20 |
| Learning Rate | 0.001 → 0.0001 (scheduler) |
| Optimizer | Adam |
| Loss | CrossEntropyLoss |
| GPU | NVIDIA RTX 3070 |

**Data augmentation:**

- RandomResizedCrop
- RandomHorizontalFlip
- RandomRotation (±15°)
- ColorJitter

##### Results

| Metric | Value |
|---------|-------|
| **Accuracy** | 99.39% |
| Precision | 99.41% |
| Recall | 99.39% |
| F1-Score | 99.39% |

##### Inference

Inference on the Raspberry Pi is performed with ONNX Runtime:

```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // 4 cores
session_options.SetGraphOptimizationLevel(ORT_ENABLE_ALL);

Ort::Session session(env, "leafsense_model.onnx", session_options);
```

**Performance on Raspberry Pi 4:**

| Metric | Value |
|--------|-------|
| Inference time | ~150 ms |
| RAM usage | ~50 MB |
| CPU usage | ~80% (1 core) |

### 4.6 Deployment

#### 4.6.1 Image Preparation

The deployment process involves:

1. **Buildroot compilation** (~1-2 hours)
   ```bash
   cd ~/buildroot/buildroot-2025.08
   make -j$(nproc)
   ```

2. **LeafSense cross-compilation**
   ```bash
   cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
   make -j$(nproc)
   ```

3. **Kernel module compilation**
   ```bash
   cd drivers/kernel_module
   make arm64
   ```

#### 4.6.2 Installation on Raspberry Pi

1. **SD card flash**
   ```bash
   sudo dd if=output/images/sdcard.img of=/dev/sdX bs=4M
   ```

2. **Partition expansion**
   ```bash
   sudo parted /dev/sdX resizepart 2 100%
   sudo resize2fs /dev/sdX2
   ```

3. **File copy via SSH**
   ```bash
   scp build-arm/LeafSense root@10.42.0.196:/opt/leafsense/
   scp ml/leafsense_model.onnx root@10.42.0.196:/opt/leafsense/
   scp external/onnxruntime-arm64/lib/*.so* root@10.42.0.196:/usr/lib/
   ```

4. **Auto-start configuration**
   ```bash
   # /etc/init.d/S98leafsense
   export QT_QPA_PLATFORM=offscreen
   cd /opt/leafsense && ./LeafSense &
   ```

#### 4.6.3 Final File Structure

```
Raspberry Pi 4B
├── /opt/leafsense/
│   ├── LeafSense              # Application (380KB)
│   ├── leafsense_model.onnx   # ML Model (5.9MB)
│   ├── leafsense.db           # Database
│   └── schema.sql             # SQL Schema
├── /usr/lib/
│   ├── libonnxruntime.so*     # ONNX Runtime (16MB)
│   └── libQt5Charts.so*       # Qt5Charts (1.9MB)
├── /lib/modules/6.12.41-v8/
│   └── led.ko                 # Kernel module (13KB)
├── /etc/init.d/
│   └── S98leafsense           # Startup script
└── /var/log/
    └── leafsense.log          # Application logs
```

### 4.7 Validation

System validation was performed through:

| Test | Result |
|-------|-----------|
| Raspberry Pi boot | ✅ Success |
| SSH connectivity | ✅ root@10.42.0.196 |
| LED module loading | ✅ `/dev/led0` created |
| LED control | ✅ ON/OFF functional |
| Database initialization | ✅ 8 tables created |
| ONNX model loading | ✅ Model loaded |
| LeafSense application | ✅ Running |
| Sensor readings (mock) | ✅ Data being recorded |
| Auto-start on boot | ✅ S98leafsense |

**System metrics:**

| Metric | Value |
|---------|-------|
| RAM used | ~60 MB / 1.8 GB (3%) |
| Load average | 0.23 |
| Tested uptime | 20+ minutes |

### 4.8 Difficulties Encountered and Solutions

| Problem | Cause | Solution |
|----------|-------|---------|
| `ioremap_nocache` does not exist | API removed in kernel 5.6+ | Replace with `ioremap` |
| Qt5Charts not found | Not included in Buildroot by default | Add `BR2_PACKAGE_QT5CHARTS=y` and recompile |
| ONNX model does not load | Incorrect relative path | Copy model to `/opt/leafsense/` |
| DB tables do not exist | DB not initialized | Execute `sqlite3 leafsense.db < schema.sql` |
| Pi not found on network | DHCP did not assign IP | Use USB-Ethernet and fixed IP |
| Qt platform "eglfs" not available | Plugin not compiled | Use `QT_QPA_PLATFORM=offscreen` |

### 4.9 Future Work

The following features are planned for future versions:

1. **Real sensor integration** - Replace mocks with functional I2C/1-Wire drivers
2. **NTP synchronization** - Fix system date/time
3. **Camera interface** - Real-time capture for ML
4. **Web server** - Remote access via browser
5. **Push notifications** - Alerts via Telegram/Email
6. **Mobile dashboard** - Android/iOS application

---

## References

1. Qt Documentation - https://doc.qt.io/qt-5/
2. ONNX Runtime - https://onnxruntime.ai/
3. Buildroot Manual - https://buildroot.org/downloads/manual/manual.html
4. BCM2711 ARM Peripherals - Raspberry Pi Documentation
5. PlantVillage Dataset - https://plantvillage.psu.edu/
