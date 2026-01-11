/**
 * @file utils.h
 * @brief GPIO Utility Definitions for Raspberry Pi Kernel Module
 * 
 * Defines the GPIO register structure and utility functions for
 * low-level GPIO manipulation on BCM2837 (Raspberry Pi 3/4).
 */

#ifndef UTILS_H
#define UTILS_H

#include <linux/types.h>

/* ============================================================================
 * Hardware Definitions
 * ============================================================================ */

#define BCM2708_PERI_BASE  0x3f000000                  ///< BCM2837 peripheral base address
#define GPIO_BASE          (BCM2708_PERI_BASE + 0x200000)  ///< GPIO controller base address

/* ============================================================================
 * GPIO Register Structure
 * ============================================================================ */

/**
 * @brief GPIO Register Map Structure
 * 
 * Maps the BCM2837 GPIO registers for direct hardware access.
 * GPFSEL: Function select (3 bits per pin, 10 pins per register)
 * GPSET:  Set output HIGH (write 1 to set, 0 has no effect)
 * GPCLR:  Set output LOW (write 1 to clear, 0 has no effect)
 */
struct GpioRegisters
{
	uint32_t GPFSEL[6];   ///< Function Select registers (GPIO 0-53)
	uint32_t Reserved1;   ///< Reserved
	uint32_t GPSET[2];    ///< Pin Output Set registers
	uint32_t Reserved2;   ///< Reserved
	uint32_t GPCLR[2];    ///< Pin Output Clear registers
};

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Configure GPIO pin function
 * @param s_pGpioRegisters Pointer to mapped GPIO registers
 * @param GPIO GPIO pin number (0-53)
 * @param functionCode Function code (000=Input, 001=Output, etc.)
 */
void SetGPIOFunction(struct GpioRegisters *s_pGpioRegisters, int GPIO, int functionCode);

/**
 * @brief Set GPIO output value
 * @param s_pGpioRegisters Pointer to mapped GPIO registers
 * @param GPIO GPIO pin number (0-53)
 * @param outputValue true=HIGH, false=LOW
 */
void SetGPIOOutputValue(struct GpioRegisters *s_pGpioRegisters, int GPIO, bool outputValue);

#endif /* UTILS_H */
