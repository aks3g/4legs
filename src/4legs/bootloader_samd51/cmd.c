/*
 * cmd.c
 *
 * Created: 2022/01/06 5:47:57
 *  Author: kiyot
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <lib4legs.h>
#include <libspine.h>
#include <console.h>

#include "xmodem.h"


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern void lib4legs_bootloader_update_main_fw(uint32_t fwsize);

typedef struct  
{
	uint32_t magic;
	uint32_t version;
	uint32_t fwsize;
	uint32_t sum;
	uint8_t  pad[112];
} UpdFileHeader;

static XmodemContext sXmodemCtx;
static uint8_t sSpineFwBin[8 * 1024 + sizeof(UpdFileHeader)];

static uint8_t _spine_fw_receive_cb(uint8_t status, const XmodemContext *ctx)
{
	if (ctx->received_size >= sizeof(sSpineFwBin)) {
		return 0;
	}

	memcpy(&(sSpineFwBin[ctx->received_size]), ctx->buf.fd.data, ctx->received_block_size);
	return 0;
}

static uint32_t _checksum32(uint32_t *arr, size_t size)
{
	uint32_t sum = 0;
	for (size_t i=0 ; i<size/sizeof(uint32_t) ; ++i) {
		sum += arr[i];
	}

	return sum;
}

const uint32_t cSpineFwMagic  = 0xaba00001;
const uint32_t cArmFwMagic    = 0xaba00002;
const uint32_t cFingerFwMagic = 0xaba00003;

#define ERROR_FWUPDATE_INVALID_SIZE			(-1)
#define ERROR_FWUPDATE_INVALID_MAGIC		(-2)
#define ERROR_FWUPDATE_CHECKSUM_ERROR		(-3)

static int _validate_spine_fw(uint8_t *fwbin, uint32_t received_size, uint32_t magic, uint32_t *fwsize)
{
	UpdFileHeader *header = (UpdFileHeader *)fwbin;

	if (header->fwsize != received_size - sizeof(UpdFileHeader)) {
		lib4legs_printf("Expected 0x%08x\n", header->fwsize );
		lib4legs_printf("Received 0x%08x\n", received_size);

		return ERROR_FWUPDATE_INVALID_SIZE;
	}
	
	uint32_t sum = 0;
	if ((sum = _checksum32((uint32_t *)&(fwbin[sizeof(UpdFileHeader)]), header->fwsize)) != header->sum) {
		lib4legs_printf("Expected 0x%08x\n", magic);
		lib4legs_printf("Received 0x%08x\n", header->sum);

		return ERROR_FWUPDATE_CHECKSUM_ERROR;
	}

	if (_checksum32((uint32_t *)&(fwbin[sizeof(UpdFileHeader)]), header->fwsize) != header->sum) {
		return ERROR_FWUPDATE_CHECKSUM_ERROR;
	}

	return 0;
}

static int update_command(const int argc, const char **argv)
{
	// check argument	
	uint32_t fwsize = 0;
	lib4legs_printf("Update arg %s\n", argv[0]);

	int spine_only = 0;
	int finger_only = 0;
	int arm_only = 0;
	if (argc >= 1) {
		if (0 == strncmp(argv[0], "-s", 2)) {
			spine_only = 1;
		}
		else if (0 == strncmp(argv[0], "-f", 2)) {
			finger_only = 1;
		}
		else if (0 == strncmp(argv[0], "-a", 2)) {
			arm_only = 1;
		}
		else {
			lib4legs_printf("Unknown option %s\n", argv[1]);
			return LIB4LEGS_ERROR_INVAL;
		}
	}

	if (spine_only) {
		lib4legs_printf("Update Spine Firmware.\n");
		if (fwsize > 8*1024) {
			lib4legs_printf("Fw size is too large. %u\n", fwsize);
			return LIB4LEGS_ERROR_NOBUF;
		}
	
		// Read firmware upto 8KiB from Xmodem
		memset (sSpineFwBin, 0xff, sizeof(sSpineFwBin));
				
		// Wait for few seconds.
		for (int i=0 ; i<10 ; ++i) {
			lib4legs_printf("Wait %2d\r", 10-i);
			lib4legs_timer_delay_ms(1000);
		}

		xmodem_receive(&sXmodemCtx, _spine_fw_receive_cb);
		int ret = _validate_spine_fw(sSpineFwBin, sXmodemCtx.received_size, cSpineFwMagic, &fwsize);
		if (ret != 0) {
			lib4legs_printf("Update File is corrupted. Code = %d\n", ret);
			return -1;
		}

		// Update via Spine If
		lib4legs_spine_if_update(SPINE0, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Spine 0 FW Update done.\n");

		lib4legs_spine_if_update(SPINE1, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Spine 1 FW Update done.\n");

		lib4legs_spine_if_update(SPINE2, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Spine 2 FW Update done.\n");

		lib4legs_spine_if_update(SPINE3, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Spine 3 FW Update done.\n");

		lib4legs_printf("Spine FW Update done.\n");
	}
	else if (finger_only) {
		lib4legs_printf("Update Finger Firmware.\n");
		if (fwsize > 8*1024) {
			lib4legs_printf("Fw size is too large. %u\n", fwsize);
			return LIB4LEGS_ERROR_NOBUF;
		}

		// Read firmware upto 8KiB from Xmodem
		memset (sSpineFwBin, 0xfc, sizeof(sSpineFwBin));
		
		// Wait for few seconds.
		for (int i=0 ; i<10 ; ++i) {
			lib4legs_printf("Wait %2d\r", 10-i);
			lib4legs_timer_delay_ms(1000);
		}

		xmodem_receive(&sXmodemCtx, _spine_fw_receive_cb);
		int ret = _validate_spine_fw(sSpineFwBin, sXmodemCtx.received_size, cFingerFwMagic, &fwsize);
		if (ret != 0) {
			lib4legs_printf("Update File is corrupted. Code = %d\n", ret);
			return -1;
		}

		lib4legs_spine_if_update(SPINE_FINGER, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Finger FW Update done.\n");
	}
	else if (arm_only) {
		lib4legs_printf("Update Arm Firmware.\n");
		if (fwsize > 8*1024) {
			lib4legs_printf("Fw size is too large. %u\n", fwsize);
			return LIB4LEGS_ERROR_NOBUF;
		}

		// Read firmware upto 8KiB from Xmodem
		memset (sSpineFwBin, 0xaa, sizeof(sSpineFwBin));
		
		// Wait for few seconds.
		for (int i=0 ; i<10 ; ++i) {
			lib4legs_printf("Wait %2d\r", 10-i);
			lib4legs_timer_delay_ms(1000);
		}

		xmodem_receive(&sXmodemCtx, _spine_fw_receive_cb);
		int ret = _validate_spine_fw(sSpineFwBin, sXmodemCtx.received_size, cArmFwMagic, &fwsize);
		if (ret != 0) {
			lib4legs_printf("Update File is corrupted. Code = %d\n", ret);
			return -1;
		}

		lib4legs_spine_if_update(SPINE_ARM, &(sSpineFwBin[128]), fwsize);
		lib4legs_printf("Arm FW Update done.\n");
	}
	else {
		lib4legs_printf("Update All Firmware.\n");
		// Wait for few seconds.
		for (int i=0 ; i<10 ; ++i) {
			lib4legs_printf("Wait %2d\r", 10-i);
			lib4legs_timer_delay_ms(1000);
		}

		lib4legs_bootloader_update_main_fw(fwsize);
		lib4legs_restart();
	}

	return 0;
}

static const char *update_help(void)
{
	return 
		"Update Firmwares\n"
		"Update all firmware | update <size>\n"
		"Update Spine only   | update <size> -s\n";
}

static ConsoleCommand update_cmd =
{
	.name = "update",
	.func = update_command,
	.help = update_help,
};


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int boot_command(const int argc, const char **argv)
{
	lib4legs_restart();
	return 0;
}

static const char *boot_help(void)
{
	return "Boot MCU";
}

static ConsoleCommand boot_cmd =
{
	.name = "boot",
	.func = boot_command,
	.help = boot_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int boot_spine_command(const int argc, const char **argv)
{
	if (argc == 0) {
		lib4legs_printf("Spines goto Application mode\n");
		lib4legs_spine_if_exec_restart(SPINE0);
		lib4legs_spine_if_exec_restart(SPINE1);
		lib4legs_spine_if_exec_restart(SPINE2);
		lib4legs_spine_if_exec_restart(SPINE3);
		lib4legs_spine_if_exec_restart(SPINE_FINGER);
		lib4legs_spine_if_exec_restart(SPINE_ARM);
	}
	else if (0 == strncmp("-b", argv[0], 2)){
		lib4legs_printf("Spines goto Bootloader mode\n");
		lib4legs_spine_if_exec_bootloader(SPINE0);
		lib4legs_spine_if_exec_bootloader(SPINE1);
		lib4legs_spine_if_exec_bootloader(SPINE2);
		lib4legs_spine_if_exec_bootloader(SPINE3);
		lib4legs_spine_if_exec_bootloader(SPINE_FINGER);
		lib4legs_spine_if_exec_bootloader(SPINE_ARM);
	}

	return 0;
}

static const char *boot_spine_help(void)
{
	return "Boot Spines";
}

static ConsoleCommand boot_spine_cmd =
{
	.name = "boot_spine",
	.func = boot_spine_command,
	.help = boot_spine_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int base_pulse_command(const int argc, const char **argv)
{
	if (argc > 0) {
		uint32_t set = strtoul(argv[0], NULL, 10);
		lib4legs_enable_base_pulse(set);
	}
	else {
		lib4legs_printf("Select Enable/1 or Diable/0\n", index);
	}

	return 0;
}

static const char *base_pulse_help(void)
{
	return "Enable/Disable Base pulse output.";
}

static ConsoleCommand base_pulse_cmd =
{
	.name = "bp",
	.func = base_pulse_command,
	.help = base_pulse_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int dcdc_command(const int argc, const char **argv)
{
	uint32_t voltage = 5;
	uint32_t enable;
	if (argc >= 2) {
		voltage = strtoul(argv[1], NULL, 10);
	}
	
	if (argc >= 1) {
		enable = strtoul(argv[0], NULL, 10);
	}
	else {
		lib4legs_printf("Select Enable:1, Disable:0\n");
		return 0;
	}

	if (enable) {
		if (voltage == 6) {
			lib4legs_power_ctrl_6v0(1);
		}
		else {
			lib4legs_power_ctrl_6v0(0);
		}

		if (voltage == 7) {
			lib4legs_power_ctrl_7v4(1);
		}
		else {
			lib4legs_power_ctrl_7v4(0);
		}
		
		lib4legs_power_ctrl_dcdc(1);
	}
	else {
		lib4legs_power_ctrl_dcdc(0);
		lib4legs_power_ctrl_7v4(0);
		lib4legs_power_ctrl_6v0(0);
	}

	return 0;
}

static const char *dcdc_help(void)
{
	return "Enable/Disable DCDC";
}

static ConsoleCommand dcdc_cmd =
{
	.name = "dcdc",
	.func = dcdc_command,
	.help = dcdc_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int power_command(const int argc, const char **argv)
{
	uint32_t vcc_mV = lib4legs_power_get_vcc();
	uint32_t vbat_mV = lib4legs_power_get_vbat();
	int power_good = lib4legs_power_check_power_good();
	
	lib4legs_printf("VCC  Voltage = %u[mV]\n", vcc_mV);
	lib4legs_printf("VBAT Voltage = %u[mV]\n", vbat_mV);
	lib4legs_printf("DCDC PG = %d\n", power_good);	

	return 0;
}

static const char *power_help(void)
{
	return "Check Power Status";
}

static ConsoleCommand power_cmd =
{
	.name = "power",
	.func = power_command,
	.help = power_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern int lib4legs_power_on(int enable);
static int pon_command(const int argc, const char **argv)
{
	lib4legs_power_on(1);

	return 0;
}

static const char *pon_help(void)
{
	return "Main Power ON";
}

static ConsoleCommand pon_cmd =
{
	.name = "pon",
	.func = pon_command,
	.help = pon_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int poff_command(const int argc, const char **argv)
{
	lib4legs_power_on(0);

	return 0;
}

static const char *poff_help(void)
{
	return "Main Power OFF";
}

static ConsoleCommand poff_cmd =
{
	.name = "poff",
	.func = poff_command,
	.help = poff_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int finger_command(const int argc, const char **argv)
{
	if (argc >= 3) {
		uint16_t f1 = strtol(argv[0], NULL, 10);
		uint16_t f2 = strtol(argv[1], NULL, 10);
		uint16_t f3 = strtol(argv[2], NULL, 10);
		
		lib4legs_spine_if_set_limit(SPINE_FINGER, 0, 800, 2400);
		lib4legs_spine_if_set_limit(SPINE_FINGER, 1, 800, 2400);
		lib4legs_spine_if_set_limit(SPINE_FINGER, 2, 800, 2400);
		
		lib4legs_spine_if_set_servo_mode(SPINE_FINGER, SPINE_MODE_DIRECT, 0);
		lib4legs_spine_if_set_servo_pulse_width(SPINE_FINGER, f1, f2, f3);
		lib4legs_spine_if_update_servo(SPINE_FINGER);

		lib4legs_printf("Set Finger pulse width : %5u %5u %5u\n", f1, f2, f3);
	}
	else {
		uint16_t f1, f2, f3;
		lib4legs_spine_if_get_servo_pulse_width(SPINE_FINGER, &f1, &f2, &f3);
		lib4legs_printf("Get Finger pulse width : %5u %5u %5u\n", f1, f2, f3);
		return 0;
	}

	return 0;
}

static const char *finger_help(void)
{
	return "Finger control";
}

static ConsoleCommand finger_cmd =
{
	.name = "fin",
	.func = finger_command,
	.help = finger_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int slider_command(const int argc, const char **argv)
{
	if (argc > 0) {
		uint16_t f1 = 0;
		uint16_t f2 = strtol(argv[0], NULL, 10);
		uint16_t f3 = 0;
		
		lib4legs_spine_if_set_limit(SPINE_ARM, 1, 0, 4096);
		
		lib4legs_spine_if_set_servo_mode(SPINE_ARM, SPINE_MODE_DIRECT, 0);
		lib4legs_spine_if_set_servo_pulse_width(SPINE_ARM, f1, f2, f3);
		lib4legs_spine_if_update_servo(SPINE_ARM);

		lib4legs_printf("Set Slider Position : %5u\n", f2);
	}
	else {
		uint16_t f1, f2, f3;
		lib4legs_spine_if_get_servo_pulse_width(SPINE_ARM, &f1, &f2, &f3);
		lib4legs_printf("Get Slider target   : %5u\n", f2);

		SPINE_POTENTION pos;
		lib4legs_spine_if_get_potention(SPINE_ARM, &pos);
		lib4legs_printf("Get Slider position : %5u\n", pos.val[1]);
		return 0;
	}

	return 0;
}

static const char *slider_help(void)
{
	return "Slider control";
}

static ConsoleCommand slider_cmd =
{
	.name = "slider",
	.func = slider_command,
	.help = slider_help,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int arm_command(const int argc, const char **argv)
{
	char c=0;

	uint16_t f1 = 900;
	uint16_t f2 = 900;
	uint16_t f3 = 900;
	uint16_t s  = 1700;

	lib4legs_spine_if_set_limit(SPINE_ARM, 1, 1700, 3000);
	lib4legs_spine_if_set_limit(SPINE_FINGER, 0, 900, 2400);
	lib4legs_spine_if_set_limit(SPINE_FINGER, 1, 900, 2400);
	lib4legs_spine_if_set_limit(SPINE_FINGER, 2, 900, 2400);
		
	lib4legs_spine_if_set_servo_mode(SPINE_FINGER, SPINE_MODE_DIRECT, 0);
	lib4legs_spine_if_set_servo_mode(SPINE_ARM, SPINE_MODE_DIRECT, 0);

	lib4legs_spine_if_set_servo_pulse_width(SPINE_FINGER, f1, f2, f3);
	lib4legs_spine_if_set_servo_pulse_width(SPINE_ARM, 0, s, 0);
	lib4legs_spine_if_update_servo(SPINE_FINGER);
	lib4legs_spine_if_update_servo(SPINE_ARM);

	lib4legs_printf("Enter Arm control mode\n");
	do {
		int print = 1;
		lib4legs_rx((uint8_t *)&c, 1, 1);
		
		switch (c)
		{
		case 'q':
			f1 += 100;
			break;
		case 'a':
			f1 -= 100;
			break;
		case 'w':
			f2 += 100;
			break;
		case 's':
			f2 -= 100;
			break;
		case 'e':
			f3 += 100;
			break;
		case 'd':
			f3 -= 100;
			break;
		case 'u':
			s += 100;
			break;
		case 'j':
			s -= 100;
			break;
		default:
			print = 0;
		}

		if (f1 < 900)  f1 =  900;
		if (f1 > 2400) f1 = 2400;
		if (f2 < 900)  f2 =  900;
		if (f2 > 2400) f2 = 2400;
		if (f3 < 900)  f3 =  900;
		if (f3 > 2400) f3 = 2400;
		if (s  < 1700) s  = 1700;
		if (s  > 3000) s  = 3000;

		if (print) {
			lib4legs_printf("%4u %4u %4u %4u\r", f1,f2,f3,s);
		}

		lib4legs_spine_if_set_servo_pulse_width(SPINE_FINGER, f1, f2, f3);
		lib4legs_spine_if_set_servo_pulse_width(SPINE_ARM, 0, s, 0);
		lib4legs_spine_if_update_servo(SPINE_FINGER);
		lib4legs_spine_if_update_servo(SPINE_ARM);
	} while (c != 0x04);
	lib4legs_printf("\nExit Arm control mode\n");

	return 0;
}

static const char *arm_help(void)
{
	return "Slider control";
}

static ConsoleCommand arm_cmd =
{
	.name = "arm",
	.func = arm_command,
	.help = arm_help,
};


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void initialize_commands(void)
{
	consoleInstallCommand(&arm_cmd);
	consoleInstallCommand(&finger_cmd);
	consoleInstallCommand(&pon_cmd);
	consoleInstallCommand(&poff_cmd);
	consoleInstallCommand(&update_cmd);
	consoleInstallCommand(&boot_spine_cmd);
	consoleInstallCommand(&boot_cmd);
	consoleInstallCommand(&base_pulse_cmd);
	consoleInstallCommand(&dcdc_cmd);
	consoleInstallCommand(&power_cmd);
	consoleInstallCommand(&slider_cmd);
}

