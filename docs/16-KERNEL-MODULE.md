# LeafSense - Kernel Module (LED Driver)

## Overview

The kernel module `led.ko` provides direct control of an LED connected to GPIO through a character device (`/dev/led0`). The module uses direct access to the GPIO registers of the BCM2711 (Raspberry Pi 4).

## Specifications

| Property | Value |
|----------|-------|
| Module name | `led` |
| Device file | `/dev/led0` |
| Major number | 237 (dynamic) |
| GPIO Pin | 20 (configurable) |
| Kernel version | 6.12.41-v8 |

## Architecture do Module

```
┌─────────────────────────────────────────────────────────┐
│                    User Space                           │
│   ┌─────────────────────────────────────────────────┐   │
│   │  echo 1 > /dev/led0    cat /dev/led0           │   │
│   └───────────────┬─────────────────┬───────────────┘   │
└───────────────────┼─────────────────┼───────────────────┘
                    │ write()         │ read()
┌───────────────────┼─────────────────┼───────────────────┐
│                   │  Kernel Space   │                   │
│   ┌───────────────▼─────────────────▼───────────────┐   │
│   │              led.ko module                      │   │
│   │  ┌─────────────────────────────────────────┐   │   │
│   │  │  file_operations:                       │   │   │
│   │  │  - led_device_open()                    │   │   │
│   │  │  - led_device_close()                   │   │   │
│   │  │  - led_device_read()                    │   │   │
│   │  │  - led_device_write()                   │   │   │
│   │  └────────────────┬────────────────────────┘   │   │
│   │                   │                             │   │
│   │  ┌────────────────▼────────────────────────┐   │   │
│   │  │  GPIO Register Access (ioremap)         │   │   │
│   │  │  - GPFSEL (Function Select)             │   │   │
│   │  │  - GPSET (Set Output High)              │   │   │
│   │  │  - GPCLR (Clear Output Low)             │   │   │
│   │  └────────────────┬────────────────────────┘   │   │
│   └───────────────────┼─────────────────────────────┘   │
└───────────────────────┼─────────────────────────────────┘
                        │
┌───────────────────────▼─────────────────────────────────┐
│                    Hardware                             │
│   ┌─────────────────────────────────────────────────┐   │
│   │  BCM2711 GPIO Controller                        │   │
│   │  Physical Address: 0xFE200000                   │   │
│   │  GPIO20 → LED                                   │   │
│   └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Código Fonte

### File: `drivers/kernel_module/ledmodule.c`

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define DEVICE_NAME "led"
#define CLASS_NAME  "led_class"

// BCM2711 GPIO base address (Raspberry Pi 4)
#define GPIO_BASE   0xFE200000
#define GPIO_SIZE   0xB4

// GPIO registers offsets
#define GPFSEL0     0x00    // GPIO Function Select 0
#define GPFSEL1     0x04    // GPIO Function Select 1
#define GPFSEL2     0x08    // GPIO Function Select 2
#define GPSET0      0x1C    // GPIO Pin Output Set 0
#define GPCLR0      0x28    // GPIO Pin Output Clear 0
#define GPLEV0      0x34    // GPIO Pin Level 0

// LED GPIO pin
#define LED_PIN     20

static int major_number;
static struct class *led_class = NULL;
static struct device *led_device = NULL;
static void __iomem *gpio_base;
static int led_state = 0;

// Set GPIO pin as output
static void SetGPIOOutput(int pin) {
    int reg_index = pin / 10;
    int bit_offset = (pin % 10) * 3;
    u32 reg_value;
    
    reg_value = ioread32(gpio_base + GPFSEL0 + (reg_index * 4));
    reg_value &= ~(7 << bit_offset);  // Clear bits
    reg_value |= (1 << bit_offset);   // Set as output
    iowrite32(reg_value, gpio_base + GPFSEL0 + (reg_index * 4));
}

// Set GPIO output value
static void SetGPIOOutputValue(int pin, int value) {
    u32 reg_value = 1 << pin;
    
    if (value) {
        iowrite32(reg_value, gpio_base + GPSET0);
    } else {
        iowrite32(reg_value, gpio_base + GPCLR0);
    }
    
    printk(KERN_INFO "SetGPIOOutputValue: register value is 0x%x\n", reg_value);
}

// Device operations
static int led_device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "led_device_open: called\n");
    return 0;
}

static int led_device_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "led_device_close: called\n");
    return 0;
}

static ssize_t led_device_read(struct file *file, char __user *buf,
                                size_t count, loff_t *ppos) {
    char state_str[4];
    int len;
    
    len = snprintf(state_str, sizeof(state_str), "%d\n", led_state);
    
    if (*ppos >= len)
        return 0;
    
    if (copy_to_user(buf, state_str, len))
        return -EFAULT;
    
    *ppos += len;
    return len;
}

static ssize_t led_device_write(struct file *file, const char __user *buf,
                                 size_t count, loff_t *ppos) {
    char kbuf[4];
    
    printk(KERN_INFO "led_device_write: called (%zu)\n", count);
    
    if (count > sizeof(kbuf) - 1)
        count = sizeof(kbuf) - 1;
    
    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    
    kbuf[count] = '\0';
    
    if (kbuf[0] == '1') {
        led_state = 1;
        SetGPIOOutputValue(LED_PIN, 1);
    } else {
        led_state = 0;
        SetGPIOOutputValue(LED_PIN, 0);
    }
    
    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_device_open,
    .release = led_device_close,
    .read = led_device_read,
    .write = led_device_write,
};

// Module init
static int __init led_init(void) {
    printk(KERN_INFO "LED module: initializing\n");
    
    // Map GPIO registers
    gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!gpio_base) {
        printk(KERN_ERR "LED module: failed to map GPIO\n");
        return -ENOMEM;
    }
    
    // Register character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "LED module: failed to register device\n");
        iounmap(gpio_base);
        return major_number;
    }
    
    // Create device class
    led_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        iounmap(gpio_base);
        return PTR_ERR(led_class);
    }
    
    // Create device
    led_device = device_create(led_class, NULL, MKDEV(major_number, 0),
                               NULL, DEVICE_NAME "0");
    if (IS_ERR(led_device)) {
        class_destroy(led_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        iounmap(gpio_base);
        return PTR_ERR(led_device);
    }
    
    // Set GPIO as output
    SetGPIOOutput(LED_PIN);
    SetGPIOOutputValue(LED_PIN, 0);  // Start OFF
    
    printk(KERN_INFO "LED module: registered with major %d\n", major_number);
    return 0;
}

// Module exit
static void __exit led_exit(void) {
    SetGPIOOutputValue(LED_PIN, 0);  // Turn OFF
    
    device_destroy(led_class, MKDEV(major_number, 0));
    class_destroy(led_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    iounmap(gpio_base);
    
    printk(KERN_INFO "LED module: unloaded\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LeafSense Team - Grupo 11");
MODULE_DESCRIPTION("LED GPIO driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
```

