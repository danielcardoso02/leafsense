# LeafSense - Terminology & Concepts Guide

**Version:** 1.5.1  
**Last Updated:** January 19, 2026

---

## Purpose

This document explains all technical terms, concepts, and processes used throughout the LeafSense project to help team members and reviewers understand the system.

---

## Table of Contents

1. [General Terminology](#general-terminology)
2. [Hardware Components](#hardware-components)
3. [Software Architecture](#software-architecture)
4. [Development & Deployment](#development--deployment)
5. [Machine Learning](#machine-learning)
6. [Database & Logging](#database--logging)
7. [Networking & Communication](#networking--communication)

---

## General Terminology

### **Embedded System**
A computer system designed for specific dedicated functions, often with real-time constraints. LeafSense is an embedded system that runs on Raspberry Pi hardware.

### **Real-Time System**
A system where timing is critical. LeafSense reads sensors every 2 seconds and must respond to threshold violations promptly.

### **Daemon**
A background process that runs continuously. The database daemon (`dDatabase`) processes messages from the queue without user interaction.

### **Middleware**
Software that connects different system components. In LeafSense, the Master controller acts as middleware between sensors, actuators, GUI, and database.

---

## Hardware Components

### **Raspberry Pi 4B**
Single-board computer (ARM64 architecture) that runs LeafSense. Specifications: ARM Cortex-A72 CPU, 2GB+ RAM, GPIO pins for hardware control.

### **Waveshare 3.5" LCD (C)**
- **Display:** 480×320 pixel touchscreen
- **Controller:** ILI9486 (display) + ADS7846 (touch)
- **Interface:** SPI communication
- **Framebuffer:** `/dev/fb1` (secondary framebuffer device)

### **GPIO (General Purpose Input/Output)**
Pins on Raspberry Pi used to control hardware:
- **Digital Output:** Turn LEDs on/off
- **PWM (Pulse Width Modulation):** Control heater power (0-100%)
- **1-Wire:** DS18B20 temperature sensor communication
- **Analog Input:** pH and EC sensors (via ADC)

### **Sensors**

#### **DS18B20 - Temperature Sensor**
- **Type:** Digital temperature sensor
- **Interface:** 1-Wire protocol
- **Range:** -55°C to +125°C
- **Accuracy:** ±0.5°C
- **Location:** `/sys/bus/w1/devices/28-*/temperature`

#### **pH Sensor**
- **Type:** Analog electrode
- **Range:** 0-14 pH
- **Interface:** Analog-to-Digital Converter (ADC)
- **Calibration:** Buffer solutions (pH 4, 7, 10)

#### **EC/TDS Sensor**
- **EC:** Electrical Conductivity (measures dissolved salts)
- **TDS:** Total Dissolved Solids (nutrient concentration)
- **Units:** µS/cm (microsiemens per centimeter) or ppm (parts per million)
- **Interface:** Analog via ADC

#### **Camera Module**
- **Model:** Raspberry Pi Camera Module v1 (OV5647)
- **Resolution:** 5MP (2592×1944), used at 640×480 for ML
- **Interface:** CSI (Camera Serial Interface)
- **Status:** Hardware not detected in current setup (using test patterns)

### **Actuators**

#### **Peristaltic Pump**
Dosing pump that moves precise amounts of liquid by squeezing tubing with rotating rollers. Used for pH adjustment and nutrient dosing.
- **Control:** Duration-based (milliseconds)
- **Example:** 500ms dose = ~5ml

#### **PWM Heater**
Water heater controlled by PWM (Pulse Width Modulation):
- **0% duty:** Off (cold)
- **50% duty:** Half power (warm)
- **100% duty:** Full power (hot)

#### **LED Alert**
Visual indicator controlled by kernel module:
- **On:** System alert or threshold violation
- **Off:** Normal operation

---

## Software Architecture

### **Qt5 Framework**
Cross-platform C++ library for GUI development:
- **Widgets:** Buttons, labels, charts
- **Event Loop:** Handles user input and timers
- **Signal/Slot:** Event notification mechanism
- **QPainter:** Graphics rendering

### **Framebuffer**
Direct memory access to display:
- **linuxfb:** Qt platform plugin for framebuffer rendering
- **/dev/fb1:** Waveshare LCD framebuffer device
- **RGB565:** 16-bit color format (5 bits red, 6 green, 5 blue)

### **OpenCV (Open Computer Vision)**
Library for image processing and computer vision:
- **cv::Mat:** Image data structure
- **Color Spaces:** BGR, RGB, Lab, HSV
- **CLAHE:** Contrast Limited Adaptive Histogram Equalization
- **Filters:** Gaussian blur, sharpening, edge detection

### **ONNX Runtime**
Machine learning inference engine:
- **ONNX:** Open Neural Network Exchange (model format)
- **Inference:** Running a trained model on new data (vs. training)
- **Session:** Loaded model ready for predictions

### **SQLite**
Lightweight embedded database:
- **No server:** Database is a single file
- **SQL:** Structured Query Language for queries
- **ACID:** Atomic, Consistent, Isolated, Durable transactions

### **Message Queue**
Inter-process communication mechanism:
- **Producer:** Components that send messages (sensors, GUI)
- **Consumer:** Database daemon that processes messages
- **Format:** `TYPE|data1|data2|data3` (pipe-delimited)

---

## Development & Deployment

### **Cross-Compilation**
Building software on one platform (x86 PC) for another platform (ARM64 Pi):
- **Host:** Development machine (your laptop/desktop) - typically x86_64 architecture
- **Target:** Deployment machine (Raspberry Pi) - ARM64/aarch64 architecture
- **Toolchain:** Set of compilers and libraries for target architecture
- **Why needed:** ARM and x86 are incompatible instruction sets - code must be compiled specifically for each

### **Architecture**
CPU instruction set type:
- **x86_64 (AMD64):** 64-bit Intel/AMD processors (typical PCs)
- **ARM64 (aarch64):** 64-bit ARM processors (Raspberry Pi 4)
- **Compatibility:** Executables are architecture-specific and not interchangeable

### **Toolchain**
Complete set of tools for compiling software:
- **Compiler:** `aarch64-linux-g++` (converts C++ to ARM64 machine code)
- **Linker:** Combines object files into executable
- **Standard Library:** ARM64 versions of libc, libstdc++
- **Headers:** Include files for target system
- **Location:** `/home/daniel/buildroot/buildroot-2025.08/output/host/`

### **Sysroot**
Root filesystem of target system used during cross-compilation:
- **Purpose:** Provides libraries and headers for target architecture
- **Location:** `/home/daniel/buildroot/buildroot-2025.08/output/host/aarch64-buildroot-linux-gnu/sysroot/`
- **Contains:** Target's `/usr/lib/`, `/usr/include/`, etc.
- **Why needed:** Compiler needs to link against ARM64 libraries, not host's x86_64 libraries

### **Native Build**
Building software on the same platform where it will run:
- **PC Build:** Compiling on x86_64 for x86_64 (for testing GUI on development machine)
- **No cross-compilation:** Uses system's default compiler and libraries
- **Faster development:** Immediate testing without deploying to Pi

### **Buildroot**
System for generating embedded Linux systems:
- **Kernel:** Linux 6.12.41-v8 (custom compiled for Raspberry Pi)
- **Root Filesystem:** Minimal Linux with only required components
- **Configuration:** `.config` files specify what to include
- **Output:** Complete bootable SD card image
- **Size:** ~289 MB (much smaller than full Raspberry Pi OS)

### **CMake**
Build system generator (creates Makefiles):
- **CMakeLists.txt:** Build configuration file (project structure, dependencies, compiler flags)
- **Targets:** Executables, libraries to build
- **Out-of-source build:** Build artifacts in separate directory (`build/` or `build-arm64/`)
- **Variables:**
  - `CMAKE_SOURCE_DIR` - Project root directory
  - `CMAKE_CURRENT_SOURCE_DIR` - Directory of current CMakeLists.txt
  - `PROJECT_SOURCE_DIR` - Directory where `project()` was called

### **Toolchain File**
CMake file that specifies cross-compilation settings:
- **Compiler paths:** Where to find ARM64 g++, gcc
- **System root:** Where ARM64 libraries are located
- **Example:** `toolchain-rpi4.cmake`
- **Usage:** `cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..`

### **Build Directory**
Separate directory for compilation output (keeps source clean):
- **build/:** PC build (x86_64) - for development/testing
- **build-arm64/:** ARM64 cross-compiled build - for Raspberry Pi deployment
- **Contains:** Object files (.o), Makefiles, executables
- **Advantage:** Can have multiple builds (Debug/Release, different architectures) simultaneously

### **MOC (Meta-Object Compiler)**
Qt tool that generates code for signals/slots:
- **Input:** Header files with Q_OBJECT macro
- **Output:** moc_*.cpp files with Qt metadata
- **Automatic:** CMake's AUTOMOC setting handles this
- **Generated files:** Found in `build*/LeafSense_autogen/`

### **RCC (Resource Compiler)**
Qt tool that embeds resources (images, fonts) into executable:
- **Input:** resources.qrc file
- **Output:** qrc_resources.cpp
- **Advantage:** No need to deploy separate image files
- **Automatic:** CMake's AUTORCC handles this

### **Deploy**
The process of copying the compiled binary to the Raspberry Pi and starting it:
```bash
scp build-arm64/src/LeafSense root@10.42.0.196:/opt/leafsense/
ssh root@10.42.0.196 'killall LeafSense && /opt/leafsense/LeafSense &'
```

### **SCP (Secure Copy)**
Command to copy files over SSH:
```bash
scp source_file user@host:/destination/path
```

### **SSH (Secure Shell)**
Remote terminal access:
```bash
ssh user@host 'command'
```
- **IP:** 10.42.0.196 (Pi's static IP on USB network)
- **User:** root (administrative access)

### **Static IP**
Fixed IP address that doesn't change:
- **LeafSense Pi:** 10.42.0.196
- **Host PC:** 10.42.0.1
- **Interface:** usb0 (USB-Ethernet gadget)

---

## Machine Learning

### **Model Training**
Process of teaching a neural network using labeled data:
- **Dataset:** 4-class plant disease images (healthy, nutrient deficiency, pest damage, disease)
- **Epochs:** Training iterations over entire dataset
- **Accuracy:** 99.39% on validation set

### **Inference**
Using a trained model to make predictions on new images:
- **Input:** 640×480 camera image
- **Preprocessing:** Resize, normalize, tensor conversion
- **Output:** Class probabilities (confidence scores)

### **Classes**
Categories the ML model can identify:
1. **Healthy** - Normal plant
2. **Nutrient Deficiency** - Yellowing, pale leaves
3. **Pest Damage** - Holes, spots, insect damage
4. **Disease** - Fungal/bacterial infections

### **Confidence**
Probability that the prediction is correct:
- **99.9%:** Very confident (typical for LeafSense)
- **<50%:** Uncertain, may need human review

### **CLAHE (Contrast Limited Adaptive Histogram Equalization)**
Image enhancement technique that improves contrast locally:
- **Adaptive:** Different enhancement per image region
- **Clip Limit:** Prevents over-amplification of noise
- **Result:** Better visibility of plant details for ML

---

## Database & Logging

### **Schema**
Database structure definition:
- **Tables:** sensor_readings, ml_detections, logs, etc.
- **Columns:** Field names and data types
- **Relationships:** Foreign keys linking tables

### **SQL (Structured Query Language)**
Language for database operations:
```sql
INSERT INTO sensor_readings (temperature, ph, ec) VALUES (23.4, 6.8, 1320);
SELECT * FROM sensor_readings WHERE timestamp > '2026-01-01';
```

### **Transaction**
Group of database operations that succeed or fail together:
- **Atomic:** All or nothing
- **Example:** Insert sensor reading + log entry (both must succeed)

### **Timestamp**
Date and time when data was recorded:
- **Format:** `YYYY-MM-DD HH:MM:SS`
- **Example:** `2026-01-10 02:15:30`

### **Log Levels**
Severity of log messages:
- **INFO:** Normal operation (sensor readings)
- **WARNING:** Potential issues (threshold violations)
- **ERROR:** Failures (database write error)
- **DEBUG:** Detailed diagnostic information

---

## Networking & Communication

### **USB Gadget Mode**
Raspberry Pi configured to act as USB Ethernet device:
- **Connection:** USB cable to PC
- **Network:** Direct Pi ↔ PC communication
- **No router needed:** Point-to-point connection

### **Dropbear**
Lightweight SSH server for embedded systems:
- **Alternative to:** OpenSSH (smaller, fewer features)
- **Port:** 22 (default SSH port)

### **Port Forwarding**
Redirecting network traffic from one port to another (not currently used in LeafSense).

### **Protocol**
Rules for communication between systems:
- **HTTP:** Web traffic
- **SSH:** Secure remote access
- **1-Wire:** Temperature sensor communication
- **SPI:** Display communication

---

## Build Process Terminology

### **Compilation**
Converting source code (.cpp) to machine code (.o):
```bash
g++ -c Temp.cpp -o Temp.o
```
- **Preprocessor:** Expands #include directives and macros
- **Compiler:** Converts C++ to assembly language
- **Assembler:** Converts assembly to machine code (object file)

### **Object File**
Compiled but not yet linked code:
- **Extension:** `.o` (Unix/Linux) or `.obj` (Windows)
- **Contents:** Machine code + symbol table (function names, variables)
- **Not executable:** Missing library code and entry point resolution

### **Linking**
Combining object files (.o) and libraries into final executable:
```bash
g++ Temp.o PHSensor.o main.o -o LeafSense -lonnxruntime -lopencv_core
```
- **Symbol Resolution:** Matches function calls to implementations
- **Address Assignment:** Determines final memory addresses
- **Output:** Executable binary (ELF format on Linux)

### **Library**
Collection of reusable compiled code:
- **Static (.a):** Embedded in final binary at link time
  - Advantage: No runtime dependencies
  - Disadvantage: Larger executable size
- **Shared (.so):** Loaded at runtime from separate file
  - Advantage: Smaller executable, shared between programs
  - Disadvantage: Must be present on target system
- **Example:** libonnxruntime.so (ONNX Runtime shared library)

### **Dependencies**
External libraries required by the program:
- **Build-time:** Libraries needed during compilation (headers + .so/.a files)
- **Runtime:** Libraries needed when program runs (.so files)
- **LeafSense deps:** Qt5, SQLite3, OpenCV, ONNX Runtime, pthread

### **Header File**
C++ file containing declarations (.h):
- **Purpose:** Tells compiler what functions/classes exist
- **Example:** `Temp.h` declares `Temp` class interface
- **Not compiled directly:** Included by .cpp files that use them

### **Include Path**
Where compiler looks for header files:
```bash
-I/path/to/headers
-I${CMAKE_SOURCE_DIR}/include
```
- **System paths:** `/usr/include/`, `/usr/local/include/`
- **Project paths:** `include/`, `include/drivers/sensors/`

### **Library Path**
Where linker looks for libraries:
```bash
-L/path/to/libs -lonnxruntime
```
- **-L:** Adds directory to library search path
- **-l:** Links specific library (libonnxruntime.so → `-lonnxruntime`)

### **RPATH**
Runtime library search path embedded in binary:
```bash
-Wl,-rpath,/opt/onnxruntime/lib
```
- **Purpose:** Tells dynamic linker where to find .so files at runtime
- **Alternative:** LD_LIBRARY_PATH environment variable
- **LeafSense:** Uses RPATH for ONNX Runtime location

### **Makefile**
Build script generated by CMake:
- **Targets:** Compilation steps (all, clean, LeafSense)
- **Dependencies:** Which files depend on which headers
- **Parallel build:** `make -j$(nproc)` uses all CPU cores
- **Incremental:** Only recompiles changed files

### **Clean Build**
Deleting all build artifacts and recompiling from scratch:
```bash
rm -rf build-arm64/
mkdir build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)
```
- **When needed:** After changing CMakeLists.txt or fixing build errors
- **Slower:** Recompiles everything, but ensures consistency

### **Incremental Build**
Recompiling only changed files:
```bash
make -j$(nproc)
```
- **Faster:** Only processes modified .cpp files
- **Risk:** May miss dependency changes (headers)

### **Build Type**
Compilation mode:
- **Debug:** Includes symbols for debugger, no optimization (-g -O0)
- **Release:** Optimized for speed, no debug info (-O3)
- **CMake option:** `-DCMAKE_BUILD_TYPE=Release`
- **Default:** Debug (for development)

### **Strip**
Removing debug symbols from binary to reduce size:
```bash
strip LeafSense
```
- **Effect:** 790K → ~300K (typical reduction)
- **Trade-off:** Can't debug stripped binary
- **When:** Production deployment, not development

---

## File System Locations

### **/opt/leafsense/**
Installation directory on Raspberry Pi:
- `LeafSense` - Main executable
- `database/` - SQLite database file
- `gallery/` - Captured images

### **/var/log/leafsense.log**
System log file (all console output redirected here)

### **/dev/**
Device files (hardware interfaces):
- `/dev/fb1` - Framebuffer (display)
- `/dev/video0` - Camera
- `/dev/input/event0` - Touchscreen
- `/dev/led0` - LED kernel module

### **/sys/**
Kernel information:
- `/sys/bus/w1/devices/` - 1-Wire devices (temperature sensor)
- `/sys/class/video4linux/` - Camera devices

---

## Common Commands

### **Build Commands**
```bash
# Configure build
cmake ../src -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake

# Compile (4 parallel jobs)
make -j4

# Clean build
make clean
```

### **Deployment Commands**
```bash
# Copy binary to Pi
scp build-arm64/LeafSense root@10.42.0.196:/opt/leafsense/

# Restart LeafSense
ssh root@10.42.0.196 'killall LeafSense && /opt/leafsense/LeafSense &'

# View logs
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log'
```

### **Database Commands**
```bash
# Connect to database
sqlite3 /opt/leafsense/database/leafsense.db

# Show tables
.tables

# Query sensor data
SELECT * FROM sensor_readings LIMIT 10;

# Exit
.quit
```

### **System Commands**
```bash
# Check running processes
ps aux | grep LeafSense

# Check camera
vcgencmd get_camera

# Check memory usage
free -h

# Check disk space
df -h
```

---

## Acronyms Quick Reference

- **ADC** - Analog-to-Digital Converter
- **API** - Application Programming Interface
- **ARM** - Advanced RISC Machine (CPU architecture)
- **BGR** - Blue-Green-Red (OpenCV color format)
- **CLAHE** - Contrast Limited Adaptive Histogram Equalization
- **CPU** - Central Processing Unit
- **CSI** - Camera Serial Interface
- **CV** - Computer Vision
- **EC** - Electrical Conductivity
- **GPIO** - General Purpose Input/Output
- **GUI** - Graphical User Interface
- **ISP** - Image Signal Processor
- **JPEG** - Joint Photographic Experts Group (image format)
- **ML** - Machine Learning
- **ONNX** - Open Neural Network Exchange
- **pH** - Potential of Hydrogen (acidity measure)
- **PWM** - Pulse Width Modulation
- **RGB** - Red-Green-Blue (color format)
- **RPATH** - Runtime Path (library search path)
- **SCP** - Secure Copy Protocol
- **SPI** - Serial Peripheral Interface
- **SQL** - Structured Query Language
- **SSH** - Secure Shell
- **TDS** - Total Dissolved Solids
- **UI** - User Interface
- **V4L2** - Video4Linux2 (camera API)

---

## Next Steps

For detailed information on specific topics, see:
- **Overview:** [01-OVERVIEW.md](01-OVERVIEW.md)
- **Architecture:** [02-ARCHITECTURE.md](02-ARCHITECTURE.md)
- **Machine Learning:** [03-MACHINE-LEARNING.md](03-MACHINE-LEARNING.md)
- **Buildroot:** [05-BUILDROOT-IMAGE.md](05-BUILDROOT-IMAGE.md)
- **Deployment:** [06-RASPBERRY-PI-DEPLOYMENT.md](06-RASPBERRY-PI-DEPLOYMENT.md)
- **Device Driver:** [07-DEVICE-DRIVER.md](07-DEVICE-DRIVER.md)
- **Database:** [08-DATABASE.md](08-DATABASE.md)
- **GUI:** [09-GUI.md](09-GUI.md)
- **Troubleshooting:** [10-TROUBLESHOOTING.md](10-TROUBLESHOOTING.md)

---

**Last Updated:** January 10, 2026  
**Maintained By:** Daniel Cardoso, Marco Costa
