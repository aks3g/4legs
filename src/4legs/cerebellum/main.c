/*
 * cerebellum.c
 *
 * Created: 2021/12/30 6:41:32
 * Author : kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <samd51_nvmctrl.h>
#include <samd51_error.h>
#include <samd51_gpio.h>
#include <samd51_sercom.h>
#include <samd51_uart.h>
#include <samd51_clock.h>
#include <samd51_timer.h>
#include <samd51_i2c.h>
#include <samd51_sdhc.h>
#include <samd51_adc.h>
#include <samd51_ac.h>
#include <samd51_dac.h>
#include <samd51_usb_device.h>
#include <samd51_interrupt.h>

#include "lib4legs.h"
#include "cmd.h"
#include "console.h"

#include "spine_controller.h"
#include "posture.h"


static void _putchar(const char c)
{
	lib4legs_putc(c);
}

static void _setup_spines(void)
{
	int ret = 0;
	for (int i=0 ; i<4 ; ++i) {
		ret = cerebellum_spine_version_check_and_update(i, 0);
		lib4legs_printf("Update spine %d firmware. 0x%08x\n", i, (uint32_t)ret);
	}

	// Setup Zero position.	
	lib4legs_spine_if_set_limit(SPINE0, SERVO_ID_BODY,       750, 2250);
	lib4legs_spine_if_set_limit(SPINE0, SERVO_ID_2ND_JOINT, 1100, 2250);
	lib4legs_spine_if_set_limit(SPINE0, SERVO_ID_1ST_JOINT,  750, 2250);
	lib4legs_spine_if_set_origin(SPINE0, 1450, 1100, 1150);

	lib4legs_spine_if_set_limit(SPINE1, SERVO_ID_BODY,       750, 2250);
	lib4legs_spine_if_set_limit(SPINE1, SERVO_ID_2ND_JOINT,  775, 1800);
	lib4legs_spine_if_set_limit(SPINE1, SERVO_ID_1ST_JOINT,  750, 2250);
	lib4legs_spine_if_set_origin(SPINE1, 1450, 1750, 1750);

	lib4legs_spine_if_set_limit(SPINE2, SERVO_ID_BODY,       750, 2250);
	lib4legs_spine_if_set_limit(SPINE2, SERVO_ID_2ND_JOINT, 1100, 2250);
	lib4legs_spine_if_set_limit(SPINE2, SERVO_ID_1ST_JOINT,  750, 2250);
	lib4legs_spine_if_set_origin(SPINE2, 1450, 1100, 1150);

	lib4legs_spine_if_set_limit(SPINE3, SERVO_ID_BODY,       750, 2250);
	lib4legs_spine_if_set_limit(SPINE3, SERVO_ID_2ND_JOINT,  775, 1800);
	lib4legs_spine_if_set_limit(SPINE3, SERVO_ID_1ST_JOINT,  750, 2250);
	lib4legs_spine_if_set_origin(SPINE3, 1450, 1800, 1750);

	// Set Home position
	cerebellum_posture_initialize();
	cerebellum_posture_home();

	// Start Servo pulse
	lib4legs_enable_base_pulse(1);

	return;
}


int main(void)
{
	lib4legs_initialzie();

	lib4legs_led_set(LIB4LEGS_LED0);
	lib4legs_led_set(LIB4LEGS_LED2);

	_setup_spines();

	while (lib4legs_linked() == 0);
	consoleInitialize(_putchar);
	initialize_commands();
	
	while(1) {
		char c;
		lib4legs_rx((uint8_t *)&c, 1, 1);
		consoleUpdate(c);
	}
	
	return 0;
}
