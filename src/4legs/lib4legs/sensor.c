/*
 * sensor.c
 *
 * Created: 2022/01/03 6:42:22
 *  Author: kiyot
 */ 
#include <stdint.h>

#include <samd51_interrupt.h>

#include "internal/lsm6ds3h.h"

static const uint16_t SENSOR_EIC_PIN_ALL = (1 << 5) | (1 << 6);

int initialize_sensor(void)
{
	lsm6ds3h_probe();

	//J EICレジスタのピン状態を確認する
	volatile uint16_t pinstate = 0;
	do {
		pinstate = samd51_external_interrupt_get_pinstate();
	} while ((pinstate & SENSOR_EIC_PIN_ALL) != SENSOR_EIC_PIN_ALL);

	lsm6ds3h_grab_oneshot();

	return 0;
}