/*
 * timer_driver.cpp
 *
 * Created: 2013/09/07 14:39:43
 *  Author: sazae7
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "libxmegaE5_error.h"
#include "libxmegaE5_utils.h"
#include "libxmegaE5_timer.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static timer_callback sCallback[LIB_XMEGA_E5_TC_MAX] = {NULL};
static volatile uint32_t sFreerunCounter[LIB_XMEGA_E5_TC_MAX] = {0};

/*---------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t  libxmegaE5_initialize_tc_as_timerMilSec(LIB_XMEGA_E5_TC tc, uint32_t periodInMiliSec, uint32_t peripheralClock)
{
	TC5_t *pTc = _get_tcxx(tc);
	if (pTc == NULL) {
		return LIB_XMEGA_E5_ERROR_NULL;
	}

	uint16_t divMap[8] = {0, 1, 2, 4, 8, 64, 256, 1024};
	volatile uint32_t dividedClock = peripheralClock;

	/*J 指定された周期を出すにあたって、相応しいClock Divを見つける */
	volatile int i;
	for (i=1 ; i<8 ; ++i) {
		dividedClock = peripheralClock / divMap[i];

		//J 16bit のカウントで収まる範囲で動かす
		if ( ((dividedClock * periodInMiliSec) / (uint32_t)(1000)) < 0x10000) {
			break;
		} else {
			dividedClock = 0;
		}
	}

	/*J 条件を満たせなかったので死ぬ */
	if (dividedClock == 0) {
		return LIB_XMEGA_E5_ERROR_TIMER_CANNOT_CONFIGURATE;
	}

	/**/
	pTc->CTRLA = i;	
	pTc->CTRLB = TC_WGMODE_NORMAL_gc;
	pTc->CTRLC = 0;
	pTc->CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;

	pTc->INTCTRLA = TC_ERRINTLVL_OFF_gc | TC_OVFINTLVL_MED_gc;
	pTc->INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc; //J 定義が怪しい
	pTc->INTFLAGS = 1; //J オーバーフロー割込みを有効化
	
	pTc->PER = (dividedClock * periodInMiliSec) / (1000L);
	
	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
uint8_t  libxmegaE5_initialize_tc_as_timerMicroSec(LIB_XMEGA_E5_TC tc, uint32_t periodInMicroSec, uint32_t peripheralClock)
{
	TC5_t *pTc = _get_tcxx(tc);
	if (pTc == NULL) {
		return LIB_XMEGA_E5_ERROR_NULL;
	}

	uint16_t divMap[8] = {0, 1, 2, 4, 8, 64, 256, 1024};
	volatile uint32_t dividedClock = peripheralClock;

	/*J 指定された周期を出すにあたって、相応しいClock Divを見つける */
	volatile int i;
	for (i=1 ; i<8 ; ++i) {
		dividedClock = peripheralClock / divMap[i];

		//J 16bit のカウントで収まる範囲で動かす
		if ( ((dividedClock * periodInMicroSec) / (uint32_t)(1000)) < 0x10000) {
			break;
		} else {
			dividedClock = 0;
		}
	}

	/*J 条件を満たせなかったので死ぬ */
	if (dividedClock == 0) {
		return LIB_XMEGA_E5_ERROR_TIMER_CANNOT_CONFIGURATE;
	}

	/**/
	pTc->CTRLA = i;
	pTc->CTRLB = TC_WGMODE_NORMAL_gc;
	pTc->CTRLC = 0;
	pTc->CTRLD = TC_EVACT_OFF_gc | TC_EVSEL_OFF_gc;

	pTc->INTCTRLA = TC_ERRINTLVL_OFF_gc | TC_OVFINTLVL_HI_gc;
	pTc->INTCTRLB = TC_CCDINTLVL_OFF_gc | TC_CCCINTLVL_OFF_gc | TC_CCBINTLVL_OFF_gc | TC_CCAINTLVL_OFF_gc;
	pTc->INTFLAGS = 1; //J オーバーフロー割込みを有効化

	pTc->PER = (dividedClock * periodInMicroSec) / (1000L*1000L);

	return LIB_XMEGA_E5_ERROR_OK;
}


/*---------------------------------------------------------------------------*/
uint8_t  libxmegaE5_tc_timer_registerCallback(LIB_XMEGA_E5_TC tc, timer_callback cb)
{
	TC5_t *pTc = _get_tcxx(tc);
	if (pTc == NULL) {
		return LIB_XMEGA_E5_ERROR_NULL;
	}

	sCallback[(int)tc] = cb;
	return LIB_XMEGA_E5_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
void libxmegaE5_tc_enable(LIB_XMEGA_E5_TC tc, uint8_t set)
{
	TC5_t *pTc = _get_tcxx(tc);
	if (pTc == NULL) {
		return;
	}

	//J 有効化
	if (set) {
		pTc->CTRLGCLR |= (1 << 5);
	}
	//J 無効化
	else {
		pTc->CTRLGSET |= (1 << 5);
		pTc->CNT = 0; // Countをゼロに戻す
	}
	
	return;
}


/*---------------------------------------------------------------------------*/
uint32_t libxmegaE5_tc_timer_getCurrentCount(LIB_XMEGA_E5_TC tc)
{
	TC5_t *pTc = _get_tcxx(tc);
	if (pTc == NULL) {
		return 0;
	}

	return sFreerunCounter[tc];
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ISR(TCC5_OVF_vect)
{
	/* 13.12.10 INTFLAGS ? Interrupt Flags register */
	TCC5.INTFLAGS = RTC_OVFIF_bm; // To Clear

	sFreerunCounter[LIB_XMEGA_E5_TCC5]++;

	if (sCallback[LIB_XMEGA_E5_TCC5]!=NULL)	{
		sCallback[LIB_XMEGA_E5_TCC5]();
	}

	return;
}

/*---------------------------------------------------------------------------*/
ISR(TCD5_OVF_vect)
{
	/* 13.12.10 INTFLAGS ? Interrupt Flags register */
	TCD5.INTFLAGS = RTC_OVFIF_bm; // To Clear

	sFreerunCounter[LIB_XMEGA_E5_TCD5]++;

	if (sCallback[LIB_XMEGA_E5_TCD5]!=NULL)	{
		sCallback[LIB_XMEGA_E5_TCD5]();
	}

	return;
}

/*---------------------------------------------------------------------------*/
ISR(TCC4_OVF_vect)
{
	/* 13.12.10 INTFLAGS ? Interrupt Flags register */
	TCC4.INTFLAGS = RTC_OVFIF_bm; // To Clear

	sFreerunCounter[LIB_XMEGA_E5_TCC4]++;

	if (sCallback[LIB_XMEGA_E5_TCC4]!=NULL)	{
		sCallback[LIB_XMEGA_E5_TCC4]();
	}

	return;
}