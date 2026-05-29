#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h>
#include "protocol.h" 

#define TARGET_PORT "/tmp/ttyV0"
#define DELAY_MS 500000 // 500 milliseconds 

int main(void) {
	printf("[*] Initializing Simulated Industrial Machinery...\n"); 

	//1. open the virtual wire for writing only
	int serial_fd = open(TARGET_PORT, O_WRONLY);
	if (serial_fd < 0) {
		perror("[-] Error: Failed to open virtual serial port. is socat running?"); 
		return EXIT_FAILURE;

		}
		printf("[+] bus locked. Target: %s\n", TARGET_PORT); 

		//2. Initialize tracking metrics (Simulation State)
		uint32_t current_rpm = 1200;
	        uint16_t current_voltage = 230; 
		int16_t current_temp = 22; // Start at room temp (signed) 
					   
		printf("[*] Emitting telemetry packets. Press Ctrl+C to stop. \n\n"); 

		while (1) {
		// Create an instance of our struct directly on the stack 
		TelemetryPacket packet; 

		// 3. Populate fields matching our strick memory offsets
		packet.sync_byte = FRAME_SYNC_BYTE; 
		packet.motor_rpm = current_rpm;
		packet.voltage = current_voltage;
		packet.temperature = current_temp; 

		// 4. compute CRC-8 over the first 9 bytes of the struc
		// Cast the struct pointer to a raw uint9_T pointer so the funtion
		// can walk through it byte-by-byte like a flat array. 
		packet.crc8 = compute_crc8((const uint8_t *) &packet, 9); 

		// 5. Blast the precise 10-byte block down the file descriptor 
		ssize_t bytes_written = write(serial_fd, &packet, sizeof(TelemetryPacket)); 

		if (bytes_written < 0) { 
			perror("[-] Write failure on serial wire"); 
		}

		// Print for local observation
		printf("[TX] Bytes: %2ld | Rpm: %4u | Volts: %3uV | Temp: %3d°C | CRC: 0x%02X\n", 
			(long)bytes_written, packet.motor_rpm, packet.voltage, packet.temperature, packet.crc8); 

		// 6. Simulate changing physical dynamics over T 
		current_rpm += 15; 
	       if (current_rpm > 3500) { 
	       		current_rpm = 1200; // Reset engine load loop 
		}

		// Mirror dynamic fluctuations 
		current_temp = (current_rpm > 2800) ? (current_temp + 1) : current_temp; 
		if (current_temp >85) current_temp = 22; // Cool down fail safe 
							 

		// Sleep to throtle the loop to industrial clock
		usleep(DELAY_MS); 

	} 

	// Clean file descriptors 
	close(serial_fd);
		return EXIT_SUCCESS;
} 


