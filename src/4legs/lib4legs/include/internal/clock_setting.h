/*
 * clock_setting.h
 *
 * Created: 2021/12/31 6:07:57
 *  Author: kiyot
 */ 


#ifndef CLOCK_SETTING_H_
#define CLOCK_SETTING_H_

#define CLK_GEN_NUMBER_MAIN			(0)
#define CLK_GEN_NUMBER_1MHZ			(1)
#define CLK_GEN_NUMBER_48MHZ		(2)
#define CLK_GEN_NUMBER_32kHZ		(3)

void initialize_clock_tree(void);


#endif /* CLOCK_SETTING_H_ */