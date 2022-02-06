/*
 * gpio_setting.c
 *
 * Created: 2021/12/30 10:59:30
 *  Author: kiyot
 */
#include <stdint.h>
#include <stddef.h>

#include <samd51_gpio.h>

#include "lib4legs.h"

void initialize_gpio(void)
{
	//J USB
	samd51_gpio_configure(USB_DM_PIN,   SAMD51_GPIO_INOUT, SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_FUNC_H);
	samd51_gpio_configure(USB_DP_PIN,   SAMD51_GPIO_INOUT, SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_FUNC_H);
	samd51_gpio_configure(USB_OE_PIN,   SAMD51_GPIO_OUT,   SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(USB_SEL_PIN,  SAMD51_GPIO_OUT,   SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(USB_CC1_PIN,  SAMD51_GPIO_IN,    SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(USB_CC2_PIN,  SAMD51_GPIO_IN,    SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(VBUS_DET_PIN, SAMD51_GPIO_IN,    SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_DEFAULT);

	samd51_gpio_output(USB_OE_PIN, 0);
	samd51_gpio_output(USB_SEL_PIN, 1);


	//J LED
	samd51_gpio_configure(LED0_PIN, SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(LED1_PIN,	SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(LED2_PIN,	SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_DEFAULT);

	samd51_gpio_output(LED0_PIN, 1);
	samd51_gpio_output(LED1_PIN, 1);
	samd51_gpio_output(LED2_PIN, 1);

	//J IMU
	samd51_gpio_configure(SINT1_PIN, SAMD51_GPIO_IN,    SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_FUNC_A);
	samd51_gpio_configure(SINT2_PIN, SAMD51_GPIO_IN,    SAMD51_GPIO_PULLUP_DOWN, SAMD51_GPIO_MUX_FUNC_A);
	samd51_gpio_configure(SCL_S_PIN, SAMD51_GPIO_INOUT, SAMD51_GPIO_NO_PULL,     SAMD51_GPIO_MUX_FUNC_C);
	samd51_gpio_configure(SDA_S_PIN, SAMD51_GPIO_INOUT, SAMD51_GPIO_NO_PULL,     SAMD51_GPIO_MUX_FUNC_C);
	
	//J Legs
	samd51_gpio_configure(INT0_PIN, SAMD51_GPIO_INOUT, SAMD51_GPIO_PULLUP_DOWN,   SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(INT1_PIN, SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN,   SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(INT2_PIN, SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN,   SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(INT3_PIN, SAMD51_GPIO_OUT, SAMD51_GPIO_PULLUP_DOWN,   SAMD51_GPIO_MUX_DEFAULT);
	samd51_gpio_configure(SCL_PIN,  SAMD51_GPIO_INOUT, SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_FUNC_C);
	samd51_gpio_configure(SDA_PIN,  SAMD51_GPIO_INOUT, SAMD51_GPIO_NO_PULL, SAMD51_GPIO_MUX_FUNC_C);

	return;
}