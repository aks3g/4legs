/*
 * spine_controller.c
 *
 * Created: 2022/02/04 6:28:56
 *  Author: kiyot
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib4legs.h"
#include "libspine.h"

extern uint8_t _binary_spine_bin_start;
extern uint8_t _binary_spine_bin_end;
extern uint8_t _binary_spine_bin_size;

static uint8_t _spinefw_buf[8*1024];

#define SPINE_FW_SIG		(0x59100000)
#define SPINE_FW_VERSION	(0x00000002)
static const uint32_t cSpineFwVersion = SPINE_FW_SIG | SPINE_FW_VERSION;

int cerebellum_spine_version_check_and_update(SPINE spine, int force)
{
	// Ensure in application mode to check fw version.
	uint8_t type = 0;
	lib4legs_spine_if_get_type(spine, &type);
	if (type != SPINE_MMAP_TYPE_APPLICATION) {
		lib4legs_spine_if_exec_restart(spine);
		lib4legs_timer_delay_ms(30);
	}

	// Check fw version.
	uint32_t version = 0;
	lib4legs_spine_if_get_fw_version(spine, &version);	
	if ( ((version & 0xffff0000) == SPINE_FW_SIG) && ((version & 0xffff) == SPINE_FW_VERSION) ) {
		return LIB4LEGS_ERROR_NOT_NEEDED;
	} 

	// Enter bootloader mode.
	lib4legs_spine_if_exec_bootloader(spine);
	lib4legs_timer_delay_ms(100);

	uint8_t *p_spine_fw = &_binary_spine_bin_start;
	uint32_t spine_fw_size = (uint32_t)&_binary_spine_bin_size;

	if (sizeof(_spinefw_buf) < spine_fw_size) {
		return LIB4LEGS_ERROR_NOBUF;
	}

	//J 8KiBのバッファにファームとVersion情報を付与
	memset(_spinefw_buf, 0xff, sizeof(_spinefw_buf));
	memcpy(_spinefw_buf, p_spine_fw, spine_fw_size);
	memcpy(&_spinefw_buf[sizeof(_spinefw_buf) - 4], &cSpineFwVersion, sizeof(cSpineFwVersion));

	lib4legs_spine_if_update(spine, _spinefw_buf, sizeof(_spinefw_buf));

	//J Restart.
	lib4legs_spine_if_exec_restart(spine);

	return LIB4LEGS_ERROR_OK;	
}