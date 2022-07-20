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

#include "spine_controller.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static const char *update_help(void)
{
	return "Update Main Fw";
}

static int update_command(const int argc, const char **argv)
{
	if (argc == 0) {
		lib4legs_printf("Indicate FW size[byte]\n");
		return LIB4LEGS_ERROR_INVAL;
	}

	uint32_t fwsize = strtoul(argv[0], NULL, 10);
	if (fwsize == 0 || fwsize > 0x000E2000) {
		lib4legs_printf("Invalid FW size %u[byte]\n", fwsize);
		return LIB4LEGS_ERROR_NOBUF;
	}

	lib4legs_update_request_and_reboot(fwsize);

	return 0;
}

static ConsoleCommand update_cmd =
{
	.name = "update",
	.func = update_command,
	.help = update_help,
};

/*---------------------------------------------------------------------------*/
static const char *update_spine_help(void)
{
	return "Update Spine Fw";
}

static int update_spine_command(const int argc, const char **argv)
{
	int ret = 0;
	for (int i=0 ; i<4 ; ++i) {
		lib4legs_spine_if_exec_restart(i);

		ret = cerebellum_spine_version_check_and_update(i, 1);
		lib4legs_printf("Update spine firmware. %d\n", ret);
	}

	return 0;
}

static ConsoleCommand update_spine_cmd =
{
	.name = "update_spine",
	.func = update_spine_command,
	.help = update_spine_help,
};

/*---------------------------------------------------------------------------*/
static const char *read_pos_help(void)
{
	return "Read potential meter from SPINEs";
}

static int read_pos_command(const int argc, const char **argv)
{
	SPINE_POTENTION pos;
	for (int i=0 ; i<4 ; ++i) {
		lib4legs_spine_if_get_potention(i, &pos);
		lib4legs_printf("[%d] %4d, %4d, %4d\n", i, pos.val[0], pos.val[1], pos.val[2]);
	}

	return 0;
}

static ConsoleCommand read_pos_cmd =
{
	.name = "pos",
	.func = read_pos_command,
	.help = read_pos_help,
};

/*---------------------------------------------------------------------------*/
static const char *diag_help(void)
{
	return "diag";
}

extern uint8_t _binary_spine_bin_start;
extern uint8_t _binary_spine_bin_end;
extern uint8_t _binary_spine_bin_size;

static int diag_command(const int argc, const char **argv)
{
	lib4legs_printf("start 0x%08x\n", &_binary_spine_bin_start);
	lib4legs_printf("end   0x%08x\n", &_binary_spine_bin_end);
	lib4legs_printf("size  0x%08x\n", &_binary_spine_bin_size);

	uint8_t type = 0;
	uint32_t version = 0;
	for (int i=0 ; i<4 ; ++i) {
		lib4legs_spine_if_get_type(i, &type);
		lib4legs_spine_if_get_fw_version(i, &version);
		
		lib4legs_printf("SPINE[%d]. Mode = %s, Version = 0x%08x\n", i, (type == SPINE_MMAP_TYPE_BOOTLOADER ? "Bootloader" : "Application"), version);
	}

	return 0;
}

static ConsoleCommand diag_cmd =
{
	.name = "diag",
	.func = diag_command,
	.help = diag_help,
};


/*---------------------------------------------------------------------------*/
static const char *bootloader_help(void)
{
	return "Jump to bootloader";
}

static int bootloader_command(const int argc, const char **argv)
{
	lib4legs_spine_if_exec_bootloader(SPINE0);
	lib4legs_spine_if_exec_bootloader(SPINE1);
	lib4legs_spine_if_exec_bootloader(SPINE2);
	lib4legs_spine_if_exec_bootloader(SPINE3);
	
	lib4legs_set_force_bootloader();
	lib4legs_jump_to_bootloader();

	return 0;
}

static ConsoleCommand bootloader_cmd =
{
	.name = "bootloader",
	.func = bootloader_command,
	.help = bootloader_help,
};


/*---------------------------------------------------------------------------*/
static const char *base_pulse_help(void)
{
	return "Enable/Disable Base pulse output.";
}

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

static ConsoleCommand base_pulse_cmd =
{
	.name = "bp",
	.func = base_pulse_command,
	.help = base_pulse_help,
};


