/*
 * intensity_measurements.h
 *
 * Created: 3/27/2017 17:32:11
 *  Author: Izabela Perenc
 */ 


#ifndef INTENSITY_MEASUREMENTS_H_
#define INTENSITY_MEASUREMENTS_H_

#include "protocol.h"

struct BURST {
	volatile bool enabled;
	volatile uint16_t timer;

	volatile BURST_CONFIGURATION config;
	volatile BURST_STATISTICS stats;
};

extern struct BURST burst;

void im_initialize(void);
void im_execute_sync(void);
void im_execute_async(void);

void im_full_resolution_synchronized(void);


#endif /* INTENSITY_MEASUREMENTS_H_ */