### File: `drivers/kernel_module/Makefile`

```makefile
obj-m := led.o
led-objs := ledmodule.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

# Cross-compilation for Raspberry Pi 4
arm64:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) \
		ARCH=arm64 \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		modules
```

## Compilation

### Para Desktop (Test)
```bash
cd drivers/kernel_module
make
```

### Para Raspberry Pi 4 (Cross-Compilation)
```bash
cd drivers/kernel_module

export KERNEL_SRC=~/buildroot/buildroot-2025.08/output/build/linux-custom
export CROSS_COMPILE=~/buildroot/buildroot-2025.08/output/host/bin/aarch64-linux-
export ARCH=arm64

make arm64
```

## Correção Aplicada

### Problema: `ioremap_nocache` deprecated
O kernel Linux 5.6+ removeu a função `ioremap_nocache`. A correção foi:

```diff
- gpio_base = ioremap_nocache(GPIO_BASE, GPIO_SIZE);
+ gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
```

A função `ioremap` no ARM64 já é não-cacheable por defeito.

## Usage

### Carregar Module
```bash
insmod /lib/modules/$(uname -r)/led.ko
# ou
modprobe led
```

### Verify Carregamento
```bash
lsmod | grep led
# led                    12288  0

ls -la /dev/led0
# crw------- 1 root root 237, 0 ... /dev/led0
```