/*---------------------------------------------------------------------------*/
static const char *legcfg_help(void)
{
	return "servo <leg_id> <servo0 pulse[us]> <servo1 pulse[us]> <servo2 pulse[us]>\n";
}

static int legcfg_command(const int argc, const char **argv)
{
	if (argc != 4) {
		lib4legs_printf("%s\n", legcfg_help());
		return -1;
	}
	uint8_t leg_id = strtoul(argv[0], NULL, 10);
	uint16_t s0_us = strtoul(argv[1], NULL, 10);
	uint16_t s1_us = strtoul(argv[2], NULL, 10);
	uint16_t s2_us = strtoul(argv[3], NULL, 10);

	if (leg_id > 3) {
		lib4legs_printf("Invalid LegID.\n");
		return -1;
	}
	
	lib4legs_spine_if_set_servo_pulse_width(leg_id, s0_us, s1_us, s2_us);
	lib4legs_spine_if_update_servo(leg_id);
	
	return 0;
}


static ConsoleCommand legcfg_cmd =
{
	.name = "legcfg",
	.func = legcfg_command,
	.help = legcfg_help,
};

/*---------------------------------------------------------------------------*/
static const char *legcfga_help(void)
{
	return "legcfga <leg_id> <angle0 [0.01deg]> <angle1 [0.01deg]> <angle2 [0.01deg]>\n";
}

