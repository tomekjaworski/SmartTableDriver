/*
 * intensity_measurements.h
 *
 * Created: 3/27/2017 17:32:11
 *  Author: Izabela Perenc
 */ 


#ifndef INTENSITY_MEASUREMENTS_H_
#define INTENSITY_MEASUREMENTS_H_

#include "protocol.h"

union measurement_buffer_t {
	uint8_t raw8[7 * 15];
	uint16_t raw16[7 * 15];
};

struct im_raw_measurement_t {
	measurement_buffer_t primary;
//	measurement_buffer_t secondary;
};


extern struct im_raw_measurement_t im_data;

void im_initialize8(void);
void im_initialize10(void);

void im_measure8(void);
void im_measure10(void);


static_assert(sizeof(union measurement_buffer_t) == 7 * 15 * sizeof(uint16_t), "sizeof(union im_raw_measurement_t)");


#endif /* INTENSITY_MEASUREMENTS_H_ */