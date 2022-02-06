/*
 * libxmegaE5_servo.h
 *
 * Created: 2022/01/12 19:55:17
 *  Author: kiyot
 */ 
#include "libxmegaE5_timer.h"
#include "libxmegaE5_gpio.h"

#ifndef LIBXMEAGE5_SERVO_H_
#define LIBXMEAGE5_SERVO_H_

typedef struct
{
	TC5_t *pTc;
	PORT_t *port;
	uint8_t pin;
	int invert;
	uint32_t pclk;
	
	uint16_t next_period;
} LibxmeaE5Servo; 

uint8_t libxmegaE5_servo_initialize(LibxmeaE5Servo *servo, LIB_XMEGA_E5_TC tc, GPIO_PORT port, GPIO_PIN pin, int invert, uint32_t peripheralClock);
uint8_t libxmegaE5_servo_set_pulse_width(LibxmeaE5Servo *servo, uint16_t width_us);
uint8_t libxmegaE5_servo_start(LibxmeaE5Servo *servo);
uint8_t libxmegaE5_servo_end(LibxmeaE5Servo *servo);

#endif /* LIBXMEAGE5_SERVO_H_ */