static int legcfga_command(const int argc, const char **argv)
{
	if (argc != 4) {
		lib4legs_printf("%s\n", legcfga_help());
		return -1;
	}
	uint8_t leg_id = strtol(argv[0], NULL, 10);
	int16_t a0 = strtol(argv[1], NULL, 10);
	int16_t a1 = strtol(argv[2], NULL, 10);
	int16_t a2 = strtol(argv[3], NULL, 10);

	if (leg_id > 3) {
		lib4legs_printf("Invalid LegID.\n");
		return -1;
	}

	lib4legs_spine_if_set_servo_mode(leg_id, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_pulse_width(leg_id, (uint16_t)a0, (uint16_t)a1, (uint16_t)a2);
	lib4legs_spine_if_update_servo(leg_id);
	
	return 0;
}


static ConsoleCommand legcfga_cmd =
{
	.name = "legcfga",
	.func = legcfga_command,
	.help = legcfga_help,
};


/*---------------------------------------------------------------------------*/
static const char *legstate_help(void)
{
	return "Check Leg status\n";
}

static int legstate_command(const int argc, const char **argv)
{
	if (argc != 1) {
		lib4legs_printf("Invalid args\n");
		return -1;
	}
	uint8_t leg_id = strtoul(argv[0], NULL, 10);

	if (leg_id > 3) {
		lib4legs_printf("Invalid LegID.\n");
		return -1;
	}

	uint16_t s0_us, s1_us, s2_us;
	lib4legs_spine_if_get_servo_pulse_width(leg_id, &s0_us, &s1_us, &s2_us);

	lib4legs_printf("LegID %d | %5d %5d %5d\n", leg_id, s0_us, s1_us, s2_us);
	
	return 0;
}


static ConsoleCommand legstate_cmd =
{
	.name = "legstate",
	.func = legstate_command,
	.help = legstate_help,
};

/*---------------------------------------------------------------------------*/
static const char *legsctrl_help(void)
{
	return "Check Leg status\n";
}

#define POW(v)				((v)*(v))
#define L2					18.0f
#define L1					22.0f

static int _position_to_angle(double x, double y, double z, int *a0, int *a1, int *a2)
{
	double d = sqrt(POW(x) + POW(y) + POW(z));
	double theta1 = M_PI - acos((POW(L1) + POW(L2) - POW(d)) / (2.0f * L1 * L2));
	double x0 = L1 * cos(theta1) + L2;
	double y0 = L1 * sin(theta1);

	double y1 = sqrt(POW(d)-POW(x));
	double theta2 = asin((x0*y1 - y0*x) / (POW(x0) + POW(y0)));
	double theta3 = y1 == 0 ? 0 : acos((y/y1));
	if (z < 0) theta3 = -theta3;

	*a0 = (theta3 * 180.0f/M_PI) * 100;
	*a1 = (theta2 * 180.0f/M_PI) * 100;
	*a2 = (theta1 * 180.0f/M_PI) * 100;

	return 0;
}

static int legsctrl_command(const int argc, const char **argv)
{
	if (argc != 4) {
		lib4legs_printf("Invalid args\n");
		return -1;
	}
	uint8_t leg_id = strtoul(argv[0], NULL, 10);
	double  x = strtod(argv[1], NULL);
	double  y = strtod(argv[2], NULL);
	double  z = strtod(argv[3], NULL);

	lib4legs_printf ("Move to (%lf, %lf, %lf)\n", x, y, z);
	int a0, a1, a2;
	_position_to_angle(x,y,z, &a0, &a1, &a2);

	lib4legs_spine_if_set_servo_mode(leg_id, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_pulse_width(leg_id, (uint16_t)a0, (uint16_t)a1, (uint16_t)a2);
	lib4legs_spine_if_update_servo(leg_id);

	return 0;
}


static ConsoleCommand legsctrl_cmd =
{
	.name = "leg",
	.func = legsctrl_command,
	.help = legsctrl_help,
};


/*---------------------------------------------------------------------------*/
static const char *move_help(void)
{
	return "Initialize leg modules\n";
}

typedef struct 
{
	double x;
	double y;
	double z;	
} Lib4legsCoordinate;

typedef struct
{
	int transition_count;
	Lib4legsCoordinate legs[4];
} Lib4legsMotion;


static const Lib4legsMotion sMove0[6] = 
{
	{6, {{0, 35, 15}, {0, 30,-15}, {0, 30,-15}, {0, 35, 15}}},
	{6, {{0, 35,-15}, {0, 30,-15}, {0, 30,-15}, {0, 35,-15}}},
	{6, {{0, 35,-15}, {0, 30, 15}, {0, 30, 15}, {0, 35,-15}}},
	{6, {{0, 30,-15}, {0, 35, 15}, {0, 35, 15}, {0, 30,-15}}},
	{6, {{0, 30,-15}, {0, 35,-15}, {0, 35,-15}, {0, 30,-15}}},
	{6, {{0, 30, 15}, {0, 35,-15}, {0, 35,-15}, {0, 30, 15}}}
};

static const Lib4legsMotion sMove1[12] = 
{
	{1, {{0, 33,  0}, {0, 33, 17.5}, {0, 33,  0}, {0, 28,-17.5}}},
	{5, {{0, 33,  0}, {0, 33, 17.5}, {0, 33,  0}, {0, 28, 17.5}}},
	{1, {{0, 33,-17.5}, {0, 33,  0}, {0, 33,-17.5}, {0, 33,  0}}},
	{1, {{0, 28,-17.5}, {0, 33,  0}, {0, 33,-17.5}, {0, 33,  0}}},
	{5, {{0, 28, 17.5}, {0, 33,  0}, {0, 33,-17.5}, {0, 33,  0}}},
	{1, {{0, 33, 17.5}, {0, 33,  0}, {0, 33,-17.5}, {0, 33,  0}}},
	{1, {{0, 33, 17.5}, {0, 33,  0}, {0, 28,-17.5}, {0, 33,  0}}},
	{5, {{0, 33, 17.5}, {0, 33,  0}, {0, 28, 17.5}, {0, 33,  0}}},
	{1, {{0, 33,  0}, {0, 33,-17.5}, {0, 33,  0}, {0, 33,-17.5}}},
	{1, {{0, 33,  0}, {0, 28,-17.5}, {0, 33,  0}, {0, 33,-17.5}}},
	{5, {{0, 33,  0}, {0, 28, 17.5}, {0, 33,  0}, {0, 33,-17.5}}},
	{1, {{0, 33,  0}, {0, 33, 17.5}, {0, 33,  0}, {0, 33,-17.5}}}
};

static const Lib4legsMotion sMove2[2] =
{
	{5, {{0, 35, 0}, {0, 35, 0}, {0, 28, 0}, {0, 28, 0}}},
	{5, {{0, 28, 0}, {0, 28, 0}, {0, 35, 0}, {0, 35, 0}}},
};

typedef struct {
	int seq_num;
	int seq;
	
	int current_count;
	
	Lib4legsMotion begin;
	Lib4legsMotion target;
	
	const Lib4legsMotion *pattern;
} LIB4LEGS_MOTION;

static LIB4LEGS_MOTION sMotionCtx;

static void _install_motion(int seq_num, const Lib4legsMotion *pattern)
{
	sMotionCtx.pattern = pattern;

	sMotionCtx.current_count = 0;

	sMotionCtx.seq_num = seq_num;	
	sMotionCtx.seq = 0;
	memcpy(&sMotionCtx.begin,  &pattern[0], sizeof(Lib4legsMotion));
	memcpy(&sMotionCtx.target, &pattern[0], sizeof(Lib4legsMotion));

	return;
}

static volatile int sMotionUpdateReq = 0;

static void _update_motion(void)
{
	sMotionCtx.current_count++;
	double rate = ((double)sMotionCtx.current_count / (double)sMotionCtx.target.transition_count);

	lib4legs_led_toggle(LIB4LEGS_LED0);

	for (int i=0 ; i<4 ; ++i) {
		double x = rate * (sMotionCtx.target.legs[i].x - sMotionCtx.begin.legs[i].x) + sMotionCtx.begin.legs[i].x;
		double y = rate * (sMotionCtx.target.legs[i].y - sMotionCtx.begin.legs[i].y) + sMotionCtx.begin.legs[i].y;
		double z = rate * (sMotionCtx.target.legs[i].z - sMotionCtx.begin.legs[i].z) + sMotionCtx.begin.legs[i].z;

		int a0,a1,a2;
		_position_to_angle(x,y,z, &a0, &a1, &a2);
		lib4legs_spine_if_set_servo_pulse_width(i,a0,a1,a2);
	}

	lib4legs_led_set(LIB4LEGS_LED1);

	if (sMotionCtx.current_count == sMotionCtx.target.transition_count) {
		//Next
		sMotionCtx.seq = (sMotionCtx.seq + 1) % sMotionCtx.seq_num;
		memcpy(&sMotionCtx.begin,  &sMotionCtx.target, sizeof(Lib4legsMotion));
		memcpy(&sMotionCtx.target, &sMotionCtx.pattern[sMotionCtx.seq], sizeof(Lib4legsMotion));
		
		sMotionCtx.current_count = 0;
	}
}

static void _update_motion_req(void)
{
	sMotionUpdateReq = 1;
}

static int move_command(const int argc, const char **argv)
{
	uint32_t pattern = 0;
	if (argc > 0) {
		pattern = strtoul(argv[0], NULL, 10);
	}
	
	lib4legs_spine_if_set_servo_mode(0, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(1, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(2, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(3, SPINE_MODE_ANGLE, 0);

	if (pattern == 1) {
		_install_motion(12, sMove1);
	}
	else if (pattern == 2) {
		_install_motion(6, sMove2);
	}
	else {
		_install_motion(6, sMove0);
	}

	lib4legs_register_base_pulse_cb(_update_motion_req);
	while (1) {
		char c = 0;
		int ret = lib4legs_getc(&c, 0);
		if (ret == 0 && c == (0x3)) break;

		if (sMotionUpdateReq != 0) {
			sMotionUpdateReq = 0;
			_update_motion();
		}
		
	}
	lib4legs_register_base_pulse_cb(NULL);

	return 0;
}


static ConsoleCommand move_cmd =
{
	.name = "move",
	.func = move_command,
	.help = move_help,
};


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void initialize_commands(void)
{
	consoleInstallCommand(&read_pos_cmd);
	consoleInstallCommand(&base_pulse_cmd);
	consoleInstallCommand(&legcfg_cmd);
	consoleInstallCommand(&legcfga_cmd);
	consoleInstallCommand(&legstate_cmd);
	consoleInstallCommand(&legsctrl_cmd);
	consoleInstallCommand(&move_cmd);
	consoleInstallCommand(&bootloader_cmd);
	consoleInstallCommand(&diag_cmd);
	consoleInstallCommand(&update_spine_cmd);
	consoleInstallCommand(&update_cmd);
}

