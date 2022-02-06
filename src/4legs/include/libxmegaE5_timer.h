/*
 * timer_driver.h
 *
 * Created: 2013/09/07 14:39:25
 *  Author: sazae7
 */ 
#ifndef TIMER_DRIVER_H_
#define TIMER_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
typedef void (*timer_callback)(void);
typedef enum
{
	LIB_XMEGA_E5_TCC4,
	LIB_XMEGA_E5_TCC5,
	LIB_XMEGA_E5_TCD5,
	LIB_XMEGA_E5_TC_MAX
} LIB_XMEGA_E5_TC;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t  libxmegaE5_initialize_tc_as_timerMilSec(LIB_XMEGA_E5_TC tc, uint32_t periodInMiliSec, uint32_t peripheralClock);
uint8_t  libxmegaE5_initialize_tc_as_timerMicroSec(LIB_XMEGA_E5_TC tc, uint32_t periodInMicroSec, uint32_t peripheralClock);
uint8_t  libxmegaE5_tc_timer_registerCallback(LIB_XMEGA_E5_TC tc, timer_callback cb);
uint32_t libxmegaE5_tc_timer_getCurrentCount(LIB_XMEGA_E5_TC tc);
void     libxmegaE5_tc_enable(LIB_XMEGA_E5_TC tc, uint8_t set);

#ifdef __cplusplus
};
#endif

#endif /* TIMER_DRIVER_H_ */