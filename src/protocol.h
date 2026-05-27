#ifndef ITB_PROTOCOL_H
#define ITB_PROTOCOL_H

#include <stdint.h> 
#include <stddef.h>

#define FRAME_SYNC_BYTE 0X02 // STX 

/** 
 * Enforce 1-byte allignment.
 * GCC/CLANG - Manjaro 
 * eliminate padding bytes, lock structure size 10 bytes. 
 */
  #pragma pack(push, 1) 

  typedef struct {
  	uint8_t sync_byte; 	//Offset 0: must be 0x02 	
	uint32_t motor_rpm;	// Offset 1: 32-bit speed metric 
	uint16_t voltage; 	// Offset 5: 16-bit electrical potential 
	int16_t temperature; 	// Offset 7: 16-bit signed operational temperature 
	uint8_t crc8; 		//Offset 9: polynomial integrity check byte 
} TelemetryPacket; 

#pragma pack(pop) 

/** 
 * computes the CRC-8 maxim checksum over a data buffer.
 * Polynomial: x^8 + x^5 + x^4 + 1 (0x31) common 1 wire system
 */
static inline uint8_t compute_crc8(const uint8_t *data, size_t length) {
	uint8_t crc = 0x00;

	for (size_t i = 0; i < length; i++) {
		crc ^= data[i];
		for (int bit = 0; bit < 8; bit++) {
			if (crc & 0x80) {
				crc = (crc <<1) ^ 0x31;
			}else{
			 crc <<= 1; 
			}
		}
	}
	return crc;
}

#endif //ITB_PROTOCOL_H
		
