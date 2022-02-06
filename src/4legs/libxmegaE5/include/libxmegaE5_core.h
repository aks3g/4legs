/*
 * core_driver.h
 *
 * Created: 2013/09/12 19:46:15
 *  Author: sazae7
 */ 


#ifndef CORE_DRIVER_H_
#define CORE_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
typedef enum {
	CORE_SRC_RC2MHZ		= 0x0,
	CORE_SRC_RC8MHZ		= 0x5,
	CORE_SRC_RC32MHZ	= 0x1,
	CORE_SRC_PLL		= 0x4,
	CORE_SRC_XTAL		= 0x3,
	CORE_SRC_RC32KHZ	= 0x2
} CORE_CLK_SRC;

/*---------------------------------------------------------------------------*/
typedef enum {
	CORE_PSA_1		= 0x00,
	CORE_PSA_2		= 0x01,
	CORE_PSA_4		= 0x03,
	CORE_PSA_8		= 0x05,
	CORE_PSA_16		= 0x07,
	CORE_PSA_32		= 0x09,
	CORE_PSA_64		= 0x0B,
	CORE_PSA_128	= 0x0D,
	CORE_PSA_256	= 0x0F,
	CORE_PSA_512	= 0x11,
	CORE_PSA_6		= 0x13,
	CORE_PSA_10		= 0x15,
	CORE_PSA_12		= 0x17,
	CORE_PSA_24		= 0x19,
	CORE_PSA_48		= 0x1B
} CORE_PRESCALER_A;

/*---------------------------------------------------------------------------*/
typedef enum {
	CORE_PSBC_1_1	= 0x0,
	CORE_PSBC_1_2	= 0x1,
	CORE_PSBC_4_1	= 0x2,
	CORE_PSBC_2_2	= 0x3
} CORE_PRESCALER_B_C;

/*---------------------------------------------------------------------------*/
typedef struct {
	CORE_PRESCALER_A	prescalerA;
	CORE_PRESCALER_B_C	perscalerBC;
} CORE_PRESCALER_OPTION;

/*---------------------------------------------------------------------------*/
typedef struct {
	CORE_CLK_SRC pllSrc;
} CORE_PLL_OPTION;


/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint32_t libxmegaE5_clock_initialize(CORE_CLK_SRC src, CORE_PRESCALER_OPTION *prescalerOpt, CORE_PLL_OPTION *pllOpt);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
typedef enum {
	CORE_SLEEP_IDLE		= 0x0,
	CORE_SLEEP_PDOWN	= 0x2,
	CORE_SLEEP_PSAVE	= 0x3,
	CORE_SLEEP_STDBY	= 0x6,
	CORE_SLEEP_ESTDBY	= 0x7
} CORE_SLEEP_MODE;

#define POWER_REDUCTION_XCL				((uint32_t)0x80 << 24)
#define POWER_REDUCTION_RTC				((uint32_t)0x04 << 24)
#define POWER_REDUCTION_EVSYS			((uint32_t)0x02 << 24)
#define POWER_REDUCTION_EDMA			((uint32_t)0x01 << 24)

#define POWER_REDUCTION_PORTA_DAC		((uint32_t)0x04 << 16)
#define POWER_REDUCTION_PORTA_ADC		((uint32_t)0x02 << 16)
#define POWER_REDUCTION_PORTA_AC		((uint32_t)0x01 << 16)

#define POWER_REDUCTION_PORTC_TWI		((uint32_t)0x40 << 8)
#define POWER_REDUCTION_PORTC_USART0	((uint32_t)0x10 << 8)
#define POWER_REDUCTION_PORTC_SPI		((uint32_t)0x08 << 8)
#define POWER_REDUCTION_PORTC_HIRES		((uint32_t)0x04 << 8)
#define POWER_REDUCTION_PORTC_TC5		((uint32_t)0x02 << 8)
#define POWER_REDUCTION_PORTC_TC4		((uint32_t)0x01 << 8)

#define POWER_REDUCTION_PORTD_TWI		((uint32_t)0x40 << 0)
#define POWER_REDUCTION_PORTD_USART0	((uint32_t)0x10 << 0)
#define POWER_REDUCTION_PORTD_SPI		((uint32_t)0x08 << 0)
#define POWER_REDUCTION_PORTD_HIRES		((uint32_t)0x04 << 0)
#define POWER_REDUCTION_PORTD_TC5		((uint32_t)0x02 << 0)
#define POWER_REDUCTION_PORTD_TC4		((uint32_t)0x01 << 0)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint32_t libxmegaE5_power_reduction(uint32_t stop);
uint32_t libxmegaE5_sleep_enable(CORE_SLEEP_MODE mode);

#ifdef __cplusplus
};
#endif

#endif /* CORE_DRIVER_H_ */