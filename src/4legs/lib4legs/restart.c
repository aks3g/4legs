/*
 * restart.c
 *
 * Created: 2021/12/30 10:59:52
 *  Author: kiyot
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

#include "internal/backup_ram.h"

#include "lib4legs.h"

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#define BOOTLOADER_HEAD				(0x00000000)
#define REGISTORY_ADD				(0x0001C000)
#define FLASH_HEAD					(0x0001E000)
#define MAIN_FLASH_SIZE				(0x000E2000)

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void RESTART (volatile uint32_t *entry_addr, volatile uint32_t *stack_addr)
{
	volatile uint32_t stack_ptr = *stack_addr;
	volatile uint32_t start_adr = *entry_addr;

	__asm__ volatile ("MOV	r0, %[in]" : : [in] "r" (stack_ptr) : "r0");
	__asm__ volatile ("MOV	r1, %[in]" : : [in] "r" (start_adr) : "r1");
	__asm__ volatile ("MSR	MSP, r0");
	__asm__ volatile ("BX	r1");
}

/*--------------------------------------------------------------------------*/
static void _reboot(uint32_t head_addr)
{
	__disable_irq();

	// Reset all pheripherals
	samd51_tc_finalize(SAMD51_TC0);
	samd51_i2c_finalize(SAMD51_SERCOM2);
	samd51_external_interrupt_finalize();
	samd51_usb_finalize();

	NVIC->ICER[0] = 0xFFFFFFFFUL;
	NVIC->ICER[1] = 0xFFFFFFFFUL;
	NVIC->ICER[2] = 0xFFFFFFFFUL;
	NVIC->ICER[3] = 0xFFFFFFFFUL;
	NVIC->ICER[4] = 0xFFFFFFFFUL;
	NVIC->ICER[5] = 0xFFFFFFFFUL;
	NVIC->ICER[6] = 0xFFFFFFFFUL;
	NVIC->ICER[7] = 0xFFFFFFFFUL;

	NVIC->ICPR[0] = 0xFFFFFFFFUL;
	NVIC->ICPR[1] = 0xFFFFFFFFUL;
	NVIC->ICPR[2] = 0xFFFFFFFFUL;
	NVIC->ICPR[3] = 0xFFFFFFFFUL;
	NVIC->ICPR[4] = 0xFFFFFFFFUL;
	NVIC->ICPR[5] = 0xFFFFFFFFUL;
	NVIC->ICPR[6] = 0xFFFFFFFFUL;
	NVIC->ICPR[7] = 0xFFFFFFFFUL;

	//J Escape from Boot loader.
	SCB->VTOR = head_addr;
	__DSB();
	
	__enable_irq();

	RESTART((uint32_t *)(head_addr+4), (uint32_t *)head_addr);
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void lib4legs_update_request_and_reboot(uint32_t fw_size)
{
	gBackupRam.fw_size = fw_size;
	gBackupRam.update_req = 0xdeadbeef;

	lib4legs_jump_to_bootloader();

	return;
}

/*--------------------------------------------------------------------------*/
void lib4legs_restart(void)
{
	_reboot(FLASH_HEAD);
}

/*--------------------------------------------------------------------------*/
void lib4legs_jump_to_bootloader(void)
{
	_reboot(BOOTLOADER_HEAD);
}

/*--------------------------------------------------------------------------*/
void lib4legs_clear_update_req(void)
{
	gBackupRam.fw_size = 0xffffffff;
	gBackupRam.update_req = 0;
}
/*--------------------------------------------------------------------------*/
void lib4legs_set_update_req(void)
{
	gBackupRam.update_req = 0xdeadbeef;
}

/*--------------------------------------------------------------------------*/
int lib4legs_check_update_req(void)
{
	return (gBackupRam.update_req == 0xdeadbeef);
}

/*--------------------------------------------------------------------------*/
void lib4legs_set_force_bootloader(void)
{
	gBackupRam.update_req = 0xf0ceb001;
}

/*--------------------------------------------------------------------------*/
int lib4legs_check_force_bootloader(void)
{
	return (gBackupRam.update_req == 0xf0ceb001);
}

/*--------------------------------------------------------------------------*/
uint32_t lib4legs_get_fwsize_for_update(void)
{
	return gBackupRam.fw_size;
}

/*--------------------------------------------------------------------------*/
int lib4legs_check_valid_flash(void)
{
	if (*((uint32_t *)(FLASH_HEAD+4)) == 0xffffffff || 
	    *((uint32_t *)FLASH_HEAD    ) == 0xffffffff ) {
		return 0;
	}
	else {
		//J Flash‚Ì“à—e‚ª‘Ã“–‚ÆŽv‚í‚ê‚é
		return 1;
	}
}