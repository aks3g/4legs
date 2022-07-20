/*
 * power.c
 *
 * Created: 2022/04/06 4:20:42
 *  Author: kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <samd51_error.h>
#include <samd51_gpio.h>
#include <samd51_adc.h>

#include "lib4legs.h"
typedef struct 
{
	uint32_t vbat_mV;
	uint32_t vcc_mV;
	
	int dcdc_enabled;
	int _7V4_enabled;
	int _6V0_enabled;
	
} Lib4legsPower;

static Lib4legsPower sPower = {0};

static uint32_t _adc_val_to_mV(int16_t val);

int lib4legs_power_ctrl_dcdc(int enable)
{
	if (enable) {
		sPower.dcdc_enabled = 1;
		samd51_gpio_output(VCC_EN_PIN, 1);
	}
	else {
		sPower.dcdc_enabled = 0;
		samd51_gpio_output(VCC_EN_PIN, 0);
	}
	
	return 0;
}

int lib4legs_power_ctrl_6v0(int enable)
{
	if (sPower.dcdc_enabled) {
		return LIB4LEGS_ERROR_POWER_INVALID;
	}

	if (enable) {
		sPower._6V0_enabled = 1;
		samd51_gpio_output(EN_6V0_PIN, 1);
	}
	else {
		sPower._6V0_enabled = 0;
		samd51_gpio_output(EN_6V0_PIN, 0);
	}
	
	return 0;	
}

int lib4legs_power_ctrl_7v4(int enable)
{
	if (sPower.dcdc_enabled) {
		return LIB4LEGS_ERROR_POWER_INVALID;
	}

	if (enable) {
		sPower._7V4_enabled = 1;
		samd51_gpio_output(EN_7V4_PIN, 1);
	}
	else {
		sPower._7V4_enabled = 0;
		samd51_gpio_output(EN_7V4_PIN, 0);
	}

	return 0;
}

uint32_t lib4legs_power_get_vbat(void)
{
	return _adc_val_to_mV(sPower.vbat_mV) * 3;
}

uint32_t lib4legs_power_get_vcc(void)
{
	return _adc_val_to_mV(sPower.vcc_mV) * 3;
}

int lib4legs_power_check_power_good(void)
{
	return samd51_gpio_input(DCDC_PG_PIN);
}



// For internal
int lib4legs_power_on(int enable)
{
	if (enable) {
		samd51_gpio_output(POWER_ON_PIN, 1);
	}
	else {
		samd51_gpio_output(POWER_ON_PIN, 0);
	}
	
	return 0;
}

int lib4legs_power_check_power_button(void)
{
	return samd51_gpio_input(SW_SENS_PIN);
}

static int sAdcIdx = 0;
static void _adc_done(int result, int16_t val) {
	// VBAT
	if (sAdcIdx) {
		samd51_adc_convert(0, SAMD51_ADC_SINGLE_END, SAMD51_ADC_POS_AIN2, SAMD51_ADC_NEG_GND, _adc_done);
		sPower.vcc_mV = val;
	}
	// VCC
	else {
		samd51_adc_convert(0, SAMD51_ADC_SINGLE_END, SAMD51_ADC_POS_AIN3, SAMD51_ADC_NEG_GND, _adc_done);
		sPower.vbat_mV = val;
	}

	sAdcIdx = 1 - sAdcIdx;
}

void lib4legs_power_update(void)
{
	samd51_adc_convert(0, SAMD51_ADC_SINGLE_END, SAMD51_ADC_POS_AIN2, SAMD51_ADC_NEG_GND, _adc_done);
}


static uint32_t _adc_val_to_mV(int16_t val)
{
	//VRef = 3.3V
	float vref = 3300.0f;
	float mV = vref * (val/4096.0f);

	return (uint32_t)mV;
}
