/*
 * lib4legs.h
 *
 * Created: 2021/12/31 4:44:58
 *  Author: kiyot
 */ 
#include <stdint.h>
#include <stddef.h>

#include <samd51_gpio.h>


#define VCC_EN_PIN		SAMD51_GPIO_A02
#define VREF_PIN		SAMD51_GPIO_A03
#define SW_SENS_PIN		SAMD51_GPIO_B04
#define POWER_ON_PIN	SAMD51_GPIO_B05
#define EN_7V4_PIN		SAMD51_GPIO_B06
#define EN_6V0_PIN		SAMD51_GPIO_B07
#define VBAT_SENS_A_PIN	SAMD51_GPIO_B08
#define VCC_SENS_A_PIN	SAMD51_GPIO_B09
#define DCDC_PG_PIN		SAMD51_GPIO_A04

#define BP_S_PIN		SAMD51_GPIO_A07
#define SCL_S_PIN		SAMD51_GPIO_A08
#define SDA_S_PIN		SAMD51_GPIO_A09

#define LED0_PIN		SAMD51_GPIO_B12

#define VBUS_DET_PIN	SAMD51_GPIO_A06
#define USB_DM_PIN		SAMD51_GPIO_A24
#define USB_DP_PIN		SAMD51_GPIO_A25

#define INT0_PIN		SAMD51_GPIO_B00
#define INT1_PIN		SAMD51_GPIO_B01
#define INT2_PIN		SAMD51_GPIO_B02
#define INT3_PIN		SAMD51_GPIO_B03
#define SDA_PIN			SAMD51_GPIO_A12
#define SCL_PIN			SAMD51_GPIO_A13


/* Call before using any library functions. */
int lib4legs_initialzie(void);


/* UART Interface */
int lib4legs_linked(void);

int lib4legs_putc(const char c);
int lib4legs_puts(const char *s);
int lib4legs_printf(const char *format, ...);
int lib4legs_getc(char *c, int timeout);

int lib4legs_tx(const uint8_t *buf, const size_t len);
int lib4legs_rx(uint8_t *buf, const size_t len, const int blocking);

/* LED */
#define LIB4LEGS_LED0			(0x01)
#define LIB4LEGS_LED1			(0x02)
#define LIB4LEGS_LED2			(0x04)

void lib4legs_led_set(uint8_t bitmap);
void lib4legs_led_clear(uint8_t bitmap);
void lib4legs_led_toggle(uint8_t bitmap);

/* GPIO */
inline int lib4legs_vbus_det_stat(void)
{
	return samd51_gpio_input(VBUS_DET_PIN);
}


/* Sensor */
typedef struct {
	float gyro[3];
	float accel[3];
} LIB4LEGS_IMU_DATA;

typedef void (*LIB4LEGS_SENSOR_CAPTUERED_CB)(LIB4LEGS_IMU_DATA *data);

int lib4legs_register_sensor_captured_cb(LIB4LEGS_SENSOR_CAPTUERED_CB cb);

/* Timer */
typedef void (*LIB4LEGS_TIMER_CB)(void);
int lib4legs_timer_register_cb(LIB4LEGS_TIMER_CB cb);
uint32_t lib4legs_timer_get_tick(void);
void lib4legs_timer_delay_ms(uint32_t ms);

/* Spine IF */
typedef enum
{
	SPINE0 = 0,
	SPINE1,
	SPINE2,
	SPINE3,
	SPINE_ARM,
	SPINE_FINGER,
	MAX_SPINE
} SPINE;

typedef enum
{
	SERVO_ID_BODY = 0,
	SERVO_ID_2ND_JOINT,
	SERVO_ID_1ST_JOINT
} SPINE_SERVO_ID;

typedef struct {
	uint16_t val[3];
} SPINE_POTENTION;

typedef enum
{
	SPINE_MODE_DIRECT = 0x00,
	SPINE_MODE_ANGLE  = 0x01
} SPINE_MODE;

