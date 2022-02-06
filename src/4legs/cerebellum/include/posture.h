/*
 * posture.h
 *
 * Created: 2022/02/07 4:43:27
 *  Author: kiyot
 */ 


#ifndef POSTURE_H_
#define POSTURE_H_

#define LEG_IDX_REAR_RIGHT		(0)
#define LEG_IDX_REAR_LEFT		(1)
#define LEG_IDX_FRONT_RIGHT		(2)
#define LEG_IDX_FRONT_LEFT		(3)

typedef struct
{
	double x;
	double y;
	double z;
} Lib4legsCoordinate;

typedef struct 
{
	Lib4legsCoordinate legs[4];	
} Lib4legsPosture;

typedef struct
{
	int transition_count;
	Lib4legsPosture posture;
} Lib4legsMotion;

int cerebellum_posture_initialize(void);
int cerebellum_posture_home(void);
int cerebellum_posture_set(const Lib4legsPosture *pos);

#endif /* POSTURE_H_ */