### Controlar LED
```bash
# Ligar LED
echo 1 > /dev/led0

# Desligar LED
echo 0 > /dev/led0

# Ler estado
cat /dev/led0
# 1 ou 0
```

### Ver Logs do Kernel
```bash
dmesg | grep led
# [  123.456789] LED module: initializing
# [  123.456790] LED module: registered with major 237
# [  234.567890] led_device_open: called
# [  234.567891] led_device_write: called (2)
# [  234.567892] SetGPIOOutputValue: register value is 0x100000
```

### Descarregar Module
```bash
rmmod led
```

## Integration com LeafSense

O módulo é carregado automaticamente pelo script de init:

```bash
# /etc/init.d/S98leafsense
if [ -f /lib/modules/$(uname -r)/led.ko ]; then
    insmod /lib/modules/$(uname -r)/led.ko 2>/dev/null || true
fi
```

A aplicação pode controlar o LED via C++:
```cpp
void setLED(bool state) {
    std::ofstream led("/dev/led0");
    if (led.is_open()) {
        led << (state ? "1" : "0");
    }
}
```

## Conexão de Hardware

```
Raspberry Pi 4 GPIO Header:
                    ┌─────────────┐
         3.3V    1  │ ●  ●  │  2   5V
        GPIO2    3  │ ●  ●  │  4   5V
        GPIO3    5  │ ●  ●  │  6   GND
        GPIO4    7  │ ●  ●  │  8   GPIO14
          GND    9  │ ●  ●  │ 10   GPIO15
       GPIO17   11  │ ●  ●  │ 12   GPIO18
       GPIO27   13  │ ●  ●  │ 14   GND
       GPIO22   15  │ ●  ●  │ 16   GPIO23
         3.3V   17  │ ●  ●  │ 18   GPIO24
       GPIO10   19  │ ●  ●  │ 20   GND
        GPIO9   21  │ ●  ●  │ 22   GPIO25
       GPIO11   23  │ ●  ●  │ 24   GPIO8
          GND   25  │ ●  ●  │ 26   GPIO7
        GPIO0   27  │ ●  ●  │ 28   GPIO1
        GPIO5   29  │ ●  ●  │ 30   GND
        GPIO6   31  │ ●  ●  │ 32   GPIO12
       GPIO13   33  │ ●  ●  │ 34   GND
       GPIO19   35  │ ●  ●  │ 36   GPIO16
  LED→ GPIO20   37  │ ●  ●  │ 38   GPIO20  ←LED
          GND   39  │ ●  ●  │ 40   GPIO21
                    └─────────────┘

Circuito LED:
    GPIO20 ──┬── 330Ω ──┬── LED (Anodo) ── LED (Catodo) ── GND
             │          │
          (Pin 38)   Resistor
```

## Registo de GPIO BCM2711

| Registo | Offset | Function |
|---------|--------|--------|
| GPFSEL0 | 0x00 | Function Select GPIO 0-9 |
| GPFSEL1 | 0x04 | Function Select GPIO 10-19 |
| GPFSEL2 | 0x08 | Function Select GPIO 20-29 |
| GPSET0 | 0x1C | Set GPIO 0-31 High |
| GPCLR0 | 0x28 | Clear GPIO 0-31 (Low) |
| GPLEV0 | 0x34 | Read GPIO 0-31 Level |

### Function Select (3 bits por GPIO)
| Value | Function |
|-------|--------|
| 000 | Input |
| 001 | Output |
| 010-111 | Alternate functions |
