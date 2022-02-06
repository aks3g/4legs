/*
 * posture.c
 *
 * Created: 2022/02/07 4:48:03
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

#include "spine_controller.h"
#include "posture.h"


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define POW(v)				((v)*(v))
#define L2					18.0f
#define L1					22.0f

/*---------------------------------------------------------------------------*/
static const Lib4legsPosture p_home = {{{0, 35,  0}, {0, 35,  0}, {0, 35,  0}, {0, 35,  0}}};

/*---------------------------------------------------------------------------*/
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

static void _update_motion_req(void)
{
	;
}

int cerebellum_posture_initialize(void)
{
	lib4legs_spine_if_set_servo_mode(0, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(1, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(2, SPINE_MODE_ANGLE, 0);
	lib4legs_spine_if_set_servo_mode(3, SPINE_MODE_ANGLE, 0);

	lib4legs_register_base_pulse_cb(_update_motion_req);

	return LIB4LEGS_ERROR_OK;
}


int cerebellum_posture_home(void)
{
	return cerebellum_posture_set(&p_home);
}

int cerebellum_posture_set(const Lib4legsPosture *pos)
{
	for (int i=0 ; i<4 ; ++i) {
		int a0,a1,a2;
		_position_to_angle(pos->legs[i].x, pos->legs[i].y, pos->legs[i].z, &a0, &a1, &a2);
		lib4legs_spine_if_set_servo_pulse_width(i,a0,a1,a2);
	}

	return LIB4LEGS_ERROR_OK;
}


