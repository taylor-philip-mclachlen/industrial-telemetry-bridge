#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <termios.h> // Magic header for controlling raw serial settings 
#include "protocol.h" 

#define SOURCE_PORT "/tmp/ttyV1"

int configure_port_raw(int fd) {
	struct termios tty;

	// Pull OS settings for file descriptor into tty struct 
	if (tcgetattr(fd, &tty) != 0) { 
		perror("[-] Error from tcgetattr");
		return -1;

	}

	// Turn off canonical mode (line buffer) and echoing
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// Turn off input processing
	tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR); 
	
	// Turn off output processing (raw bytes from kernel) 	
	tty.c_oflag &= ~OPOST;

	//set non-blocking parameters 
	tty.c_cc[VMIN] = 1;	// Wake up read() as exactly 1 byte arrives
	tty.c_cc[VTIME] = 0;	// No timeout timer  

	// Flush the OS buffer, clear wire with bad data
	tcflush(fd, TCIFLUSH); 

	// Push low latency rules into kernel 
	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
	    perror("[-] Error from tcsetattr");
    		return -1;
    
  	}
	return 0;
}

int main (void) {
	printf("[*] Launching Industrial Telemetry Bridge Core (ITB)...\n");

	// 1. Open the receiving side of virtual wire
	int serial_fd = open(SOURCE_PORT, O_RDONLY); 
	if (serial_fd < 0) { 
		perror("[-] Error Failed to open virtual serial port. Is socat running?"); 
		return EXIT_FAILURE;

	}
	printf("[+] Listening on raw wire %s\n", SOURCE_PORT); 

	// 2. Tame tty layer with built func 
	if (configure_port_raw(serial_fd) < 0) { 
		printf("[-] Critical configuration failure. Aborting. \n"); 
		close(serial_fd);
		return EXIT_FAILURE; 
	}
	printf ("[+] termios successfully flipped to Raw Mode (VMIN =1, VTIME =0) .\n\n"); 

	// Allocate a raw 10-byte buffer on the stack to hold the incoming raw bytes
	uint8_t rx_buffer[sizeof(TelemetryPacket)]; 
	size_t bytes_read_total =0; 

	while (1) { 
		// Read bytes directly from the kernal serial buffer 
		// Because VMIN=1, this system call blocks until at least 1 byte is available
	ssize_t result = read(serial_fd, rx_buffer + bytes_read_total, sizeof(TelemetryPacket));

	if (result < 0) { 
		perror("[-] Read Error occurred on serial bus"); 
		break; 
	}else if (result == 0) {
		// End of file / port disconnected 
		printf("[-] Wire disconnected. \n");
		break; 
	}

	bytes_read_total += result;

	// 3. Frame Alignment Check 
	// Once we collect a full 10 bytes, we parse it 
	if (bytes_read_total == sizeof(TelemetryPacket)) { 

		// Validate the first byte matches our STX marker (0x02) 
		if (rx_buffer[0] != FRAME_SYNC_BYTE) { 
			printf ("[WARN] Misaligned frame stream. Missing sync byte. Dropping 1 byte to realign...\n"); 
			// Shift buffer left by 1 byte to search for the 0x02 sync byte in the next itiration 
			memmove(rx_buffer, rx_buffer + 1, sizeof(TelemetryPacket) -1);
			bytes_read_total--; 
			continue; 

		} 

		// 4. run the Golden Rule of CRC-8 
		// Pass all 10 bytes (including CRC byte) to compute_crc8.
		// if pristine, the remainder must be equal to 0x00 
		uint8_t verification = compute_crc8(rx_buffer, sizeof(TelemetryPacket)); 

		if (verification == 0x00) { 
			// Point our structural blueprint layout over the raw byte array 
			TelemetryPacket *packet = (TelemetryPacket *)rx_buffer; 
 
			printf("[RX] [VALID] RPM: %4u | Volts: %3uV | Temp: %3d°C | Checksum: PASS (0x%02X)\n",
					packet->motor_rpm, packet->voltage, packet->temperature, packet->crc8); 
		} else { 
			printf("[ERROR] [CORRUPTED] CRC division failed. Remainder: 0x%02X. Discarding packet .\n", verification); 
		}

		// Clear out counter to process the next incoming packet 
		bytes_read_total =0; 
	 }
	
	}

	close(serial_fd); 
	return EXIT_SUCCESS;

}
	 

		
