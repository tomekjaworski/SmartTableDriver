/*
 * intensity_measurements.h
 *
 * Created: 3/27/2017 17:32:11
 *  Author: Izabela Perenc
 */ 


#ifndef INTENSITY_MEASUREMENTS_H_
#define INTENSITY_MEASUREMENTS_H_

struct BURST {
	uint16_t time_point;
	volatile bool enabled;
	volatile uint16_t timer;
};

extern struct BURST burst;

void im_initialize(void);
void im_execute_sync(void);
void im_execute_async(void);

void im_full_resolution_synchronized(void);


#endif /* INTENSITY_MEASUREMENTS_H_ */