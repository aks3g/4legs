/*
 * pwm_driver.h
 *
 * Created: 2013/09/07 5:26:49
 *  Author: sazae7
 */ 


#ifndef PWM_DRIVER_H_
#define PWM_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define TCCxIO_REMAP		(1)
#define TCCxIO_NOT_REMAP	(0)

/*---------------------------------------------------------------------------*/
typedef enum {
	TCCxA	= (0),
	TCCxB	= (1),
	TCCxC	= (2),
	TCCxD	= (3)
} PWM_CH;	


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint16_t libxmegaE5_tcc4_pwm_initialize(uint32_t carrerFreqInHz, uint32_t peripheralClock, PWM_CH ch, uint8_t remap);

void libxmegaE5_tcc4_pwm_start(void);
void libxmegaE5_tcc4_pwm_stop(void);
void libxmegaE5_tcc4_pwm_enable(PWM_CH ch, uint8_t enable);

void libxmegaE5_tcc4_pwm_setDuty(uint16_t duty, PWM_CH idx);
uint16_t libxmegaE5_tcc4_pwm_getDuty(PWM_CH idx);

#ifdef __cplusplus
};
#endif

#endif /* PWM_DRIVER_H_ */