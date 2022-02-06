/*
 * hid.c
 *
 * Created: 2021/12/30 11:01:55
 *  Author: kiyot
 */ 
#include <stdint.h>
#include <stddef.h>

#include <samd51_gpio.h>

#include "lib4legs.h"

void lib4legs_led_set(uint8_t bitmap)
{
	if (bitmap & LIB4LEGS_LED0) {
		samd51_gpio_output(LED0_PIN, 0);
	}

	if (bitmap & LIB4LEGS_LED1) {
		samd51_gpio_output(LED1_PIN, 0);
	}

	if (bitmap & LIB4LEGS_LED2) {
		samd51_gpio_output(LED2_PIN, 0);
	}
	
	return;
}

void lib4legs_led_clear(uint8_t bitmap)
{
	if (bitmap & LIB4LEGS_LED0) {
		samd51_gpio_output(LED0_PIN, 1);
	}

	if (bitmap & LIB4LEGS_LED1) {
		samd51_gpio_output(LED1_PIN, 1);
	}

	if (bitmap & LIB4LEGS_LED2) {
		samd51_gpio_output(LED2_PIN, 1);
	}
	
	return;	
}

void lib4legs_led_toggle(uint8_t bitmap)
{
	if (bitmap & LIB4LEGS_LED0) {
		samd51_gpio_output_toggle(LED0_PIN);
	}

	if (bitmap & LIB4LEGS_LED1) {
		samd51_gpio_output_toggle(LED1_PIN);
	}

	if (bitmap & LIB4LEGS_LED2) {
		samd51_gpio_output_toggle(LED2_PIN);
	}
	
	return;
}
