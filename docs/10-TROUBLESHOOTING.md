# LeafSense - Troubleshooting Guide

## Table of Contents

1. [Compilation Problems](#compilation-problems)
2. [Cross-Compilation Problems](#cross-compilation-problems)
3. [Runtime Problems](#runtime-problems)
4. [Network Problems](#network-problems)
5. [Hardware Problems](#hardware-problems)
6. [Database Problems](#database-problems)
7. [Machine Learning Problems](#machine-learning-problems)
8. [Touchscreen Problems](#touchscreen-problems)
9. [Useful Logs](#useful-logs)

---

## Compilation Problems

### Qt5 not found
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**Solution:**
```bash
# Ubuntu/Debian
sudo apt install qt5-default qtcharts5-dev libqt5svg5-dev libqt5sql5-sqlite

# Or specify path explicitly
cmake -DQt5_DIR=/path/to/qt5/lib/cmake/Qt5 ..
```

### OpenCV not found
```
CMake Error: Could not find OpenCV
```

**Solution:**
```bash
# Ubuntu/Debian
sudo apt install libopencv-dev

# Or compile from source
git clone https://github.com/opencv/opencv.git
cd opencv && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc) && sudo make install
```

### C++17 compilation error
```
error: 'filesystem' is not a namespace-name
```

**Solution:**
```cmake
# CMakeLists.txt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# And add to linker
target_link_libraries(LeafSense PRIVATE stdc++fs)
```

---

## Cross-Compilation Problems

### Toolchain not found
```
CMake Error: CMAKE_C_COMPILER not found
```

**Solution:**
```bash
# Verify Buildroot was compiled
ls ~/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-gcc

# If it doesn't exist, compile Buildroot
cd ~/buildroot/buildroot-2025.08
make raspberrypi4_64_defconfig
make -j$(nproc)
```

### Invalid sysroot
```
cannot find -lQt5Core
```

**Solution:**
```cmake
# Verify CMAKE_SYSROOT in toolchain file
set(CMAKE_SYSROOT "${TOOLCHAIN_PREFIX}/aarch64-buildroot-linux-gnu/sysroot")

# Verify libraries exist
ls ${CMAKE_SYSROOT}/usr/lib/libQt5*.so
```

### ONNX Runtime ARM64 error
```
undefined reference to `OrtGetApiBase'
```

**Solution:**
```bash
# Verify ARM64 version
file external/onnxruntime-arm64/lib/libonnxruntime.so
# Should show: ELF 64-bit LSB shared object, ARM aarch64

# Verify linkage in CMakeLists.txt
grep ONNXRUNTIME CMakeLists.txt
```

### ioremap_nocache does not exist
```
error: implicit declaration of function 'ioremap_nocache'
```

**Solution (kernel 5.6+):**
```c
// Before
gpio_base = ioremap_nocache(GPIO_BASE, GPIO_SIZE);

// After
gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
```

---

## Runtime Problems

### Library not found
```
error while loading shared libraries: libQt5Charts.so.5: cannot open
```

**Solution:**
```bash
# Verify available libraries
ssh root@10.42.0.196 "ls /usr/lib/libQt5*"

# Copy missing library
scp ~/buildroot/buildroot-2025.08/output/target/usr/lib/libQt5Charts.so.5.15.14 \
    root@10.42.0.196:/usr/lib/

# Create symlinks
ssh root@10.42.0.196 "cd /usr/lib && \
    ln -sf libQt5Charts.so.5.15.14 libQt5Charts.so.5 && \
    ln -sf libQt5Charts.so.5.15.14 libQt5Charts.so"
```

### Qt platform plugin not found
```
qt.qpa.plugin: Could not find the Qt platform plugin "eglfs"
```

**Solution:**
```bash
# Use linuxfb for Waveshare 3.5" (fb1)
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1

# Or for HDMI (fb0)
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0

# Or offscreen (no display)
export QT_QPA_PLATFORM=offscreen

# See available plugins
ls /usr/lib/qt/plugins/platforms/
```

### Segmentation fault on startup
```
Segmentation fault
```

**Solution:**
```bash
# Run with GDB (if available)
gdb ./LeafSense
(gdb) run
(gdb) bt

# Verify dependencies
ldd ./LeafSense

# Verify architecture
file ./LeafSense
```

### Insufficient permissions
```
Permission denied: /dev/led0
```

**Solution:**
```bash
# Run as root
sudo ./LeafSense

# Or adjust permissions
sudo chmod 666 /dev/led0

# Or create udev rule
echo 'KERNEL=="led0", MODE="0666"' | sudo tee /etc/udev/rules.d/99-led.rules
```

---

## Network Problems

### Pi not found on network
```
ssh: connect to host 10.42.0.196 port 22: No route to host
```

**Solution:**
```bash
# Verify network interface
ip link show

# Scan for Pi on network
arp-scan --localnet

# Verify USB-Ethernet cable
dmesg | grep -i eth

# Use static IP on Pi
# Add to /etc/network/interfaces:
# auto eth0
# iface eth0 inet static
# address 10.42.0.196
# netmask 255.255.255.0
```

### SSH connection refused
```
Connection refused
```

**Solution:**
```bash
# Verify SSH is running on Pi
ssh root@10.42.0.196 "ps aux | grep dropbear"

# Start SSH if not running
/etc/init.d/S50dropbear start

# Verify port 22 is open
netstat -tlnp | grep 22
```

### Connection timeout
```
Connection timed out
```

**Solution:**
```bash
# Verify physical connection
ping 10.42.0.196

# Check firewall
iptables -L

# Try direct cable connection
ip addr add 10.42.0.1/24 dev enx00e04c3601a6
```

---

## Hardware Problems

### LED kernel module doesn't load
```
insmod: ERROR: could not insert module led.ko: Invalid module format
```

**Solution:**
```bash
# Verify kernel version
uname -r

# Recompile module for correct kernel
export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
make clean && make
```

### Sensors not responding
```
[Sensor] Error: No response from pH sensor
```

**Solution:**
```bash
# Verify I2C devices
i2cdetect -y 1

# Check connections
# Verify sensor power supply
# Check wiring for shorts
```

### Camera Not Detected
```
vcgencmd get_camera
supported=0 detected=0, libcamera interfaces=0
```

**Diagnosis:**
- `supported=0` = GPU firmware doesn't support camera (config.txt issue)
- `detected=0` = Camera hardware not physically detected

**Solution - Check config.txt:**
```bash
# Mount boot partition
mount /dev/mmcblk0p1 /mnt/boot

# Verify camera is enabled
cat /mnt/boot/config.txt | grep -E "start_x|gpu_mem|ov5647"
```

Required settings in `/mnt/boot/config.txt`:
```ini
# Enable camera support
start_x=1
gpu_mem=128

# Camera driver (OV5647 = Pi Camera v1)
dtoverlay=ov5647
```

**Solution - Check Hardware:**
1. **Ribbon cable orientation**: Blue side toward Ethernet port on Pi
2. **Secure connection**: Both ends of ribbon must be fully seated
3. **Clean contacts**: Gently clean cable contacts with isopropyl alcohol
4. **Try different cable**: 15-pin flex cables are fragile
5. **Try different camera**: Module may be defective

**Verify camera module:**
```bash
# Check kernel messages for camera
dmesg | grep -i "ov5647\|camera\|csi"

# Look for errors like:
# "failed to open vchiq instance" = GPU firmware issue
# "ov5647: chip id mismatch" = Wrong camera model
# "Fixed dependency cycle" = Device tree issue (usually harmless)
```

**If still not working:**
- Ensure `start4.elf` and `fixup4.dat` are present in boot partition
- Try Camera v2 (IMX219) with `dtoverlay=imx219` instead

### GPIO access denied
```
[GPIO] Error: Cannot access GPIO
```

**Solution:**
```bash
# Verify permissions
ls -la /dev/gpiomem

# Create udev rule
echo 'KERNEL=="gpiomem", MODE="0666"' | sudo tee /etc/udev/rules.d/99-gpio.rules
udevadm control --reload-rules
```

---

## Database Problems

### Tables do not exist
```
[DB] Error: no such table: sensor_readings
```

**Solution:**
```bash
# Initialize database
cd /opt/leafsense
sqlite3 leafsense.db < database/schema.sql

# Verify tables
sqlite3 leafsense.db ".tables"
```

### Database locked
```
[DB] Error: database is locked
```

**Solution:**
```bash
# Check for running processes
fuser leafsense.db

# Kill blocking process
kill -9 <PID>

# Or wait and retry
# SQLite default timeout is 5 seconds
```

### Database corrupted
```
[DB] Error: database disk image is malformed
```

**Solution:**
```bash
# Backup corrupted database
cp leafsense.db leafsense.db.bak

# Try to recover
sqlite3 leafsense.db ".recover" | sqlite3 leafsense_new.db

# Or reinitialize
rm leafsense.db
sqlite3 leafsense.db < database/schema.sql
```

---

## Machine Learning Problems

### Model does not load
```
[ML] Warning: Model file not found: ./leafsense_model.onnx
```

**Solution:**
```bash
# Verify path
ls -la /opt/leafsense/leafsense_model.onnx

# Copy to working directory
cp /opt/leafsense/models/leafsense_model.onnx /opt/leafsense/

# Verify size (should be ~6MB)
ls -lh /opt/leafsense/leafsense_model.onnx
```

### ONNX Runtime crash
```
Segmentation fault (in onnxruntime)
```

**Solution:**
```bash
# Verify ONNX Runtime version
strings /usr/lib/libonnxruntime.so | grep "1.16"

# Verify ARM64 compatibility
file /usr/lib/libonnxruntime.so
# Should show: ARM aarch64

# Verify ONNX model
python3 -c "import onnx; m=onnx.load('leafsense_model.onnx'); print(m.opset_import)"
```

### Inference too slow
```
Inference time: > 1000ms
```

**Solution:**
```cpp
// Optimize session options
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // Use 4 cores
session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_ENABLE_ALL
);

// Reduce input resolution
cv::resize(image, resized, cv::Size(224, 224));

// Use batch size = 1
std::vector<int64_t> input_shape = {1, 3, 224, 224};
```

---

## Touchscreen Problems

### Black display (Waveshare 3.5")
```
LeafSense runs but touchscreen is black
```

**Solution:**
```bash
# Verify available framebuffers
cat /proc/fb
# Expected: 0 BCM2708 FB, 1 fb_ili9486

# Use fb1 for Waveshare, not fb0
./LeafSense -platform linuxfb:fb=/dev/fb1

# Verify overlay is loaded
dmesg | grep -E "ili9486|fb_ili9486|waveshare"
```

### Boot hang with touchscreen (white screen)
```
Pi hangs on boot with white screen, only red LED on
```

**Solution:**
```bash
# 1. Verify overlay is installed
ls /boot/overlays/waveshare35c.dtbo

# 2. Verify config.txt has the correct lines:
# dtoverlay=waveshare35c,speed=48000000,fps=20
# hdmi_force_hotplug=1
# hdmi_group=2
# hdmi_mode=87
# hdmi_cvt=480 320 60 6 0 0 0

# 3. If overlay doesn't exist, copy it:
scp deploy/waveshare35c.dtbo root@10.42.0.196:/boot/overlays/
```

### Touch not responding
```
Display works but touch input is ignored
```

**Solution:**
```bash
# Step 1: Verify touch device exists
ls -la /dev/input/event*
cat /proc/bus/input/devices | grep -A 4 ADS7846

# Step 2: Check udev rules are configured
cat /etc/udev/rules.d/99-touchscreen.rules

# Step 3: Create udev rule if it doesn't exist
echo 'SUBSYSTEM=="input", KERNEL=="event*", ATTRS{name}=="*ADS7846*", MODE="0666"' > /etc/udev/rules.d/99-touchscreen.rules

# Step 4: CRITICAL - Enable Qt evdev touch plugin
# The touch won't work without this environment variable!
export QT_QPA_GENERIC_PLUGINS=evdevtouch
export QT_QPA_MOUSEDRIVER=linuxinput
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1

# Step 5: Verify the plugin is loaded
# Check startup logs for: "loaded library libqevdevtouchplugin.so"
tail -f /var/log/leafsense.log | grep -i "evdev\|touch\|plugin"

# Step 6: Verify the application is using the correct plugin path
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins

# Then start LeafSense
cd /opt/leafsense
./LeafSense -platform linuxfb:fb=/dev/fb1
```

**Note:** The Qt evdev touch plugin (`libqevdevtouchplugin.so`) MUST be enabled via the `QT_QPA_GENERIC_PLUGINS=evdevtouch` environment variable. Without this, touch input will not work even if the ADS7846 device is properly detected. This is the most common cause of touchscreen unresponsiveness.

---

## Useful Logs

### View system logs
```bash
dmesg                           # Kernel messages
cat /var/log/messages           # System messages
tail -f /var/log/leafsense.log  # Application logs
```

### View logs with timestamp
```bash
dmesg -T | tail -20
```

### Filter logs
```bash
grep -i error /var/log/leafsense.log
grep -i "ML\|model\|onnx" /var/log/leafsense.log
dmesg | grep -i "led\|gpio"
```

---

## Contact

For problems not resolved by this guide, check:
1. Issues in the GitHub repository
2. Detailed system logs
3. Official documentation for Qt5, ONNX Runtime, Buildroot

---

*Document last updated: January 9, 2026*
