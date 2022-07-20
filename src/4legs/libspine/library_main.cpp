/*
 * libspine.cpp
 *
 * Created: 2022/01/05 4:53:31
 * Author : kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <avr/wdt.h>

#include <libxmegaE5_error.h>
#include <libxmegaE5_utils.h>
#include <libxmegaE5_gpio.h>
#include <libxmegaE5_timer.h>
#include <libxmegaE5_i2c.h>
#include <libxmegaE5_servo.h>
#include <libxmegaE5_adc.h>

#include "libspine.h"

#if defined(TYPE_ARM)
#	define PWM_OUT0_PIN		(GPIO_PIN2)
#	define SERVO_ADC_CH0	(ADC_CH3)
#	define SERVO_ADC_CH1	(ADC_CH4)
#	define SERVO_ADC_CH2	(ADC_CH7)	// not used
#	define MOTOR_DIR_F		(GPIO_PIN6)
#	define MOTOR_DIR_R		(GPIO_PIN7)
#elif defined(TYPE_FINGER)
#	define PWM_OUT0_PIN		(GPIO_PIN2)
#	define PWM_OUT1_PIN		(GPIO_PIN4)
#	define PWM_OUT2_PIN		(GPIO_PIN6)
#	define SERVO_ADC_CH0	(ADC_CH3)
#	define SERVO_ADC_CH1	(ADC_CH5)
#	define SERVO_ADC_CH2	(ADC_CH7)
#else
#	define PWM_OUT0_PIN		(GPIO_PIN3)
#	define PWM_OUT1_PIN		(GPIO_PIN5)
#	define PWM_OUT2_PIN		(GPIO_PIN7)
#	define SERVO_ADC_CH0	(ADC_CH2)
#	define SERVO_ADC_CH1	(ADC_CH4)
#	define SERVO_ADC_CH2	(ADC_CH6)
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// I2C Interface
typedef struct {
	uint8_t addr;
	union {
		uint8_t arr[256];
		struct {
			volatile uint8_t  type;			// 0
			volatile uint8_t  status;		// 1
			volatile uint8_t  restart;		// 2
			volatile uint8_t  program;		// 3
			volatile uint8_t  debug;		// 4
			uint8_t  rsvd_0x05_0x07[3];		// 5 - 7
			uint8_t  servo_mode;			// 8
			uint8_t  servo_update;			// 9
			uint8_t  rsvd_0x0a_0x0f[6];		// a - f
			uint16_t servo0_pulse_us;		// 0x10 - 0x11
			uint16_t servo1_pulse_us;		// 0x12 - 0x13
			uint16_t servo2_pulse_us;		// 0x14 - 0x15
			uint16_t servo_ctrl_steps;		// 0x16 - 0x17
			uint16_t servo0_limit_min;		// 0x18 - 0x19
			uint16_t servo0_limit_max;		// 0x1a - 0x1b
			uint16_t servo1_limit_min;		// 0x1c - 0x1d
			uint16_t servo1_limit_max;		// 0x1e - 0x1f
			uint16_t servo2_limit_min;		// 0x20 - 0x21
			uint16_t servo2_limit_max;		// 0x22 - 0x23
			uint16_t servo0_pos;			// 0x24 - 0x25		
			uint16_t servo1_pos;			// 0x26 - 0x27
			uint16_t servo2_pos;			// 0x28 - 0x29
			uint16_t servo0_potention;		// 0x2A - 0x2B
			uint16_t servo1_potention;		// 0x2C - 0x2D
			uint16_t servo2_potention;		// 0x2E - 0x2F
			char     revision[16];			// 0x30 - 0x3F			
			uint8_t  rsvd_0x40_0x77[56];	// 0x40 - 0x77
			uint32_t flash_version;			// 0x78 - 0x7b
			uint32_t flash_addr;			// 0x7c - 0x7f
			uint8_t  flash_block[128];		// 0x80 - 0xff
		} map;
	} buf;
	uint16_t rx_len;
} I2C_BUF;
static I2C_BUF si2c_buf;

/*---------------------------------------------------------------------------*/
typedef struct {
	uint8_t ch_remap[3];
	int8_t  polarity;
} Lib4legsRemap;

