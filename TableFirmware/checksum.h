/*
 * crc16.h
 *
 * Created: 25/03/2017 13:55:32
 *  Author: Tomasz Jaworski
 */ 


#ifndef CRC16_H_
#define CRC16_H_

typedef uint16_t checksum_t;

checksum_t calc_checksum(const void *buf, uint8_t size);
checksum_t calc_checksum(const void *buf1, uint8_t size1, const void* buf2, uint8_t size2);

static_assert(sizeof(checksum_t) == 2, "checksum_t has invalid size");



#endif /* CRC16_H_ */