/*
 * gpio_driver.cpp
 *
 * Created: 2013/09/08 23:10:51
 *  Author: sazae7
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "libxmegaE5_error.h"
#include "libxmegaE5_utils.h"
#include "libxmegaE5_gpio.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static PORT_t *checkParamsAndGetPort(GPIO_PORT port);
static uint8_t checkParamsAndGetPinMask(GPIO_PIN pin);
static register8_t *checkParamsAndGetPinControlReg(GPIO_PORT port, GPIO_PIN pin);


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_gpio_initialize(GPIO_PORT port, GPIO_PIN pin, GPIO_DIR dir, GPIO_PULL_MODE mode)
{
	/*J PortとPinが正しいか確認して、使える状態にする */
	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PORT_NOTFOUND;
	}

	uint8_t pinMask = checkParamsAndGetPinMask(pin);
	if (pinMask == 0) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}

	/*J ポートの方向をチェック */
	if (dir != GPIO_OUT && dir != GPIO_IN) {
		return LIB_XMEGA_E5_ERROR_GPIO_INVALID_DIR;
	}

	/*J ここからは、基本的に安心して作業して良い*/
	if (dir == GPIO_OUT) {
		pPort->DIRSET = pinMask; 
 	} else if(dir == GPIO_IN) {
		pPort->DIRCLR = pinMask;		 
	}

	/*J ピンコントロールを引き抜く */
	register8_t *pinControlReg = checkParamsAndGetPinControlReg(port, pin);
	if (pinControlReg == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}


	*pinControlReg = ((*pinControlReg) & ~PORT_OPC_gm) | ((mode << PORT_OPC_gp) & PORT_OPC_gm);

	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_gpio_output(GPIO_PORT port, GPIO_PIN pin, uint8_t out)
{
	/*J PortとPinが正しいか確認して、使える状態にする */
	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PORT_NOTFOUND;
	}

	uint8_t pinMask = checkParamsAndGetPinMask(pin);
	if (pinMask == 0) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}
	
	if (out) {
		pPort->OUTSET = pinMask;
	} else {
		pPort->OUTCLR = pinMask;		
	}

	return LIB_XMEGA_E5_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_gpio_output_toggle(GPIO_PORT port, GPIO_PIN pin)
{
	/*J PortとPinが正しいか確認して、使える状態にする */
	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PORT_NOTFOUND;
	}

	uint8_t pinMask = checkParamsAndGetPinMask(pin);
	if (pinMask == 0) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}
	
	pPort->OUTTGL = pinMask;

	return LIB_XMEGA_E5_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_gpio_input(GPIO_PORT port, GPIO_PIN pin, uint8_t *in)
{
	/*J PortとPinが正しいか確認して、使える状態にする */
	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PORT_NOTFOUND;
	}

	uint8_t pinMask = checkParamsAndGetPinMask(pin);
	if (pinMask == 0) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}

	if (in != NULL ) {
		*in = (pPort->IN & pinMask);
	} else {
		return LIB_XMEGA_E5_ERROR_NULL;
	}	

	return LIB_XMEGA_E5_ERROR_OK;
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static gpio_interrupt_callback sCallback[GPIO_PORT_MAX][GPIO_PIN_MAX]
=
{
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_gpio_configeInterrupt(GPIO_PORT port, GPIO_PIN pin, INTERRUPT_MODE mode, gpio_interrupt_callback callback)
{
	if (!(INT_BOTHE_EDGE <= mode && mode <=INT_LEVEL)) {
		return LIB_XMEGA_E5_ERROR_GPIO_INVALID_INT_MODE;
	}

	if (callback == NULL) {
		return LIB_XMEGA_E5_ERROR_NULL;
	}

	register8_t *pinCtrlReg = checkParamsAndGetPinControlReg(port, pin);
	if (pinCtrlReg == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}

	uint8_t ret = libxmegaE5_gpio_initialize(port, pin, GPIO_IN, GPIO_TOTEM);
	if (ret != LIB_XMEGA_E5_ERROR_OK) {
		return ret;
	}

	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return LIB_XMEGA_E5_ERROR_GPIO_PORT_NOTFOUND;
	}

	uint8_t pinMask = checkParamsAndGetPinMask(pin);
	if (pinMask == 0) {
		return LIB_XMEGA_E5_ERROR_GPIO_PIN_NOTFOUND;
	}

	/* 12.13.14 PINnCTRL ? Pin n Control register */
	*pinCtrlReg = (*pinCtrlReg & ~(PORT_ISC_gm)) | mode;
	sCallback[port][pin] = callback;

	/* 12.13.10 INTCTRL ? Interrupt Control register */
	pPort->INTCTRL = (pPort->INTCTRL & ~(PORT_INTLVL_gm)) | PORT_INTLVL_HI_gc;

	/* 12.13.11 INTMASK ? Interrupt Mask register */
	pPort->INTMASK = pinMask;

	/* 11.8.3 CTRL ? Control register */
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;

	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------*/
static uint8_t checkParamsAndGetPinMask(GPIO_PIN pin)
{
	/*J Pin番号を確認 */
	if (!(0<=pin && pin<GPIO_PIN_MAX)) {
		return 0;
	}

	return (1 << pin);	
}

/*---------------------------------------------------------------------------*/
static register8_t *checkParamsAndGetPinControlReg(GPIO_PORT port, GPIO_PIN pin)
{
	PORT_t *pPort = checkParamsAndGetPort(port);
	if (pPort == NULL) {
		return NULL;
	}

	switch (pin) {
	case GPIO_PIN0:
		return &(pPort->PIN0CTRL);
		break;
	case GPIO_PIN1:
		return &(pPort->PIN1CTRL);
		break;
	case GPIO_PIN2:
		return &(pPort->PIN2CTRL);
		break;
	case GPIO_PIN3:
		return &(pPort->PIN3CTRL);
		break;
	case GPIO_PIN4:
		return &(pPort->PIN4CTRL);
		break;
	case GPIO_PIN5:
		return &(pPort->PIN5CTRL);
		break;
	case GPIO_PIN6:
		return &(pPort->PIN6CTRL);
		break;
	case GPIO_PIN7:
		return &(pPort->PIN7CTRL);
		break;
	default:
		break;
	}

	return NULL;
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ISR(PORTA_INT_vect)
{
	uint8_t flg = PORTA.INTFLAGS;
	PORTA.INTFLAGS = 0xff;

	uint8_t i;	
	for (i=0 ; i<8 ; ++i) {
		if ((BIT(i) & flg) && sCallback[GPIO_PORTA][i] != NULL) {
			sCallback[GPIO_PORTA][i]();
		}
	}

	return;
}


/*---------------------------------------------------------------------------*/
ISR(PORTC_INT_vect)
{
	uint8_t flg = PORTC.INTFLAGS;
	PORTC.INTFLAGS = 0xff;

	uint8_t i;
	for (i=0 ; i<8 ; ++i) {
		if ((BIT(i) & flg) && sCallback[GPIO_PORTC][i] != NULL) {
			sCallback[GPIO_PORTC][i]();
		}
	}

	return;
}


/*---------------------------------------------------------------------------*/
ISR(PORTD_INT_vect)
{
	uint8_t flg = PORTD.INTFLAGS;
	PORTD.INTFLAGS = 0xff;

	uint8_t i;
	for (i=0 ; i<8 ; ++i) {
		if ((BIT(i) & flg) && sCallback[GPIO_PORTD][i] != NULL) {
			sCallback[GPIO_PORTD][i]();
		}
	}

	return;
}