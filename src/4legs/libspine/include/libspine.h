/*
 * libspine.h
 *
 * Created: 2022/01/05 4:54:01
 *  Author: kiyot
 */ 


#ifndef LIBSPINE_H_
#define LIBSPINE_H_

#define SPINE_MMAP_ADDR_TYPE				0x00
#define SPINE_MMAP_ADDR_STATUS				0x01
#define SPINE_MMAP_ADDR_RESTART				0x02
#define SPINE_MMAP_ADDR_PROGRAM				0x03
#define SPINE_MMAP_ADDR_DEBUG				0x04

#define SPINE_MMAP_ADDR_SERVO_MODE			0x08
#define SPINE_MMAP_ADDR_SERVO_UPDATE		0x09
#define SPINE_MMAP_ADDR_SERVO0_PULSE_US_L	0x10
#define SPINE_MMAP_ADDR_SERVO0_PULSE_US_H	0x11
#define SPINE_MMAP_ADDR_SERVO1_PULSE_US_L	0x12
#define SPINE_MMAP_ADDR_SERVO1_PULSE_US_H	0x13
#define SPINE_MMAP_ADDR_SERVO2_PULSE_US_L	0x14
#define SPINE_MMAP_ADDR_SERVO2_PULSE_US_H	0x15

#define SPINE_MMAP_ADDR_SERVO_CTRL_STEPS_L	0x16
#define SPINE_MMAP_ADDR_SERVO_CTRL_STEPS_H	0x17

#define SPINE_MMAP_ADDR_SERVO0_LIMIT_MIN_L	0x18
#define SPINE_MMAP_ADDR_SERVO0_LIMIT_MIN_H	0x19
#define SPINE_MMAP_ADDR_SERVO0_LIMIT_MAX_L	0x1a
#define SPINE_MMAP_ADDR_SERVO0_LIMIT_MAX_H	0x1b

#define SPINE_MMAP_ADDR_SERVO1_LIMIT_MIN_L	0x1c
#define SPINE_MMAP_ADDR_SERVO1_LIMIT_MIN_H	0x1d
#define SPINE_MMAP_ADDR_SERVO1_LIMIT_MAX_L	0x1e
#define SPINE_MMAP_ADDR_SERVO1_LIMIT_MAX_H	0x1f

#define SPINE_MMAP_ADDR_SERVO2_LIMIT_MIN_L	0x20
#define SPINE_MMAP_ADDR_SERVO2_LIMIT_MIN_H	0x21
#define SPINE_MMAP_ADDR_SERVO2_LIMIT_MAX_L	0x22
#define SPINE_MMAP_ADDR_SERVO2_LIMIT_MAX_H	0x23

#define SPINE_MMAP_ADDR_SERVO0_POS0_L		0x24
#define SPINE_MMAP_ADDR_SERVO0_POS0_H		0x25
#define SPINE_MMAP_ADDR_SERVO1_POS0_L		0x26
#define SPINE_MMAP_ADDR_SERVO1_POS0_H		0x27
#define SPINE_MMAP_ADDR_SERVO2_POS0_L		0x28
#define SPINE_MMAP_ADDR_SERVO2_POS0_H		0x29

#define SPINE_MMAP_ADDR_SERVO0_POTENTION_L	0x2A
#define SPINE_MMAP_ADDR_SERVO0_POTENTION_H	0x2B
#define SPINE_MMAP_ADDR_SERVO1_POTENTION_L	0x2C
#define SPINE_MMAP_ADDR_SERVO1_POTENTION_H	0x2D
#define SPINE_MMAP_ADDR_SERVO2_POTENTION_L	0x2E
#define SPINE_MMAP_ADDR_SERVO2_POTENTION_H	0x2F

#define SPINE_MMAP_ADDR_REVISION_STR		0x30

#define SPINE_MMAP_ADDR_FW_VERSION			0x78 // to 0x7b

#define SPINE_MMAP_ADDR_FLASH_ADDR0			0x7C // LSB
#define SPINE_MMAP_ADDR_FLASH_ADDR1			0x7D
#define SPINE_MMAP_ADDR_FLASH_ADDR2			0x7E
#define SPINE_MMAP_ADDR_FLASH_ADDR3			0x7F // MSB
#define SPINE_MMAP_ADDR_FLASH_BUF_HEAD		0x80



#define SPINE_MMAP_BUSY						0x01
#define SPINE_RESTART_KEY					0x66
#define SPINE_RESTART_BOOTLOADER_KEY		0xA8
#define SPINE_PROGRAM_KEY					0x89

#define SPINE_MMAP_TYPE_BOOTLOADER			0x00
#define SPINE_MMAP_TYPE_APPLICATION			0x01

#define SPINE_MMAP_ADDR_SERVO_MODE_DIRECT	0x00
#define SPINE_MMAP_ADDR_SERVO_MODE_ANGLE	0x01
#define SPINE_MMAP_ADDR_SERVO_MODE_SEQENCE	0x80

#define SPINE_MMAP_REVISION_STR_LEN			16

#define SPINE_ERROR_OK						(0x00)
#define SPINE_ERROR_NULL					(-1)
#define SPINE_ERROR_INVAL					(-2)
#define SPINE_ERROR_NODEV					(-3)
#define SPINE_ERROR_NOBUF					(-4)

int libspine_initialize(uint8_t fwtype, char *rev, size_t rev_size);
int libspine_poll(void);
void libspine_load_servo_param_from_shared_memory_all(void);

uint8_t libspine_get_id(void);

typedef void (*LibSpineServoUpdateCallbackFunc)(void);
uint8_t libspine_register_servo_update_cb(LibSpineServoUpdateCallbackFunc func);

uint8_t libspine_get_servo_mode(void);
uint8_t libspine_update_servo_param(uint16_t servo0_us, uint16_t servo1_us, uint16_t servo2_us);
uint8_t libspine_get_target_angles(int16_t *angle0, int16_t *angle1, int16_t *angle2);
void libspine_load_servo_param_by_degree(int16_t angle0, int16_t angle1, int16_t angle2);

typedef void (*LibSpineSharedMemoryUpdatedCallbackFunc)(uint8_t addr);
uint8_t libspine_shared_memory_updated_cb(LibSpineSharedMemoryUpdatedCallbackFunc func);
uint8_t libspine_shared_memory_write(uint8_t addr, uint8_t *buf, size_t len);
uint8_t libspine_shared_memory_read(uint8_t addr, uint8_t *buf, size_t len);

#endif /* LIBSPINE_H_ */