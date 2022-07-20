/*
 * library_main.c
 *
 * Created: 2021/12/31 5:43:00
 *  Author: kiyot
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
#include "internal/usb_cdc.h"
#include "internal/usb_dev.h"
#include "internal/gpio_setting.h"
#include "internal/clock_setting.h"
#include "internal/sensor.h"
#include "internal/lsm6ds3h.h"
#include "internal/spine_if.h"
#include "internal/backup_ram.h"
#include "internal/power.h"

LIB4LEGS_BACKUP_RAM gBackupRam;

#define VBUS_DET_SOURCE_EXINT_CH				(SAMD51_EIC_CHANNEL7)
static volatile uint32_t sTimer0Tick = 0;

static Lib4legsUpdatePulseCb sBasePulseCb = NULL;
static int sBasePulseEnabled = 0;

const  uint16_t cPowerOnThreshold  = 10;
const  uint16_t cPowerOffThreshold = 200;
static uint16_t sPowerButtonPress  = 0;

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void _timer10ms_cb(void)
{	
	if (sBasePulseEnabled) {
		samd51_gpio_output_toggle(INT0_PIN);
		samd51_gpio_output_toggle(INT1_PIN);
		samd51_gpio_output_toggle(INT2_PIN);
		samd51_gpio_output_toggle(INT3_PIN);
		samd51_gpio_output_toggle(BP_S_PIN);
		
		if (samd51_gpio_input(INT0_PIN) !=0 && sBasePulseCb != NULL) {
			sBasePulseCb();
		}
	}

	if (lib4legs_power_check_power_button()) {
		sPowerButtonPress++;
		if (sPowerButtonPress > cPowerOffThreshold){
			lib4legs_power_on(0);
		}
		else if (sPowerButtonPress > cPowerOnThreshold) {
			lib4legs_power_on(1);
		}
	}
	else {
		sPowerButtonPress = 0;
	}

//	lib4legs_power_update();

	sTimer0Tick++;
	
	return;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static volatile int sLinkUp = 0;
static void _check_link_up(void)
{
	sLinkUp = 1;
	return;
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int lib4legs_initialzie(void)
{
	initialize_gpio();
	initialize_clock_tree();

	lib4legs_led_clear(LIB4LEGS_LED0 | LIB4LEGS_LED1 | LIB4LEGS_LED2);

	//J Setting Timer
	samd51_tc_initialize_as_timer(SAMD51_TC0, 1000000, 10000, _timer10ms_cb);

	//J Setting I2C
	samd51_i2c_initialize(SAMD51_SERCOM0, 100000);
	samd51_i2c_initialize(SAMD51_SERCOM2, 100000);

	//J Setting USB
	char reg_buf[128];
	sprintf(reg_buf, "%08X%08X%08X%08X",
		*((unsigned int *)0x008061FC),
		*((unsigned int *)0x00806010),
		*((unsigned int *)0x00806014),
		*((unsigned int *)0x00806018));	
	usbCdcRegisterLinkUpCallback(_check_link_up);
	initialize_usb(reg_buf, "DDC Hacker Serial Port");

	//J Setting Ex Interrupt
	samd51_external_interrupt_initialize(0);

	//J Setting ADC
	SAMD51_ADC_POST_PROCESS_OPT adc0_opt;
	{
		adc0_opt.average_cnt = SAMD51_ADC_AVERAGE_16_SAMPLES;
		adc0_opt.average_div_power = 4;
	}
	samd51_adc_setup(0, SAMD51_ADC_SINGLE_SHOT, SAMD51_ADC_BIT_RES_12, SAMD51_ADC_REF_EXTERNAL_REFA, &adc0_opt);
	lib4legs_power_update();

	inialize_spine_if(SAMD51_SERCOM2, SAMD51_SERCOM0);

	return 0;
}


int lib4legs_register_sensor_captured_cb(LIB4LEGS_SENSOR_CAPTUERED_CB cb) {
	lsm6ds3h_register_captured_done_cb((LSM6DS3H_CAPTURED_CB)cb);
	
	return 0;
}

uint32_t lib4legs_timer_get_tick(void)
{
	return sTimer0Tick;
}

void lib4legs_timer_delay_ms(uint32_t ms)
{
	volatile uint32_t end   = sTimer0Tick + ((ms + 9) / 10) + 1;
	while (sTimer0Tick != end);

	return;
}

void lib4legs_enable_base_pulse(int set)
{
	sBasePulseEnabled = set;
}

int lib4legs_register_base_pulse_cb(Lib4legsUpdatePulseCb func)
{
	sBasePulseCb = func;
	
	return 0;
}