static uint8_t sModuleId;
static const Lib4legsRemap sRemap[4]
=
{
	{{0,1,2}, 1},
	{{2,1,0},-1},
	{{0,1,2}, 1},
	{{2,1,0},-1}
};

#if defined(TYPE_ARM)
typedef struct
{
	uint16_t target;
	uint16_t active;
} Lib4legsLinerSlider;
static Lib4legsLinerSlider sSlider = {0, 0};
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static LibSpineServoUpdateCallbackFunc sServoUpdateCb = NULL;
static LibSpineSharedMemoryUpdatedCallbackFunc sSharedMemoryUpdatedCb = NULL;

static LibxmeaE5Servo sServo[3];

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void _initialzie_gpio(void)
{
	// PWM Output
#if defined(TYPE_ARM)
	libxmegaE5_gpio_initialize(GPIO_PORTA, PWM_OUT0_PIN, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_output(GPIO_PORTA, PWM_OUT0_PIN, 1);

	libxmegaE5_gpio_initialize(GPIO_PORTD, MOTOR_DIR_F, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_initialize(GPIO_PORTD, MOTOR_DIR_R, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_F, 1);
	libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_R, 1);

#else
	libxmegaE5_gpio_initialize(GPIO_PORTA, PWM_OUT0_PIN, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_initialize(GPIO_PORTA, PWM_OUT1_PIN, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_initialize(GPIO_PORTA, PWM_OUT2_PIN, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_output(GPIO_PORTA, PWM_OUT0_PIN, 1);
	libxmegaE5_gpio_output(GPIO_PORTA, PWM_OUT1_PIN, 1);
	libxmegaE5_gpio_output(GPIO_PORTA, PWM_OUT2_PIN, 1);
#endif
	// LED
	libxmegaE5_gpio_initialize(GPIO_PORTC, GPIO_PIN6, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_initialize(GPIO_PORTC, GPIO_PIN7, GPIO_OUT, GPIO_TOTEM);
	libxmegaE5_gpio_output(GPIO_PORTC, GPIO_PIN6, 1);
	libxmegaE5_gpio_output(GPIO_PORTC, GPIO_PIN7, 1);
	
	// Board ID
	libxmegaE5_gpio_initialize(GPIO_PORTC, GPIO_PIN4, GPIO_IN, GPIO_PULLDOWN);
	libxmegaE5_gpio_initialize(GPIO_PORTC, GPIO_PIN5, GPIO_IN, GPIO_PULLDOWN);

	// Interrupt
	libxmegaE5_gpio_initialize(GPIO_PORTC, GPIO_PIN2, GPIO_IN, GPIO_TOTEM);
		
	return;
}

static void _adc_ain0_cb(uint8_t ch, uint16_t val);
static void _adc_ain1_cb(uint8_t ch, uint16_t val);
static void _adc_ain2_cb(uint8_t ch, uint16_t val);

static void _adc_ain0_cb(uint8_t ch, uint16_t val)
{
	si2c_buf.buf.map.servo0_potention = val;
	
	libxmegaE5_adc_convert(SERVO_ADC_CH1, ADC_GAIN_1X, _adc_ain1_cb);
}
static void _adc_ain1_cb(uint8_t ch, uint16_t val)
{
	si2c_buf.buf.map.servo1_potention = val;
	
	libxmegaE5_adc_convert(SERVO_ADC_CH2, ADC_GAIN_1X, _adc_ain2_cb);
}
static void _adc_ain2_cb(uint8_t ch, uint16_t val)
{
	si2c_buf.buf.map.servo2_potention = val;	
}

/*---------------------------------------------------------------------------*/
static void _base_clock_cb(void)
{
	uint8_t in;
	libxmegaE5_gpio_input(GPIO_PORTC, GPIO_PIN2, &in);
	if(in) {
#if defined(TYPE_ARM)
		libxmegaE5_servo_start(&sServo[0]);
#else
		libxmegaE5_servo_start(&sServo[0]);
		libxmegaE5_servo_start(&sServo[1]);
		libxmegaE5_servo_start(&sServo[2]);
#endif
	}
	else {
		if (sServoUpdateCb != NULL) sServoUpdateCb();
		libxmegaE5_adc_convert(SERVO_ADC_CH0, ADC_GAIN_1X, _adc_ain0_cb);
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t _startCb(uint8_t mode)
{
	if (mode == I2C_MASTER_READ) {
		;
	}
	else if (mode == I2C_MASTER_WRITE) {
		si2c_buf.rx_len = 0;
	}
	
	return 0;
}

/*---------------------------------------------------------------------------*/
uint8_t _stopCb(void)
{
	if (sSharedMemoryUpdatedCb && si2c_buf.buf.map.status != 0) {
		sSharedMemoryUpdatedCb(si2c_buf.addr);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
uint8_t _tx(uint8_t ack)
{
	return si2c_buf.buf.arr[si2c_buf.addr++];
}

/*---------------------------------------------------------------------------*/
uint8_t _rx(uint8_t data)
{
	if (si2c_buf.rx_len == 0) {
		si2c_buf.addr = data;
	}
	else {
		si2c_buf.buf.arr[si2c_buf.addr] = data;
		
		switch (si2c_buf.addr)
		{
		case SPINE_MMAP_ADDR_TYPE:
		case SPINE_MMAP_ADDR_STATUS:
		case SPINE_MMAP_ADDR_RESTART:
		case SPINE_MMAP_ADDR_PROGRAM:
		case SPINE_MMAP_ADDR_DEBUG:
		case SPINE_MMAP_ADDR_SERVO_MODE:
		case SPINE_MMAP_ADDR_SERVO_UPDATE:
			si2c_buf.buf.map.status |= SPINE_MMAP_BUSY;
			break;
		default:
			si2c_buf.addr++;
			break;
		}
	}
	si2c_buf.rx_len++;

	return 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
uint8_t libspine_register_servo_update_cb(LibSpineServoUpdateCallbackFunc func)
{
	sServoUpdateCb = func;

	return LIB_XMEGA_E5_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libspine_update_servo_param(uint16_t servo0_us, uint16_t servo1_us, uint16_t servo2_us)
{
#if defined(TYPE_ARM)
	if (si2c_buf.buf.map.servo0_limit_min <= servo0_us && servo0_us <= si2c_buf.buf.map.servo0_limit_max) {
		libxmegaE5_servo_set_pulse_width(&sServo[sRemap[sModuleId].ch_remap[0]], servo0_us);
	}

	if (si2c_buf.buf.map.servo1_limit_min <= servo1_us && servo1_us <= si2c_buf.buf.map.servo1_limit_max) {
		if (sSlider.target != servo1_us) {
			sSlider.target = servo1_us;
			sSlider.active = 1;
		}
	}

#else
	if (si2c_buf.buf.map.servo0_limit_min <= servo0_us && servo0_us <= si2c_buf.buf.map.servo0_limit_max) {
		libxmegaE5_servo_set_pulse_width(&sServo[sRemap[sModuleId].ch_remap[0]], servo0_us);
	}

	if (si2c_buf.buf.map.servo1_limit_min <= servo1_us && servo1_us <= si2c_buf.buf.map.servo1_limit_max) {
		libxmegaE5_servo_set_pulse_width(&sServo[sRemap[sModuleId].ch_remap[1]], servo1_us);
	}

	if (si2c_buf.buf.map.servo2_limit_min <= servo2_us && servo2_us <= si2c_buf.buf.map.servo2_limit_max) {
		libxmegaE5_servo_set_pulse_width(&sServo[sRemap[sModuleId].ch_remap[2]], servo2_us);
	}
#endif
	return LIB_XMEGA_E5_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libspine_get_id(void)
{
	uint8_t id = 0;
	uint8_t in = 0;
	libxmegaE5_gpio_input(GPIO_PORTC, GPIO_PIN4, &in);
	if (in) id |= 0x01;

	libxmegaE5_gpio_input(GPIO_PORTC, GPIO_PIN5, &in);
	if (in) id |= 0x02;

	return id;
}

/*---------------------------------------------------------------------------*/
uint8_t libspine_shared_memory_updated_cb(LibSpineSharedMemoryUpdatedCallbackFunc func)
{
	if (func == NULL) {
		return SPINE_ERROR_NULL;
	}
	
	sSharedMemoryUpdatedCb = func;

	return SPINE_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libspine_shared_memory_write(uint8_t addr, uint8_t *buf, size_t len)
{
	if (buf == NULL) {
		return SPINE_ERROR_NULL;
	}
	if (len == 0) {
		return SPINE_ERROR_NOBUF;
	}
	if (((uint16_t)addr + len) > 256) {
		return SPINE_ERROR_INVAL;
	}

	memcpy(&(si2c_buf.buf.arr[addr]), buf, len);
	
	return SPINE_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
uint8_t libspine_shared_memory_read(uint8_t addr, uint8_t *buf, size_t len)
{
	if (buf == NULL) {
		return SPINE_ERROR_NULL;
	}
	if (len == 0) {
		return SPINE_ERROR_NOBUF;
	}
	if (((uint16_t)addr + len) > 256) {
		return SPINE_ERROR_INVAL;
	}

	memcpy(buf, &(si2c_buf.buf.arr[addr]), len);
	
	return SPINE_ERROR_OK;
}

/*---------------------------------------------------------------------------*/
int libspine_initialize(uint8_t fwtype, char *rev, size_t rev_size)
{
	//J GPIOを設定する
	_initialzie_gpio();
	
	//J Module IDを読み込む
	sModuleId = libspine_get_id();
	
	//J Servo 用のPWM 信号を生成する為のTimerを準備する
#if defined (TYPE_ARM)
	libxmegaE5_servo_initialize(&sServo[0], LIB_XMEGA_E5_TCC4, GPIO_PORTA, PWM_OUT0_PIN, 1, 32000000);
#else
	libxmegaE5_servo_initialize(&sServo[0], LIB_XMEGA_E5_TCC4, GPIO_PORTA, PWM_OUT0_PIN, 1, 32000000);
	libxmegaE5_servo_initialize(&sServo[1], LIB_XMEGA_E5_TCC5, GPIO_PORTA, PWM_OUT1_PIN, 1, 32000000);
	libxmegaE5_servo_initialize(&sServo[2], LIB_XMEGA_E5_TCD5, GPIO_PORTA, PWM_OUT2_PIN, 1, 32000000);
#endif
	//J 20msの基準クロックを外部からもらう
	libxmegaE5_gpio_configeInterrupt(GPIO_PORTC, GPIO_PIN2, INT_BOTHE_EDGE, _base_clock_cb);
	
	//J I2C を初期化し外部からの操作を受け取る
	LIB_XMEGA_E5_I2C_INIT_OPT i2c_opt;
	{
		i2c_opt.ownAddress = 0x30 + sModuleId;
		i2c_opt.startCB    = _startCb;
		i2c_opt.stopCB     = _stopCb;
		i2c_opt.masterRxCB = _tx;
		i2c_opt.masterTxCB = _rx;
	}
	libxmegaE5_i2c_initialize(100000, 32000000, I2C_SLAVE, &i2c_opt);

	//J 共有メモリを初期化
	memset(&si2c_buf, 0, sizeof(si2c_buf));
	si2c_buf.buf.map.type = fwtype;
	si2c_buf.buf.map.servo0_pulse_us = 1500;
	si2c_buf.buf.map.servo1_pulse_us = 1500;
	si2c_buf.buf.map.servo2_pulse_us = 1500;

	//J Fw Versionを入れる
	uint32_t pFwInfo = pgm_read_dword(0x2000 - 4);
	si2c_buf.buf.map.flash_version = pFwInfo;

	//J FW Revisionを設定する
	rev_size = (rev_size > 16) ? 16 : rev_size;
	memcpy((void *)si2c_buf.buf.map.revision, (void *)rev, rev_size);

	libspine_update_servo_param(si2c_buf.buf.map.servo0_pulse_us, si2c_buf.buf.map.servo1_pulse_us, si2c_buf.buf.map.servo2_pulse_us);

	//J ADCを初期化しておく
	ADC_INIT_OPT adc_opt;
	adc_opt.vrefSelect = VREF_EXTERNAL_VREF_ON_PORTA;
	libxmegaE5_adc_initialize(2000, 32000000, ADC_ONESHOT, 12, &adc_opt);

	//J Status LEDを更新
	libxmegaE5_gpio_output(GPIO_PORTC, GPIO_PIN6, 0);
	Enable_Int();

	return 0;	
}

/*---------------------------------------------------------------------------*/
// Restart
typedef void (*application_func_t)(void);
application_func_t application_start = (application_func_t)(0x0000);
application_func_t bootloader_start  = (application_func_t)(0x1000);

/*---------------------------------------------------------------------------*/
static void _swap_vector(int boot_sect)
{
	cli();
	if (boot_sect) {
		CPU_CCP    = CCP_IOREG_gc;  // Unlock reg.
		PMIC_CTRL  = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm | PMIC_IVSEL_bm; // Bootloader vector
	}
	else {
		CPU_CCP    = CCP_IOREG_gc;  // Unlock reg.
		PMIC_CTRL  = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; // Application vector
	}
	sei();
}

/*---------------------------------------------------------------------------*/
#define THRES_HOLD_P 20
#define THRES_HOLD_M -20
int libspine_poll(void)
{
#if defined(TYPE_ARM)
	if (sSlider.active) {

	libxmegaE5_gpio_output(GPIO_PORTC, GPIO_PIN6, 0);
	libxmegaE5_gpio_output(GPIO_PORTC, GPIO_PIN7, 0);

		int16_t diff = (int16_t)sSlider.target - (int16_t)si2c_buf.buf.map.servo1_potention;

		if (diff > THRES_HOLD_P) {
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_F, 1);
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_R, 0);
		}
		else if (diff < THRES_HOLD_M){
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_F, 0);
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_R, 1);
		}
		else {
			sSlider.active = 0;
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_F, 1);
			libxmegaE5_gpio_output(GPIO_PORTD, MOTOR_DIR_R, 1);
		}
		
	}
#endif

	if(si2c_buf.buf.map.status == 0) {
		return -1;
	}

	if (si2c_buf.addr == SPINE_MMAP_ADDR_RESTART) {
		
		if (si2c_buf.buf.map.restart == SPINE_RESTART_KEY) {
			_swap_vector(0);
			application_start();
			while(1);
		}
		else if (si2c_buf.buf.map.restart == SPINE_RESTART_BOOTLOADER_KEY) {
			wdt_enable(WDTO_15MS);
			while(1) {};
		} 
	}
	else if (si2c_buf.addr == SPINE_MMAP_ADDR_SERVO_UPDATE) {
	}




	si2c_buf.buf.map.status = 0;

	return 0;
}

uint8_t libspine_get_servo_mode(void)
{
	return si2c_buf.buf.map.servo_mode;
}


void libspine_load_servo_param_from_shared_memory_all(void)
{
	libspine_update_servo_param(si2c_buf.buf.map.servo0_pulse_us, si2c_buf.buf.map.servo1_pulse_us, si2c_buf.buf.map.servo2_pulse_us);
}


uint8_t libspine_get_target_angles(int16_t *angle0, int16_t *angle1, int16_t *angle2)
{
	*angle0 = (int16_t)si2c_buf.buf.map.servo0_pulse_us;
	*angle1 = (int16_t)si2c_buf.buf.map.servo1_pulse_us;
	*angle2 = (int16_t)si2c_buf.buf.map.servo2_pulse_us;

	return 0;
}

void libspine_load_servo_param_by_degree(int16_t angle0, int16_t angle1, int16_t angle2)
{
//	int16_t delta0_us = (int)(1000.0f * ((float)angle0 / 100.0f) / 90.0f);
//	int16_t delta1_us = (int)(1000.0f * ((float)angle1 / 100.0f) / 90.0f);
//	int16_t delta2_us = (int)(1000.0f * ((float)angle2 / 100.0f) / 90.0f);
	int16_t delta0_us = (int)(angle0 / 9);
	int16_t delta1_us = (int)(angle1 / 9);
	int16_t delta2_us = (int)(angle2 / 9);

	uint16_t pulse0_us = sRemap[sModuleId].polarity * delta0_us + si2c_buf.buf.map.servo0_pos;
	uint16_t pulse1_us = sRemap[sModuleId].polarity * delta1_us + si2c_buf.buf.map.servo1_pos;
	uint16_t pulse2_us = sRemap[sModuleId].polarity * delta2_us + si2c_buf.buf.map.servo2_pos;

	libspine_update_servo_param(pulse0_us, pulse1_us, pulse2_us);

	return;
}

