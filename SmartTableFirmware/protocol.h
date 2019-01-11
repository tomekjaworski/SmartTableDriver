/*
 * protocol.h
 *
 * Created: 22.04.2017 15:08:48
 *  Author: Tomek Jaworski
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#define RX_PAYLOAD_CAPACITY	32
#define TX_PAYLOAD_CAPACITY	128

#define ADDRESS_BROADCAST	(device_address_t)0xFF
#define ADDRESS_NONE		(device_address_t)0x00


typedef unsigned char device_address_t;



enum class ADCBlockType : uint8_t {
	Invalid = 0,	// none
	_1Bits = 1,		// 8 measurements on 1 byte
	_2Bits = 2,		// 4 measurements on 1 byte
	_3Bits = 3,		// 8 measurements on 3 bytes
	_4Bits = 4,		// 2 measurements on 1 byte
	_5Bits = 5,		// 8 measurements on 5 bytes
	_6Bits = 6,		// 4 measurements on 3 bytes
	_7Bits = 7,		// 8 measurements on 7 bytes
	_8Bits = 8,		// 1 measurement on 1 byte - no additional type is needed
	_9Bits = 9,		// 8 measurements on 9 bytes
	_10Bits = 10,	// 4 measurements on 5 bytes
};


struct ADC_BLOCK10_N4_B5 // 4 measurements on 5 bytes
{
	uint16_t v1 : 10;
	uint16_t v2 : 10;
	uint16_t v3 : 10;
	uint16_t v4 : 10;
} __attribute__((packed));

struct ADC_BLOCK9_N8_B9 // 8 measurements on 9 bytes
{
	uint16_t v1 : 9;
	uint16_t v2 : 9;
	uint16_t v3 : 9;
	uint16_t v4 : 9;
	uint16_t v5 : 9;
	uint16_t v6 : 9;
	uint16_t v7 : 9;
	uint16_t v8 : 9;
} __attribute__((packed));

// 1 measurement on 1 byte - no additional type is needed

struct ADC_BLOCK7_N8_B7 // 8 measurements on 7 bytes
{
	uint16_t v1 : 7;
	uint16_t v2 : 7;
	uint16_t v3 : 7;
	uint16_t v4 : 7;
	uint16_t v5 : 7;
	uint16_t v6 : 7;
	uint16_t v7 : 7;
	uint16_t v8 : 7;
} __attribute__((packed));

struct ADC_BLOCK6_N4_B3 // 4 measurements on 3 bytes
{
	uint16_t v1 : 6;
	uint16_t v2 : 6;
	uint16_t v3 : 6;
	uint16_t v4 : 6;
} __attribute__((packed));

struct ADC_BLOCK5_N8_B5 // 8 measurements on 5 bytes
{
	uint16_t v1 : 5;
	uint16_t v2 : 5;
	uint16_t v3 : 5;
	uint16_t v4 : 5;
	uint16_t v5 : 5;
	uint16_t v6 : 5;
	uint16_t v7 : 5;
	uint16_t v8 : 5;
} __attribute__((packed));

struct ADC_BLOCK4_N2_B1 // 2 measurements on 1 byte
{
	uint16_t v1 : 4;
	uint16_t v2 : 4;
} __attribute__((packed));

struct ADC_BLOCK3_N8_B3 // 8 measurements on 3 bytes
{
	uint16_t v1 : 3;
	uint16_t v2 : 3;
	uint16_t v3 : 3;
	uint16_t v4 : 3;
	uint16_t v5 : 3;
	uint16_t v6 : 3;
	uint16_t v7 : 3;
	uint16_t v8 : 3;
} __attribute__((packed));

struct ADC_BLOCK2_N4_B1 // 4 measurements on 1 byte
{
	uint16_t v1 : 2;
	uint16_t v2 : 2;
	uint16_t v3 : 2;
	uint16_t v4 : 2;
} __attribute__((packed));

struct ADC_BLOCK1_N8_B1 // 8 measurements on 1 byte
{
	uint16_t v1 : 1;
	uint16_t v2 : 1;
	uint16_t v3 : 1;
	uint16_t v4 : 1;
	uint16_t v5 : 1;
	uint16_t v6 : 1;
	uint16_t v7 : 1;
	uint16_t v8 : 1;
} __attribute__((packed));

static_assert(sizeof(ADC_BLOCK10_N4_B5) == 5, "ADC_BLOCK10_N4_B5 has invalid_size");
static_assert(sizeof(ADC_BLOCK9_N8_B9) == 9, "ADC_BLOCK9_N8_B9 has invalid_size");
static_assert(sizeof(ADC_BLOCK7_N8_B7) == 7, "ADC_BLOCK7_N8_B7 has invalid_size");
static_assert(sizeof(ADC_BLOCK6_N4_B3) == 3, "ADC_BLOCK6_N4_B3 has invalid_size");
static_assert(sizeof(ADC_BLOCK5_N8_B5) == 5, "ADC_BLOCK5_N8_B5 has invalid_size");
static_assert(sizeof(ADC_BLOCK4_N2_B1) == 1, "ADC_BLOCK4_N2_B1 has invalid_size");
static_assert(sizeof(ADC_BLOCK3_N8_B3) == 3, "ADC_BLOCK3_N8_B3 has invalid_size");
static_assert(sizeof(ADC_BLOCK2_N4_B1) == 1, "ADC_BLOCK2_N4_B1 has invalid_size");
static_assert(sizeof(ADC_BLOCK1_N8_B1) == 1, "ADC_BLOCK1_N8_B1 has invalid_size");

//
//
//  ###############################################################################################
//
//


enum class MessageType : uint8_t
{
	__BroadcastFlag = 0x80,
	__ResponseFlag = 0x40,
	Invalid = 0x00,
	None = 0x01,

	// Classical ping-pong messages to see if a device is alive and kickin'
	PingRequest = 0x02,
	PingResponse = 0x02 | __ResponseFlag,
	
	// Get version of the active firmware
	GetVersionRequest = 0x03,
	GetVersionResponse = 0x03 | __ResponseFlag,
	
	// Do the full resolution measurement, wait for it and then send the results (array of 100 uint16_ts)
	GetFullResolutionSyncMeasurementRequest = 0x04,
	GetFullResolutionSyncMeasurementResponse = 0x04 | __ResponseFlag,
	
	// Set timing configuration for the burst mode
	SetBurstConfigurationRequest = 0x05,
	SetBurstConfigurationResponse = 0x05 | __ResponseFlag,
	
	// Execute a burst measurement
	DoBurstMeasurementRequest = 0x06,
	DoBurstMeasurementResponse = 0x06 | __ResponseFlag,
	
	// Get statistics of previously executed burst measurement
	GetBurstMeasurementStatisticsRequest = 0x07,
	GetBurstMeasurementStatisticsResponse = 0x07 | __ResponseFlag,
	
	__RequestMinCode = PingRequest,
	__RequestMaxCode = GetBurstMeasurementStatisticsRequest,
	
	//__MAX,
	//__MIN = Ping,
};


struct BURST_CONFIGURATION {
	uint16_t time_point;
	uint16_t silence_interval;
	uint8_t bits_per_point;
	
} __attribute__((packed));


struct BURST_STATISTICS {
	uint16_t last_measure_time;
	uint16_t last_transmission_time;
	uint16_t count;
} __attribute__((packed));



struct PROTO_HEADER {
	device_address_t address;		// receiver address (if given) or ADDRESS_BROADCAST
	MessageType type;				// type of the received message
	uint8_t payload_length;			//
} __attribute__((packed));

// data size asserts
static_assert(sizeof(enum MessageType) == 1, "MessageType has invalid size");
static_assert(sizeof(PROTO_HEADER) == 3, "PROTO_HEADER has invalid size");
static_assert(sizeof(BURST_CONFIGURATION) == 5, "BURST_CONFIGURATION has invalid size");
static_assert(sizeof(BURST_STATISTICS) == 6, "BURST_STATISTICS has invalid size");



#endif /* PROTOCOL_H_ */