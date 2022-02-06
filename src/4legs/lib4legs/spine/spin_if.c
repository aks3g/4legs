/*
 * spin_if.c
 *
 * Created: 2021/12/30 11:01:37
 *  Author: kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <samd51_sercom.h>
#include <samd51_i2c.h>

#include "lib4legs.h"
#include "libspine.h"


static SAMD51_SERCOM sSercom;
static uint8_t sBuf[256 + 1];

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static uint8_t _spineSlaveAddr(const SPINE spine)
{
	switch (spine) {
	case SPINE0:
	case SPINE1:
	case SPINE2:
	case SPINE3:
		return (uint8_t)spine + 0x30;
	default:
		return 0;
	}
}

/*--------------------------------------------------------------------------*/
static void _spine_write(const uint8_t spine_addr, const uint8_t addr, const uint8_t *buf, size_t len)
{
	// Setup Tx Buffer
	sBuf[0] = addr;
	memcpy(&(sBuf[1]), buf, len);

	samd51_i2c_txrx(sSercom, spine_addr, sBuf, len+1, NULL, 0, NULL);
	return;
}

/*--------------------------------------------------------------------------*/
static void _spine_read(const uint8_t spine_addr, const uint8_t addr, uint8_t *buf, size_t len)
{
	// Setup Tx Buffer
	sBuf[0] = addr;

	samd51_i2c_txrx(sSercom, spine_addr, sBuf, 1, buf, len, NULL);
	return;
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int inialize_spine_if(SAMD51_SERCOM i2c_if)
{
	sSercom = i2c_if;
	
	return LIB4LEGS_ERROR_OK;
}


int lib4legs_spine_if_read_byte(const SPINE spine, const uint8_t addr, uint8_t *dat)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	_spine_read(spine_slave_addr, addr, dat, 1);
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_write_byte(const SPINE spine, const uint8_t addr, const uint8_t dat)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	_spine_write(spine_slave_addr, addr, &dat, 1);
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_update(const SPINE spine, const uint8_t *fwbuf, size_t size)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	// Update Firmware
	uint32_t addr = 0;
	uint32_t aligned_size = (size + 127) & 0xffffff80;
	uint8_t  program_key = SPINE_PROGRAM_KEY;
	uint8_t  status = 0;
	while (addr != aligned_size) {
		_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_FLASH_ADDR0,    (uint8_t *)&addr, 4);
		_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_FLASH_BUF_HEAD, &(fwbuf[addr]), 128);
		_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_PROGRAM,        &program_key,   1);

		do {
			lib4legs_timer_delay_ms(100);
			_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_STATUS, &status, 1);
		} while(status == SPINE_MMAP_BUSY);

		addr += 128;
	}

	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_exec_restart(const SPINE spine)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	uint8_t reboot_key = SPINE_RESTART_KEY;
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_RESTART, &reboot_key,   1);
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_exec_bootloader(const SPINE spine)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	uint8_t reboot_key = SPINE_RESTART_BOOTLOADER_KEY;
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_RESTART, &reboot_key,   1);
	
	return LIB4LEGS_ERROR_OK;
}


int lib4legs_spine_if_set_servo_pulse_width(const SPINE spine, uint16_t s0_us, uint16_t s1_us, uint16_t s2_us)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO0_PULSE_US_L, (uint8_t *)&s0_us, sizeof(uint16_t));
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO1_PULSE_US_L, (uint8_t *)&s1_us, sizeof(uint16_t));
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO2_PULSE_US_L, (uint8_t *)&s2_us, sizeof(uint16_t));

	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_get_servo_pulse_width(const SPINE spine, uint16_t *s0_us, uint16_t *s1_us, uint16_t *s2_us)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_SERVO0_PULSE_US_L, (uint8_t *)s0_us, sizeof(uint16_t));
	_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_SERVO1_PULSE_US_L, (uint8_t *)s1_us, sizeof(uint16_t));
	_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_SERVO2_PULSE_US_L, (uint8_t *)s2_us, sizeof(uint16_t));

	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_update_servo(const SPINE spine)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	uint8_t update = 1;
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO_UPDATE, &update,   1);
	
	return LIB4LEGS_ERROR_OK;		
}

int lib4legs_spine_if_set_limit(const SPINE spine, const SPINE_SERVO_ID servo, uint16_t min_limit, uint16_t max_limit)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	uint8_t offset = SPINE_MMAP_ADDR_SERVO0_LIMIT_MIN_L + (int)servo * 4;

	_spine_write(spine_slave_addr, offset+0, (uint8_t *)&min_limit, sizeof(uint16_t));
	_spine_write(spine_slave_addr, offset+2, (uint8_t *)&max_limit, sizeof(uint16_t));
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_set_origin(const SPINE spine, uint16_t servo0_orig, uint16_t servo1_orig, uint16_t servo2_orig)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}
	uint16_t buf[3];
	buf[0] = servo0_orig;
	buf[1] = servo1_orig;
	buf[2] = servo2_orig;
	
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO0_POS0_L, (uint8_t *)buf, sizeof(uint16_t) * 3);
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_set_servo_mode(const SPINE spine, SPINE_MODE mode, int enable_seq)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}

	uint8_t _mode = (uint8_t)mode;
	if (enable_seq) _mode |= 0x80;
	
	_spine_write(spine_slave_addr, SPINE_MMAP_ADDR_SERVO_MODE, &_mode, sizeof(uint8_t));
	
	return LIB4LEGS_ERROR_OK;	
}

int lib4legs_spine_if_get_type(const SPINE spine, uint8_t *type)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}
	if (type == NULL) {
		return LIB4LEGS_ERROR_NULL;
	}

	_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_TYPE, type, sizeof(uint8_t));
	
	return LIB4LEGS_ERROR_OK;	
}

int lib4legs_spine_if_get_fw_version(const SPINE spine, uint32_t *version)
{
	uint8_t spine_slave_addr = _spineSlaveAddr(spine);
	if (spine_slave_addr == 0) {
		return LIB4LEGS_ERROR_SPINE_IF_NOTFOUND;
	}
	if (version == NULL) {
		return LIB4LEGS_ERROR_NULL;
	}

	_spine_read(spine_slave_addr, SPINE_MMAP_ADDR_FW_VERSION, (uint8_t *)version, sizeof(uint32_t));
	
	return LIB4LEGS_ERROR_OK;
}

int lib4legs_spine_if_get_status(const SPINE spine, uint8_t *status);

int lib4legs_spine_if_exec_program(const SPINE spine);
int lib4legs_spine_if_exec_debug(const SPINE spine, uint8_t val);

