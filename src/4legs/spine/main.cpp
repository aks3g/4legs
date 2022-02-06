/*
 * spine.cpp
 *
 * Created: 2021/12/30 6:46:15
 * Author : kiyot
 */ 

#include <libxmegaE5_error.h>
#include <libxmegaE5_utils.h>
#include <libxmegaE5_gpio.h>
#include <libxmegaE5_timer.h>

#include <libspine.h>

static int16_t sAngle[3] = {0,0,0};
static void _servo_update_cb(void)
{
//	libspine_load_servo_param_from_shared_memory_all();

	uint8_t mode = libspine_get_servo_mode();
	if ((mode & 0x7F) == SPINE_MMAP_ADDR_SERVO_MODE_DIRECT) {
		libspine_load_servo_param_from_shared_memory_all();
	}
	else if ((mode & 0x7F) == SPINE_MMAP_ADDR_SERVO_MODE_ANGLE) {
		libspine_get_target_angles(&sAngle[0], &sAngle[1], &sAngle[2]);
		libspine_load_servo_param_by_degree(sAngle[0], sAngle[1], sAngle[2]);
	}
}

int main(void)
{
	libspine_initialize(SPINE_MMAP_TYPE_APPLICATION);
	libspine_register_servo_update_cb(_servo_update_cb);

    /* Replace with your application code */
    while (1) {
		libspine_poll();
    }
}

