/**
 * @file ledmodule.c
 * @brief Linux Kernel Module for GPIO LED Control
 * 
 * This kernel module provides a character device interface (/dev/led0)
 * for controlling an LED connected to GPIO pin 20 on Raspberry Pi.
 * 
 * Usage:
 *   echo '1' > /dev/led0  # Turn LED ON
 *   echo '0' > /dev/led0  # Turn LED OFF
 * 
 * @author LeafSense Team
 * @version 1.0
 */

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/io.h>

#include "utils.h"

/* ============================================================================
 * Module Configuration
 * ============================================================================ */

#define DEVICE_NAME "led0"      ///< Device name in /dev/
#define CLASS_NAME  "ledClass"  ///< Device class name

MODULE_LICENSE("GPL");

/* ============================================================================
 * Device Variables
 * ============================================================================ */

static struct class* ledDevice_class = NULL;    ///< Device class pointer
static struct device* ledDevice_device = NULL;  ///< Device object pointer
static dev_t ledDevice_majorminor;              ///< Major/minor device numbers
static struct cdev c_dev;                       ///< Character device structure

static struct class *s_pDeviceClass;
static struct device *s_pDeviceObject;
struct GpioRegisters *s_pGpioRegisters;         ///< GPIO registers mapping

static const int LedGpioPin = 20;               ///< GPIO pin for LED control

/* ============================================================================
 * File Operations Implementation
 * ============================================================================ */

/**
 * @brief Write handler for LED device
 * 
 * Writes '0' to turn LED OFF, any other character turns LED ON.
 * 
 * @param pfile File pointer
 * @param pbuff User buffer containing command
 * @param len Length of data
 * @param off File offset
 * @return Number of bytes written, or negative error code
 */
ssize_t led_device_write(struct file *pfile, const char __user *pbuff, size_t len, loff_t *off) 
{
	struct GpioRegisters *pdev; 
	
	pr_alert("%s: called (%u)\n", __FUNCTION__, len);

	if (unlikely(pfile->private_data == NULL))
		return -EFAULT;

	pdev = (struct GpioRegisters *)pfile->private_data;
	
	if (pbuff[0] == '0')
		SetGPIOOutputValue(pdev, LedGpioPin, 0);
	else
		SetGPIOOutputValue(pdev, LedGpioPin, 1);
	
	return len;
}

/**
 * @brief Read handler for LED device
 * 
 * Currently returns 0 (no data to read).
 * 
 * @param pfile File pointer
 * @param p_buff User buffer
 * @param len Length requested
 * @param poffset File offset
 * @return Always returns 0
 */
ssize_t led_device_read(struct file *pfile, char __user *p_buff, size_t len, loff_t *poffset) 
{
	pr_alert("%s: called (%u)\n", __FUNCTION__, len);
	return 0;
}

/**
 * @brief Close handler for LED device
 * 
 * @param p_inode Inode pointer
 * @param pfile File pointer
 * @return Always returns 0
 */
int led_device_close(struct inode *p_inode, struct file *pfile) 
{
	pr_alert("%s: called\n", __FUNCTION__);
	pfile->private_data = NULL;
	return 0;
}

/**
 * @brief Open handler for LED device
 * 
 * Stores GPIO registers pointer in file's private data.
 * 
 * @param p_indode Inode pointer
 * @param p_file File pointer
 * @return Always returns 0
 */
int led_device_open(struct inode* p_indode, struct file *p_file) 
{
	pr_alert("%s: called\n", __FUNCTION__);
	p_file->private_data = (struct GpioRegisters *)s_pGpioRegisters;
	return 0;
}

/* ============================================================================
 * File Operations Structure
 * ============================================================================ */

static struct file_operations ledDevice_fops = {
	.owner   = THIS_MODULE,
	.write   = led_device_write,
	.read    = led_device_read,
	.release = led_device_close,
	.open    = led_device_open,
};

/* ============================================================================
 * Module Initialization / Cleanup
 * ============================================================================ */

/**
 * @brief Module initialization function
 * 
 * Creates character device, maps GPIO registers, and configures
 * the LED pin as output.
 * 
 * @return 0 on success, negative error code on failure
 */
static int __init ledModule_init(void) 
{
	int ret;
	struct device *dev_ret;

	pr_alert("%s: called\n", __FUNCTION__);

	// Allocate character device region
	if ((ret = alloc_chrdev_region(&ledDevice_majorminor, 0, 1, DEVICE_NAME)) < 0) {
		return ret;
	}

	// Create device class
	if (IS_ERR(ledDevice_class = class_create(THIS_MODULE, CLASS_NAME))) {
		unregister_chrdev_region(ledDevice_majorminor, 1);
		return PTR_ERR(ledDevice_class);
	}
	
	// Create device
	if (IS_ERR(dev_ret = device_create(ledDevice_class, NULL, ledDevice_majorminor, NULL, DEVICE_NAME))) {
		class_destroy(ledDevice_class);
		unregister_chrdev_region(ledDevice_majorminor, 1);
		return PTR_ERR(dev_ret);
	}

	// Initialize and add character device
	cdev_init(&c_dev, &ledDevice_fops);
	c_dev.owner = THIS_MODULE;
	
	if ((ret = cdev_add(&c_dev, ledDevice_majorminor, 1)) < 0) {
		printk(KERN_NOTICE "Error %d adding device", ret);
		device_destroy(ledDevice_class, ledDevice_majorminor);
		class_destroy(ledDevice_class);
		unregister_chrdev_region(ledDevice_majorminor, 1);
		return ret;
	}

	// Map GPIO registers to virtual memory
	s_pGpioRegisters = (struct GpioRegisters *)ioremap_nocache(GPIO_BASE, sizeof(struct GpioRegisters));
	
	pr_alert("map to virtual adresse: 0x%x\n", (unsigned)s_pGpioRegisters);
	
	// Configure LED pin as output
	SetGPIOFunction(s_pGpioRegisters, LedGpioPin, 0b001);

	return 0;
}

/**
 * @brief Module exit function
 * 
 * Resets GPIO pin to input, unmaps registers, and cleans up
 * all allocated resources.
 */
static void __exit ledModule_exit(void) 
{
	pr_alert("%s: called\n", __FUNCTION__);
	
	// Reset GPIO pin to input mode
	SetGPIOFunction(s_pGpioRegisters, LedGpioPin, 0);
	
	// Unmap GPIO registers
	iounmap(s_pGpioRegisters);
	
	// Cleanup device
	cdev_del(&c_dev);
	device_destroy(ledDevice_class, ledDevice_majorminor);
	class_destroy(ledDevice_class);
	unregister_chrdev_region(ledDevice_majorminor, 1);
}

module_init(ledModule_init);
module_exit(ledModule_exit);
