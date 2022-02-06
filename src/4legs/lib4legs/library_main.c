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

LIB4LEGS_BACKUP_RAM gBackupRam;

#define SINT1_SOURCE_EXINT_CH					(SAMD51_EIC_CHANNEL5)
#define SINT2_SOURCE_EXINT_CH					(SAMD51_EIC_CHANNEL6)
#define VBUS_DET_SOURCE_EXINT_CH				(SAMD51_EIC_CHANNEL7)

typedef struct {
	int is_vbus_connected;
	int checking;
	uint16_t timer;
} UsbCableChecker;
static UsbCableChecker sUsbCableCheck = {0,0,0};
static const uint16_t cUsbCableCheckTimer = 500;
static volatile uint32_t sTimer0Tick = 0;

static Lib4legsUpdatePulseCb sBasePulseCb = NULL;
static int sBasePulseEnabled = 0;

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void _timer10ms_cb(void)
{
	if (sUsbCableCheck.checking) {
		sUsbCableCheck.timer--;
		if (sUsbCableCheck.timer == 0) {
			sUsbCableCheck.timer = cUsbCableCheckTimer;
			// lib4legs_usb_sel_swap();
		}
	}
	
	if (sBasePulseEnabled) {
		samd51_gpio_output_toggle(INT0_PIN);
		samd51_gpio_output_toggle(INT1_PIN);
		samd51_gpio_output_toggle(INT2_PIN);
		samd51_gpio_output_toggle(INT3_PIN);
		
		if (samd51_gpio_input(INT0_PIN) !=0 && sBasePulseCb != NULL) {
			sBasePulseCb();
		}
	}
	
	sTimer0Tick++;
	
	return;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void _usb_cable_check_and_connect(void) {
	if (lib4legs_cc1_stat()) {
		lib4legs_led_set(LIB4LEGS_LED1);
	}
	else if(lib4legs_cc2_stat()) {
		lib4legs_led_set(LIB4LEGS_LED2);
	}
	else {
		sUsbCableCheck.checking = 1;
		sUsbCableCheck.timer = cUsbCableCheckTimer;
	}
	
}

/*--------------------------------------------------------------------------*/
static void _vbus_det_cb(void) {
	if (lib4legs_vbus_det_stat()) {
		sUsbCableCheck.is_vbus_connected = 1;
		_usb_cable_check_and_connect();
	}
	else {
		sUsbCableCheck.is_vbus_connected = 0;
	}
}

/*--------------------------------------------------------------------------*/
static volatile int sLinkUp = 0;
static void _check_link_up(void)
{
	sLinkUp = 1;
	sUsbCableCheck.checking = 0;
	return;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void _int1_cb(void)
{
	lsm6ds3h_on_int1();
}

/*--------------------------------------------------------------------------*/
static void _int2_cb(void)
{
	lsm6ds3h_on_int2();
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
//	samd51_i2c_initialize(SAMD51_SERCOM0, 400000);
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

	//J 最初からVBUSが立っていれば通信処理を走らせる
	if (lib4legs_vbus_det_stat()) {
		sUsbCableCheck.is_vbus_connected = 1;
		_usb_cable_check_and_connect();
	}

	//J Setting Ex Interrupt
	samd51_external_interrupt_initialize(0);
	samd51_external_interrupt_setup(SINT1_SOURCE_EXINT_CH, SAMD51_EIC_SENSE_RISE, 0, _int1_cb);
	samd51_external_interrupt_setup(SINT2_SOURCE_EXINT_CH, SAMD51_EIC_SENSE_RISE, 0, _int2_cb);
//	samd51_external_interrupt_setup(VBUS_DET_SOURCE_EXINT_CH, SAMD51_EIC_SENSE_BOTH, 0, _vbus_det_cb);

	//J IMUのセットアップ
//	initialize_sensor();

	inialize_spine_if(SAMD51_SERCOM2);

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