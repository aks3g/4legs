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

static XmodemContext sXmodemCtx;
static uint8_t sSpineFwBin[8 * 1024];

static uint8_t _spine_fw_receive_cb(uint8_t status, const XmodemContext *ctx)
{
	memcpy(&(sSpineFwBin[ctx->received_size]), ctx->buf.fd.data, ctx->received_block_size);	
	return 0;
}

static int update_command(const int argc, const char **argv)
{
	// check argument
	if (argc == 0) {
		lib4legs_printf("Need firmware size[byte]\n");
		return LIB4LEGS_ERROR_INVAL;
	}
	
	uint32_t fwsize = strtoul(argv[0], NULL, 10);
	if (fwsize == 0){
		return LIB4LEGS_ERROR_NOBUF;
	}


	int spine_only = 0;
	if (argc == 2) {
		if (0 == strncmp(argv[1], "-s", 2)) {
			spine_only = 1;
		}
		else {
			lib4legs_printf("Unknown option %s\n", argv[1]);
			return LIB4LEGS_ERROR_INVAL;
		}
	}

	lib4legs_printf("Update %s Firmware. size = %u[byte]\n", spine_only ? "spine" : "all", fwsize);
	if (spine_only) {
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

		xmodem_receive(&sXmodemCtx, fwsize, _spine_fw_receive_cb);

		// Update via Spine If
		lib4legs_spine_if_update(SPINE0, sSpineFwBin, fwsize);
		lib4legs_spine_if_update(SPINE1, sSpineFwBin, fwsize);
		lib4legs_spine_if_update(SPINE2, sSpineFwBin, fwsize);
		lib4legs_spine_if_update(SPINE3, sSpineFwBin, fwsize);

		lib4legs_printf("Fw Received\n");
	}
	else {
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
	}
	else if (0 == strncmp("-b", argv[0], 2)){
		lib4legs_printf("Spines goto Bootloader mode\n");
		lib4legs_spine_if_exec_bootloader(SPINE0);
		lib4legs_spine_if_exec_bootloader(SPINE1);
		lib4legs_spine_if_exec_bootloader(SPINE2);
		lib4legs_spine_if_exec_bootloader(SPINE3);
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
void initialize_commands(void)
{
	consoleInstallCommand(&update_cmd);
	consoleInstallCommand(&boot_spine_cmd);
	consoleInstallCommand(&boot_cmd);
	consoleInstallCommand(&base_pulse_cmd);
}

