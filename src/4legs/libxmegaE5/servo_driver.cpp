/*
 * servo_driver.c
 *
 * Created: 2022/01/12 19:54:59
 *  Author: kiyot
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "libxmegaE5_error.h"
#include "libxmegaE5_utils.h"
#include "libxmegaE5_servo.h"
#include "libxmegaE5_timer.h"
#include "libxmegaE5_gpio.h"

/*---------------------------------------------------------------------------*/
static TC5_t *_get_tcxx(LIB_XMEGA_E5_TC tc)
{
	// TC4もTimerとして使う場合にはTC5_tで対応できる
	switch (tc)
	{
		case LIB_XMEGA_E5_TCC4:
		return (TC5_t *)&TCC4;
		case LIB_XMEGA_E5_TCC5:
		return (TC5_t *)&TCC5;
		case LIB_XMEGA_E5_TCD5:
		return (TC5_t *)&TCD5;
		default:
		return NULL;
	}
}

static PORT_t *checkParamsAndGetPort(GPIO_PORT port)
{
	PORT_t *pPort = NULL;

	/*J Port を見つける */
	if (port == GPIO_PORTA) {
		pPort = &PORTA;
		} else if (port == GPIO_PORTC) {
		pPort = &PORTC;
		} else if (port == GPIO_PORTD) {
		pPort = &PORTD;
		} else if (port == GPIO_PORTR) {
		pPort = &PORTR;
	}
	return pPort;
}


static LibxmeaE5Servo *sServo[LIB_XMEGA_E5_TC_MAX] =
{
	NULL, NULL, NULL
};


static void _tcc4_cb(void) {
	libxmegaE5_servo_end(sServo[LIB_XMEGA_E5_TCC4]);
}

static void _tcc5_cb(void) {
	libxmegaE5_servo_end(sServo[LIB_XMEGA_E5_TCC5]);
}

static void _tcd5_cb(void) {
	libxmegaE5_servo_end(sServo[LIB_XMEGA_E5_TCD5]);
}

static timer_callback _sCallbackTable[LIB_XMEGA_E5_TC_MAX] =
{
	_tcc4_cb,
	_tcc5_cb,
	_tcd5_cb
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_servo_initialize(LibxmeaE5Servo *servo, LIB_XMEGA_E5_TC tc, GPIO_PORT port, GPIO_PIN pin, int invert, uint32_t peripheralClock)
{
	if (servo == NULL) return LIB_XMEGA_E5_ERROR_NULL;

	servo->pTc = _get_tcxx(tc);
	if (servo->pTc == NULL) {
		return LIB_XMEGA_E5_ERROR_NULL;
	}

	servo->pclk = peripheralClock / 2; //TODO
	servo->port = checkParamsAndGetPort(port);
	servo->pin  = 1 <<pin;
	servo->invert = invert ? 1 : 0;

	//J Timer Clockに同期
	//J 割込みで動作停止
	servo->pTc->CTRLA = TC_CLKSEL_DIV2_gc | TC5_SYNCHEN_bm | TC5_UPSTOP_bm;
	servo->pTc->CTRLB = TC_WGMODE_NORMAL_gc;
	servo->pTc->CTRLC = 0;
	servo->pTc->CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;

	servo->pTc->INTCTRLA = TC_ERRINTLVL_OFF_gc | TC_OVFINTLVL_MED_gc;
	servo->pTc->INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc; //J 定義が怪しい
	servo->pTc->INTFLAGS = 1; //J オーバーフロー割込みを有効化

	libxmegaE5_servo_set_pulse_width(servo, 1500);
	servo->pTc->PER = (servo->pclk / 1000000L) * 1500;

	libxmegaE5_tc_timer_registerCallback(tc, _sCallbackTable[tc]);
	sServo[tc] = servo;

	return LIB_XMEGA_E5_ERROR_OK;	
}
uint8_t libxmegaE5_servo_set_pulse_width(LibxmeaE5Servo *servo, uint16_t width_us)
{
	servo->next_period = (servo->pclk / 1000000L) * width_us; 
	return LIB_XMEGA_E5_ERROR_OK;
}

uint8_t libxmegaE5_servo_start(LibxmeaE5Servo *servo)
{
	if (servo->invert) {
		servo->port->OUTCLR = servo->pin;
	}
	else {
		servo->port->OUTSET = servo->pin;
	}
	servo->pTc->PER = servo->next_period;
	servo->pTc->CTRLGCLR = TC5_STOP_bm;
	return LIB_XMEGA_E5_ERROR_OK;
}

uint8_t libxmegaE5_servo_end(LibxmeaE5Servo *servo)
{
	servo->port->OUTTGL = servo->pin;
	servo->pTc->CTRLGSET = TC_CMD_RESTART_gc | TC5_STOP_bm;

	return LIB_XMEGA_E5_ERROR_OK;
}