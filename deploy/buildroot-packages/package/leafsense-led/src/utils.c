/**
 * @file utils.c
 * @brief GPIO Utility Functions for Raspberry Pi Kernel Module
 * 
 * Provides low-level GPIO register manipulation functions for
 * configuring pin functions and setting output values.
 */

#include "utils.h"
#include <linux/module.h>

/* ============================================================================
 * GPIO Configuration Functions
 * ============================================================================ */

/**
 * @brief Set the function of a GPIO pin
 * 
 * Configures a GPIO pin for a specific function (input, output, alt functions).
 * Each GPFSEL register controls 10 pins, with 3 bits per pin.
 * 
 * Function codes:
 *   - 000 = Input
 *   - 001 = Output
 *   - 100 = Alt function 0
 *   - 101 = Alt function 1
 *   - etc.
 * 
 * @param s_pGpioRegisters Pointer to mapped GPIO registers
 * @param GPIO GPIO pin number (0-53)
 * @param functionCode 3-bit function code
 */
void SetGPIOFunction(struct GpioRegisters *s_pGpioRegisters, int GPIO, int functionCode) 
{
	int registerIndex = GPIO / 10;
	int bit = (GPIO % 10) * 3;

	unsigned oldValue = s_pGpioRegisters->GPFSEL[registerIndex];
	unsigned mask = 0b111 << bit;

	pr_alert("%s: register index is %d\n", __FUNCTION__, registerIndex);
	pr_alert("%s: mask is 0x%x\n", __FUNCTION__, mask);
	pr_alert("%s: update value is 0x%x\n", __FUNCTION__, ((functionCode << bit) & mask));
	
	s_pGpioRegisters->GPFSEL[registerIndex] = (oldValue & ~mask) | ((functionCode << bit) & mask);
}

/**
 * @brief Set the output value of a GPIO pin
 * 
 * Sets or clears a GPIO pin that has been configured as output.
 * Uses the GPSET and GPCLR registers for atomic operations.
 * 
 * @param s_pGpioRegisters Pointer to mapped GPIO registers
 * @param GPIO GPIO pin number (0-53)
 * @param outputValue true = HIGH, false = LOW
 */
void SetGPIOOutputValue(struct GpioRegisters *s_pGpioRegisters, int GPIO, bool outputValue) 
{
	pr_alert("%s: register value is 0x%x\n", __FUNCTION__, (1 << (GPIO % 32)));
			 
	if (outputValue)
		s_pGpioRegisters->GPSET[GPIO / 32] = (1 << (GPIO % 32));
	else
		s_pGpioRegisters->GPCLR[GPIO / 32] = (1 << (GPIO % 32));
}
