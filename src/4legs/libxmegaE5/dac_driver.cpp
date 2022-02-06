/*
 * dac_driver.cpp
 *
 * Created: 2014/02/23 1:50:35
 *  Author: sazae7
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "libxmegaE5_error.h"
#include "libxmegaE5_utils.h"
#include "libxmegaE5_dac.h"

#include "libxmegaE5_gpio.h"

/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_dac_initialize(DAC_INIT_OPT *opt)
{
	uint8_t vref_select = 0;
	
	if (opt != NULL) {
		vref_select = (opt->vrefSelect << DAC_REFSEL_gp) & DAC_REFSEL_gm;
	} else {
		vref_select = DAC_REFSEL_AVCC_gc;
	}
	
	/* 25.10.1 CTRLA ? Control register A */
	//J DAC‚ð—LŒø‰»
	DACA_CTRLA = DAC_ENABLE_bm;
	
	/* 25.10.2 CTRLB ? Control register B */
	//J ‚Æ‚è‚ ‚¦‚¸—¼•û“®‚­‚æ‚¤‚É‚Í‚·‚é
	DACA_CTRLB = DAC_CHSEL_DUAL_gc;

	/* 25.10.3 CTRLC ? Control register C */
	DACA_CTRLC = vref_select;

	/* 25.10.4 EVCTRL ? Event Control register */
	DACA_EVCTRL = 0;


	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_dac_enable(DAC_CH channel, uint8_t enable)
{
	uint8_t ch_bitmap = 0;
	
	if(channel == DAC_CH0) {
		ch_bitmap = DAC_CH0EN_bm;
	} else if (channel == DAC_CH1) {
		ch_bitmap = DAC_CH1EN_bm;		
	} else {
		return LIB_XMEGA_E5_ERROR_DAC_INVALID_CHANNEL;
	}
	
	if (enable) {
		DACA_CTRLA |= ch_bitmap;	
	} else {
		DACA_CTRLA &=~ch_bitmap;
	}
	
	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_dac_setValue(DAC_CH channel, uint16_t val)
{
	if(channel == DAC_CH0) {
		/* 25.10.10 CH0DATAH ? Channel 0 Data register High */
		DACA_CH0DATA = val;
	} else if (channel == DAC_CH1) {
		/* 25.10.12 CH1DATAH ? Channel 1 Data register High */
		DACA_CH1DATA = val;
	} else {
		return LIB_XMEGA_E5_ERROR_DAC_INVALID_CHANNEL;
	}

	return LIB_XMEGA_E5_ERROR_OK;	
}


/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_dac_calibrat(DAC_CH channel, uint8_t gain, uint8_t offset)
{
	
	return LIB_XMEGA_E5_ERROR_OK;
}
