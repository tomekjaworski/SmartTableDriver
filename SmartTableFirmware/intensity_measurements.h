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
	volatile uint8_t timer;

	volatile BURST_CONFIGURATION config;
	volatile BURST_STATISTICS stats;
};

union im_raw_measurement_t {
	uint8_t raw8[7 * 15];
	uint16_t raw16[7 * 15];
};


extern struct BURST burst;
extern union im_raw_measurement_t im_data;

void im_initialize8(void);
void im_initialize10(void);
void im_execute_sync(void);

void im_measure8(void);
void im_measure10(void);


static_assert(sizeof(union im_raw_measurement_t) == 7 * 15 * sizeof(uint16_t), "sizeof(union im_raw_measurement_t)");


#endif /* INTENSITY_MEASUREMENTS_H_ */