/*
 * serial.c
 *
 * Created: 2021/12/31 7:33:54
 *  Author: kiyot
 */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include "internal/usb_cdc.h"
#include "lib4legs.h"

int lib4legs_linked(void)
{
	return usbCdc_isLinkedUp();
}

int lib4legs_putc(const char c)
{
	return usbCdc_putc(c);
}

int lib4legs_puts(const char *s)
{
	return usbCdc_puts(s);
}

int lib4legs_tx(const uint8_t *buf, const size_t len)
{
	return usbCdc_tx(buf, len);
}

int lib4legs_getc(char *c, int timeout)
{
	if (timeout == 0) {
		return lib4legs_rx((uint8_t *)c, 1, 0);
	}
	else {
		return lib4legs_rx((uint8_t *)c, 1, 1);
	}
}

int lib4legs_rx(uint8_t *buf, const size_t len, const int blocking)
{
	if (blocking) {
		return usbCdc_rx(buf, len);
	}
	else if (blocking == 0 && len == 1){
		return usbCdc_try_rx(buf);
	}

	return -1;
}

static char sCommonLineBuf[512];
int lib4legs_printf(const char *format, ...)
{
	va_list ap;

	va_start( ap, format );
	int len = vsnprintf(sCommonLineBuf, sizeof(sCommonLineBuf), format, ap );
	va_end( ap );

	usbCdc_puts(sCommonLineBuf);
	
	return len;
}


