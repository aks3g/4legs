/*
 * uart_driver.h
 *
 * Created: 2013/09/12 0:19:58
 *  Author: sazae7
 */ 


#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define USART0_ON_PORTC_0					(0)
#define USART0_ON_PORTC_1					(1)
#define USART0_ON_PORTD_0					(2)
#define USART0_ON_PORTD_1					(3)

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t libxmegaE5_uart_initialize(uint8_t usartType, uint32_t baudrate, uint32_t peripheralClock);

uint8_t libxmegaE5_uart_tx(uint8_t *dat, uint32_t len);
uint8_t libxmegaE5_uart_txNonBuffer(uint8_t *dat, uint32_t len);

uint8_t libxmegaE5_uart_rx(uint8_t *dat, uint32_t *len);
uint8_t libxmegaE5_uart_rx_n(uint8_t *dat, uint32_t rlen, uint32_t *len);
uint32_t libxmegaE5_uart_get_rxlen(void);
uint8_t libxmegaE5_uart_rxBlocking(uint8_t *dat, uint32_t rlen);

#ifdef __cplusplus
};
#endif

#endif /* UART_DRIVER_H_ */