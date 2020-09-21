/*
 * crc16.h
 *
 * Created: 25/03/2017 13:55:32
 *  Author: Tomasz Jaworski
 */ 


#ifndef CRC16_H_
#define CRC16_H_


uint16_t calc_checksum(const void *buf, uint8_t size);
uint16_t calc_checksum(const void *buf1, uint8_t size1, const void* buf2, uint8_t size2);




#endif /* CRC16_H_ */