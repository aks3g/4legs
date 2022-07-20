/*
 * samd51_bootloader.c
 *
 * Created: 2019/09/05 23:34:12
 * Author : kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sam.h>

#include <samd51_nvmctrl.h>
#include <samd51_error.h>
#include <samd51_gpio.h>
#include <samd51_sercom.h>
#include <samd51_uart.h>
#include <samd51_clock.h>
#include <samd51_timer.h>
#include <samd51_i2c.h>
#include <samd51_sdhc.h>
#include <samd51_adc.h>
#include <samd51_ac.h>
#include <samd51_dac.h>
#include <samd51_usb_device.h>
#include <samd51_interrupt.h>

#include "include/xmodem.h"

#include "lib4legs.h"
#include "console.h"
#include "cmd.h"

#define REGISTORY_ADD				(0x0001C000)
#define FLASH_HEAD					(0x0001E000)
#define MAIN_FLASH_SIZE				(0x000E2000)

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static uint32_t sFlashWp = FLASH_HEAD;

static uint32_t sFwSizeInPackets = 0;
static uint8_t sFlashBuf[512];
static uint32_t sFlashBufWp = 0;


static uint8_t _fw_receive_cb(uint8_t status, const XmodemContext *ctx)
{
	memcpy(&(sFlashBuf[sFlashBufWp]), ctx->buf.fd.data, 128);
	sFlashBufWp += 128;

	sFwSizeInPackets--;
	if (sFwSizeInPackets == 0 || sFlashBufWp >= 512) {
		size_t block_size = samd51_nvmctrl_get_block_size();
		if ((sFlashWp & (block_size-1)) == 0) {
			(void)samd51_nvmctrl_erase_block(sFlashWp, 1);
		}

		samd51_nvmctrl_write_page(sFlashWp, sFlashBuf, 1);
		memset(sFlashBuf, 0xff, sizeof(sFlashBuf));
		sFlashWp += 512;
		sFlashBufWp = 0;
	}

	return 0;
}

void lib4legs_bootloader_update_main_fw(uint32_t fwsize)
{
	XmodemContext xmodemctx;

	sFwSizeInPackets = (fwsize + 127) / 128;
	sFlashBufWp = 0;
	sFlashWp = FLASH_HEAD;

	for (int i=0 ; i<10 ; ++i) {
		lib4legs_printf("Wait %2d\r", 10-i);
		lib4legs_timer_delay_ms(1000);
	}

//	xmodem_receive(&xmodemctx, _fw_receive_cb);

	return;
}


static void _putchar(const char c)
{
	lib4legs_putc(c);
}

static void boot_loader_console(void)
{
	while (lib4legs_linked() == 0);
	consoleInitialize(_putchar);
	initialize_commands();
	
	while(1) {
		char c;
		lib4legs_rx((uint8_t *)&c, 1, 1);
		consoleUpdate(c);
	}
}


int main(void)
{
	lib4legs_initialzie();
	
	//J Flashの内容がダメそうで、UpdateのReqestも来てないときにはConsoleを出す
	if ((lib4legs_check_valid_flash() == 0 && lib4legs_check_update_req() == 0) || lib4legs_check_force_bootloader()) {
		boot_loader_console();
	}
	//J Update Requestが出ていればUpdateSequenceに入る
	else if (lib4legs_check_update_req() != 0) {
		while (lib4legs_linked() == 0);
		
		uint32_t fw_size_in_bytes = lib4legs_get_fwsize_for_update();
		lib4legs_bootloader_update_main_fw(fw_size_in_bytes);
	}
	//J そのままFlash ProgramにBootする
	else {
	}

	lib4legs_restart();
	
	return 0;
}