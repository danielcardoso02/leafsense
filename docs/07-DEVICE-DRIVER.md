# LeafSense LED Alert Driver - Device Driver Documentation

## Overview

The LeafSense LED Alert Driver is a Linux kernel module that provides a character device interface for controlling an LED indicator used for system alerts. The driver implements a simple write-only interface at `/dev/leddev` where user-space applications can send commands ('1' or '0') to turn the LED on or off.

**Driver Type:** Character Device  
**Device Node:** `/dev/leddev`  
**Major Number:** Dynamic (assigned by kernel)  
**Minor Number:** 0  
**Module Name:** `ledmodule`  
**Source Files:** `drivers/kernel_module/ledmodule.c`, `utils.c`, `utils.h`

---

## Table of Contents

1. [Architecture](#architecture)
2. [Hardware Interface](#hardware-interface)
3. [Module Operations](#module-operations)
4. [API Reference](#api-reference)
5. [Compilation & Installation](#compilation--installation)
6. [Usage Examples](#usage-examples)
7. [Integration with LeafSense](#integration-with-leafsense)
8. [Troubleshooting](#troubleshooting)
9. [Development](#development)

---

## Architecture

### Driver Components

```
ledmodule/
├── ledmodule.c       # Main driver implementation
├── utils.c           # Utility functions
├── utils.h           # Utility function declarations
└── Makefile          # Kernel build configuration
```

### Module Structure

```
+-------------------+
| User Space        |
|  LeafSense App    |
+-------------------+
        |
        | open("/dev/leddev", O_WRONLY)
        | write(fd, "1", 1)
        | close(fd)
        v
+-------------------+
| Kernel Space      |
|  ledmodule.ko     |
|  +-------------+  |
|  | File Ops    |  |
|  | - open      |  |
|  | - write     |  |
|  | - release   |  |
|  +-------------+  |
+-------------------+
        |
        | GPIO operations
        v
+-------------------+
| Hardware          |
|  LED on GPIO pin  |
+-------------------+
```

### Key Concepts

1. **Character Device**: Provides byte-stream I/O interface
2. **File Operations**: Standard UNIX file operations (open, write, close)
3. **GPIO Control**: Direct hardware control via GPIO subsystem
4. **Synchronization**: Mutex protection for concurrent access

---

## Hardware Interface

### GPIO Configuration

The LED is connected to a GPIO pin on the Raspberry Pi. The specific pin is configured in the driver source code.

**Default Configuration:**
- **GPIO Pin**: Configurable (typically GPIO 17, 18, or 27)
- **Direction**: Output
- **Initial State**: Low (LED off)
- **Active Level**: High (LED on when GPIO=1)

### Circuit Diagram

```
Raspberry Pi GPIO
    |
    +--- GPIO_PIN (e.g., GPIO17) ---[ 220Ω ]---[LED]----> GND
```

**Components:**
- Raspberry Pi GPIO pin (3.3V logic)
- 220Ω current-limiting resistor
- LED (forward voltage ~2V, current ~15mA)

### Pin Configuration

To change the GPIO pin, modify the driver source:

```c
// In ledmodule.c
#define LED_GPIO_PIN 17  // Change to your desired pin
```

---

## Module Operations

### Module Initialization (`led_init`)

**Function:** `static int __init led_init(void)`

**Actions:**
1. Allocates major/minor device numbers
2. Initializes character device structure
3. Creates device class
4. Creates device node `/dev/leddev`
5. Initializes GPIO pin for LED control
6. Sets initial LED state to OFF

**Return:**
- `0` on success
- Negative error code on failure

**Example Output:**
```
[LED Module] Registered with major number 240
[LED Module] Device created: /dev/leddev
[LED Module] LED initialized (GPIO: 17)
```

### Module Cleanup (`led_exit`)

**Function:** `static void __exit led_exit(void)`

**Actions:**
1. Turns LED off
2. Frees GPIO resources
3. Destroys device node
4. Destroys device class
5. Unregisters character device
6. Frees device numbers

**Example Output:**
```
[LED Module] Device removed
[LED Module] GPIO cleaned up
[LED Module] Unregistered successfully
```

### File Operations

#### Open (`led_open`)

**Prototype:** `static int led_open(struct inode *inodep, struct file *filep)`

**Description:** Called when a process opens `/dev/leddev`

**Actions:**
- Acquires mutex lock to prevent concurrent access
- Logs open event

**Return:** 0 (always succeeds)

#### Write (`led_write`)

**Prototype:** `static ssize_t led_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)`

**Description:** Receives LED control commands from user space

**Input Format:**
- '1' or '0' as single character
- Only first byte is processed

**Actions:**
1. Copies data from user space
2. Parses command ('1' = ON, '0' = OFF)
3. Sets GPIO pin accordingly
4. Returns number of bytes written

**Return:**
- Number of bytes written on success
- `-EFAULT` if copy_from_user fails

**Example:**
```c
char cmd = '1';
write(fd, &cmd, 1);  // Turn LED ON
```

#### Release (`led_release`)

**Prototype:** `static int led_release(struct inode *inodep, struct file *filep)`

**Description:** Called when file descriptor is closed

**Actions:**
- Releases mutex lock
- Logs close event

**Return:** 0 (always succeeds)

---

## API Reference

### User-Space API

#### Opening the Device

```c
#include <fcntl.h>
#include <unistd.h>

int fd = open("/dev/leddev", O_WRONLY);
if (fd < 0) {
    perror("Failed to open /dev/leddev");
    return -1;
}
```

#### Writing Commands

```c
// Turn LED ON
char cmd_on = '1';
ssize_t ret = write(fd, &cmd_on, 1);
if (ret < 0) {
    perror("Failed to write to device");
}

// Turn LED OFF
char cmd_off = '0';
write(fd, &cmd_off, 1);
```

#### Closing the Device

```c
close(fd);
```

### Kernel-Space API (Internal)

#### GPIO Control Functions (utils.c)

```c
/**
 * @brief Initialize GPIO pin for output
 * @param pin GPIO pin number
 * @return 0 on success, negative on error
 */
int gpio_init(unsigned int pin);

/**
 * @brief Set GPIO pin state
 * @param pin GPIO pin number
 * @param value 1 for HIGH, 0 for LOW
 */
void gpio_set(unsigned int pin, int value);

/**
 * @brief Cleanup GPIO resources
 * @param pin GPIO pin number
 */
void gpio_cleanup(unsigned int pin);
```

---

## Compilation & Installation

### Prerequisites

- Linux kernel headers for target kernel version
- GCC compiler
- Make build tool

**Install on Debian/Ubuntu:**
```bash
sudo apt-get install build-essential linux-headers-$(uname -r)
```

**Install on Buildroot:**
```bash
# Kernel headers are included in Buildroot build
# No additional installation needed
```

### Building the Module

```bash
cd drivers/kernel_module
make
```

**Expected Output:**
```
make -C /lib/modules/6.12.41-v8/build M=/path/to/drivers/kernel_module modules
make[1]: Entering directory '/usr/src/linux-headers-6.12.41-v8'
  CC [M]  /path/to/drivers/kernel_module/ledmodule.o
  CC [M]  /path/to/drivers/kernel_module/utils.o
  LD [M]  /path/to/drivers/kernel_module/ledmodule.ko
make[1]: Leaving directory '/usr/src/linux-headers-6.12.41-v8'
```

**Generated Files:**
- `ledmodule.ko` - Kernel module binary
- `ledmodule.mod.c` - Module metadata
- `*.o` - Object files

### Loading the Module

```bash
# Load module
sudo insmod ledmodule.ko

# Verify module is loaded
lsmod | grep led
# Output: ledmodule  16384  0

# Check kernel log
dmesg | tail -5
# Output: [LED Module] Registered with major number 240
#         [LED Module] Device created: /dev/leddev
```

### Unloading the Module

```bash
# Unload module
sudo rmmod ledmodule

# Verify removal
lsmod | grep led
# (no output)

# Check kernel log
dmesg | tail -3
# Output: [LED Module] Device removed
#         [LED Module] Unregistered successfully
```

### Installing Permanently

**Option 1: Manual Load on Boot**

Add to `/etc/rc.local` (before `exit 0`):
```bash
insmod /path/to/ledmodule.ko
```

**Option 2: Module Auto-load**

Copy module to kernel modules directory:
```bash
sudo cp ledmodule.ko /lib/modules/$(uname -r)/extra/
sudo depmod -a
```

Create module configuration:
```bash
echo "ledmodule" | sudo tee -a /etc/modules
```

**Option 3: Udev Rule for Device Permissions**

Create `/etc/udev/rules.d/99-leddev.rules`:
```
KERNEL=="leddev", MODE="0666", GROUP="gpio"
```

Reload udev rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

## Usage Examples

### Command Line

#### Using echo

```bash
# Turn LED ON
echo 1 > /dev/leddev

# Turn LED OFF
echo 0 > /dev/leddev
```

#### Using printf

```bash
# Turn LED ON
printf "1" > /dev/leddev

# Turn LED OFF
printf "0" > /dev/leddev
```

### Shell Script

```bash
#!/bin/bash
# led_blink.sh - Blink LED 5 times

for i in {1..5}; do
    echo "Blink $i"
    echo 1 > /dev/leddev
    sleep 0.5
    echo 0 > /dev/leddev
    sleep 0.5
done

echo "Done"
```

### C Program

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    char command;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <0|1>\n", argv[0]);
        return 1;
    }

    command = argv[1][0];
    if (command != '0' && command != '1') {
        fprintf(stderr, "Error: Command must be '0' or '1'\n");
        return 1;
    }

    fd = open("/dev/leddev", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open /dev/leddev");
        return 1;
    }

    if (write(fd, &command, 1) < 0) {
        perror("Failed to write to device");
        close(fd);
        return 1;
    }

    printf("LED turned %s\n", (command == '1') ? "ON" : "OFF");
    close(fd);
    return 0;
}
```

Compile and run:
```bash
gcc -o led_control led_control.c
./led_control 1  # Turn ON
./led_control 0  # Turn OFF
```

### Python Program

```python
#!/usr/bin/env python3
import time

def led_control(state):
    """Control LED state
    Args:
        state: True for ON, False for OFF
    """
    with open('/dev/leddev', 'w') as f:
        f.write('1' if state else '0')

def led_blink(times=5, interval=0.5):
    """Blink LED
    Args:
        times: Number of blinks
        interval: Seconds between state changes
    """
    for _ in range(times):
        led_control(True)
        time.sleep(interval)
        led_control(False)
        time.sleep(interval)

if __name__ == '__main__':
    print("Blinking LED...")
    led_blink(5, 0.5)
    print("Done")
```

---

## Integration with LeafSense

### Master Controller Integration

The LED driver is integrated with the LeafSense Master controller to provide visual alerts when sensor readings are out of range.

**File:** `src/middleware/Master.cpp`

**Function:** `void Master::updateAlertLED()`

```cpp
void Master::updateAlertLED() 
{
    // Get ideal ranges
    float tempRange[2], phRange[2], tdsRange[2];
    idealConditions->getTemp(tempRange);
    idealConditions->getPH(phRange);
    idealConditions->getTDS(tdsRange);
    
    // Get current sensor values
    float t = tempSensor->readSensor();
    float p = phSensor->readSensor();
    float e = tdsSensor->readSensor();
    
    // Check if any parameter is out of range
    bool alertActive = (t < tempRange[0] || t > tempRange[1] ||
                        p < phRange[0] || p > phRange[1] ||
                        e < tdsRange[0] || e > tdsRange[1]);
    
    // Control LED via kernel module
    static int ledFd = -1;
    
    // Open LED device on first call
    if (ledFd == -1) {
        ledFd = open("/dev/leddev", O_WRONLY);
        if (ledFd < 0) {
            // LED module not loaded, skip LED control
            return;
        }
    }
    
    // Write '1' or '0' to LED device
    const char* cmd = alertActive ? "1" : "0";
    ssize_t written = write(ledFd, cmd, 1);
    
    if (written != 1) {
        std::cerr << "[Master] Failed to write to LED device" << std::endl;
    }
}
```

### Integration Points

1. **Initialization**: LED device opened on first sensor read cycle
2. **Update Frequency**: Checked every sensor read (2-second interval)
3. **Alert Logic**: LED ON when any parameter out of ideal range
4. **Error Handling**: Gracefully skips if module not loaded

### Startup Integration

**File:** `/opt/leafsense/start_leafsense.sh`

```bash
#!/bin/sh

# Load LED kernel module
insmod /lib/modules/$(uname -r)/extra/ledmodule.ko 2>/dev/null

# Set permissions
chmod 666 /dev/leddev 2>/dev/null

# Start LeafSense
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb1
export QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS="/dev/input/event0:rotate=90"
cd /opt/leafsense
./LeafSense -platform linuxfb:fb=/dev/fb1
```

---

## Troubleshooting

### Module Won't Load

**Error:** `insmod: ERROR: could not insert module ledmodule.ko: Invalid module format`

**Solution:** Module compiled for wrong kernel version
```bash
# Rebuild for current kernel
cd drivers/kernel_module
make clean
make
```

### Device Node Not Created

**Error:** `/dev/leddev` doesn't exist after `insmod`

**Check kernel log:**
```bash
dmesg | grep -i led
```

**Possible causes:**
1. Module initialization failed
2. Insufficient permissions
3. Device class creation failed

**Solution:**
```bash
# Check if module loaded
lsmod | grep led

# If loaded but no device, manually create
sudo mknod /dev/leddev c $(cat /proc/devices | grep leddev | awk '{print $1}') 0
sudo chmod 666 /dev/leddev
```

### Permission Denied

**Error:** `write: Permission denied`

**Solution:**
```bash
# Change device permissions
sudo chmod 666 /dev/leddev

# Or run as root
sudo echo 1 > /dev/leddev
```

### LED Doesn't Turn On

**Possible causes:**
1. Wrong GPIO pin configured
2. LED connected incorrectly
3. Current-limiting resistor too high

**Debug steps:**
```bash
# Check GPIO state directly
cat /sys/class/gpio/gpio17/value  # Replace 17 with your pin

# Monitor kernel log
dmesg -w &
echo 1 > /dev/leddev
```

### Module in Use (Can't Unload)

**Error:** `rmmod: ERROR: Module ledmodule is in use`

**Check:** LeafSense or another process has `/dev/leddev` open

**Solution:**
```bash
# Find processes using device
lsof | grep leddev

# Kill process
killall LeafSense

# Then unload
sudo rmmod ledmodule
```

---

## Development

### Modifying the Driver

#### Changing GPIO Pin

Edit `ledmodule.c`:
```c
#define LED_GPIO_PIN 27  // Change from 17 to 27
```

Rebuild and reload:
```bash
make clean && make
sudo rmmod ledmodule
sudo insmod ledmodule.ko
```

#### Adding Debug Messages

```c
printk(KERN_DEBUG "[LED Module] Debug message: %d\n", value);
```

View debug messages:
```bash
sudo dmesg -n 8  # Enable debug level
dmesg | grep "LED Module"
```

#### Adding Ioctl Support

Define ioctl commands in header:
```c
#define LED_IOC_MAGIC  'k'
#define LED_IOC_ON     _IO(LED_IOC_MAGIC, 1)
#define LED_IOC_OFF    _IO(LED_IOC_MAGIC, 2)
#define LED_IOC_TOGGLE _IO(LED_IOC_MAGIC, 3)
```

Implement ioctl handler:
```c
static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch(cmd) {
        case LED_IOC_ON:
            gpio_set(LED_GPIO_PIN, 1);
            break;
        case LED_IOC_OFF:
            gpio_set(LED_GPIO_PIN, 0);
            break;
        case LED_IOC_TOGGLE:
            // Toggle implementation
            break;
        default:
            return -EINVAL;
    }
    return 0;
}
```

Add to file_operations:
```c
static struct file_operations fops = {
    .open = led_open,
    .write = led_write,
    .unlocked_ioctl = led_ioctl,
    .release = led_release,
};
```

### Testing

#### Unit Tests

```bash
#!/bin/bash
# test_led_driver.sh

echo "Testing LED Driver..."

# Test 1: Device exists
if [ ! -c /dev/leddev ]; then
    echo "FAIL: Device /dev/leddev does not exist"
    exit 1
fi
echo "PASS: Device exists"

# Test 2: Can write to device
if echo 1 > /dev/leddev 2>/dev/null; then
    echo "PASS: Can turn LED ON"
else
    echo "FAIL: Cannot write to device"
    exit 1
fi

sleep 1

if echo 0 > /dev/leddev 2>/dev/null; then
    echo "PASS: Can turn LED OFF"
else
    echo "FAIL: Cannot write to device"
    exit 1
fi

# Test 3: Invalid commands
if echo 2 > /dev/leddev 2>/dev/null; then
    echo "WARN: Accepted invalid command"
else
    echo "PASS: Rejects invalid commands"
fi

echo "All tests passed"
```

#### Stress Test

```bash
#!/bin/bash
# stress_test_led.sh - Rapid on/off cycles

COUNT=1000
echo "Stress testing LED driver ($COUNT cycles)..."

for i in $(seq 1 $COUNT); do
    echo 1 > /dev/leddev
    echo 0 > /dev/leddev
    if [ $((i % 100)) -eq 0 ]; then
        echo "Completed $i cycles"
    fi
done

echo "Stress test completed successfully"
```

---

## API Summary

### Device File Operations

| Operation | Description | Parameters | Return |
|-----------|-------------|------------|--------|
| `open()` | Open device | `/dev/leddev`, `O_WRONLY` | File descriptor or -1 |
| `write()` | Set LED state | '1' (ON) or '0' (OFF) | Bytes written or -1 |
| `close()` | Close device | File descriptor | 0 or -1 |

### Kernel Module Operations

| Function | Description | Called When |
|----------|-------------|-------------|
| `led_init()` | Initialize module | `insmod` |
| `led_exit()` | Cleanup module | `rmmod` |
| `led_open()` | Open device | User opens `/dev/leddev` |
| `led_write()` | Process write | User writes to device |
| `led_release()` | Close device | User closes file descriptor |

---

## References

- [Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/)
- [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
- [Raspberry Pi GPIO Documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio)
- [Character Device Drivers](https://www.kernel.org/doc/html/latest/driver-api/basics.html)

---

**Author:** LeafSense Development Team  
**Last Updated:** January 10, 2026  
**Version:** 1.0  
**License:** Academic Project - University of Minho
