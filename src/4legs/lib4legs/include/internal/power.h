/*
 * power.h
 *
 * Created: 2022/04/06 5:08:13
 *  Author: kiyot
 */ 


#ifndef POWER_H_
#define POWER_H_

int lib4legs_power_on(int enable);
int lib4legs_power_check_power_button(void);
void lib4legs_power_update(void);

#endif /* POWER_H_ */