int lib4legs_spine_if_read_byte(const SPINE spine, const uint8_t addr, uint8_t *dat);

int lib4legs_spine_if_write_byte(const SPINE spine, const uint8_t addr, const uint8_t dat);

int lib4legs_spine_if_get_type(const SPINE spine, uint8_t *type);
int lib4legs_spine_if_get_status(const SPINE spine, uint8_t *status);
int lib4legs_spine_if_exec_restart(const SPINE spine);
int lib4legs_spine_if_exec_bootloader(const SPINE spine);
int lib4legs_spine_if_exec_program(const SPINE spine);
int lib4legs_spine_if_exec_debug(const SPINE spine, uint8_t val);
int lib4legs_spine_if_write_flash(const SPINE spine, const uint32_t addr, const uint8_t *page);

int lib4legs_spine_if_get_fw_version(const SPINE spine, uint32_t *version);

int lib4legs_spine_if_update(const SPINE spine, const uint8_t *fwbuf, size_t size);
int lib4legs_spine_if_set_limit(const SPINE spine, const SPINE_SERVO_ID servo, uint16_t min_limit, uint16_t max_limit);
int lib4legs_spine_if_set_origin(const SPINE spine, uint16_t servo0_orig, uint16_t servo1_orig, uint16_t servo2_orig);
int lib4legs_spine_if_set_servo_mode(const SPINE spine, SPINE_MODE mode, int enable_seq);
int lib4legs_spine_if_set_servo_pulse_width(const SPINE spine, uint16_t s0_us, uint16_t s1_us, uint16_t s2_us);
int lib4legs_spine_if_get_servo_pulse_width(const SPINE spine, uint16_t *s0_us, uint16_t *s1_us, uint16_t *s2_us);
int lib4legs_spine_if_update_servo(const SPINE spine);
int lib4legs_spine_if_get_potention(const SPINE spine, SPINE_POTENTION *potention);


/* Control Pulse */
typedef void (*Lib4legsUpdatePulseCb)(void);
void lib4legs_enable_base_pulse(int set);
int lib4legs_register_base_pulse_cb(Lib4legsUpdatePulseCb func);

/* Restart and Reboot */
void lib4legs_update_request_and_reboot(uint32_t fw_size);
void lib4legs_restart(void);
void lib4legs_jump_to_bootloader(void);
void lib4legs_clear_update_req(void);
void lib4legs_set_update_req(void);
int lib4legs_check_update_req(void);
void lib4legs_set_force_bootloader(void);
int lib4legs_check_force_bootloader(void);
uint32_t lib4legs_get_fwsize_for_update(void);
int lib4legs_check_valid_flash(void);

/* Power */
int lib4legs_power_ctrl_dcdc(int enable);
int lib4legs_power_ctrl_6v0(int enable);
int lib4legs_power_ctrl_7v4(int enable);
uint32_t lib4legs_power_get_vbat(void);
uint32_t lib4legs_power_get_vcc(void);
int lib4legs_power_check_power_good(void);


/* Error Code */
#define LIB4LEGS_ERROR_OK							(0)
#define LIB4LEGS_ERROR_NULL							(0x80500000)
#define LIB4LEGS_ERROR_INVAL						(0x80500001)
#define LIB4LEGS_ERROR_NOBUF						(0x80500002)
#define LIB4LEGS_ERROR_NODEV						(0x80500003)
#define LIB4LEGS_ERROR_BUSY							(0x80500004)
#define LIB4LEGS_ERROR_NOT_NEEDED					(0x80500005)

#define LIB4LEGS_ERROR_SPINE_IF_NOTFOUND			(0x80510000)
#define LIB4LEGS_ERROR_SPINE_IF_NULL				(0x80510001)
#define LIB4LEGS_ERROR_SPINE_IF_NOBUF				(0x80510002)

#define LIB4LEGS_ERROR_POWER_INVALID				(0x80520000)