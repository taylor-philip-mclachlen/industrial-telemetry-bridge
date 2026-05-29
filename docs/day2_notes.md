day 2 telementry bridge project. 
not in school today so we are taking advantage of my free time. 

made some type corrections to the protocol.h seems solid. 

focus today will be socat / termois. 

so our socat will be emulating the livewire. which will make the C application thin kits speaking directly to real hardware. i think a good proof of concept but also proving that resourfulness overrules accessibilty. if you want to get philisophical. 

RS-232 / RS-485 serial controller is the target. we will use this on real hardware. once its proven concept. 

---
so to emulate the bare metal(motor controller) we need to pump raw bytes into the wire (socat) 

[200~Leave this terminal running. It acts as your physical RS-232 cable.

/tmp/ttyV0 will be our Transmitter (the mock machinery script).

/tmp/ttyV1 will be our Receiver (your future ITB C core).

socat PTY,link=/tmp/ttyV0,raw,echo=0 PTY,link=/tmp/ttyV1,raw,echo=0

termois practicality for this project. 

The canonical mode default tty is a beautiful thing that i use everyday. but in this application the kernel stores the incoming data in a OS buffer. 
in a simulation test on the canonical mode i might run the app and Voltage might just evaluate 0x0A, then the kernel thinks this is a newline. it will fire this chunck of data to the app and the c app just blocks indefinetly. 

termois in raw mode takes all that away, strpping the terminal of this human behavior. 

termois rules:
disable canonical inputs, no waiting for \n 
VMIN = 1: tell the kernel that every one byte that lands, you have to wake up the C program.
VMIN = 0: disable read timers so it dont time out. 

without this rules the C program cant stream raw bytes.

ok now we move to the generator script. 

thoughts are that python seems perfect for writting a mock motor controll, but im already stretched too thin with learnign go and C. two options are write in C using, which will allow me to undstand binary struct handling better in C. 

alternative option is Go as i am learning it for tooling/sidekick things such as this it would be a good opertunity to prove its functionallity on not just high level, or cloud applications.  

pros / cons

C:  more time with syntax that i need to learn, alligns well with the low level code already written in C, also when i want to stress test the system its easy to install faults into the generator.c app.
also: bit by bit persiontion for simulating corrupted bytes. low overhead, and very high levels of control over the timing and complexity of faults. 

GO: Great compilier, saftey, and byte slices insted of bit manipulation. 
also: Scalibilty is GO's shining star. goroutines would be amazing for later scaling this project to 10's of plcs or controls. more dynamic behavior, network dropping, timingout, or partial packets, instant struncating packets. and yeah we can touch on safety again. memory safety is no joke especially when we are talking about industrial applications like this. honestly another good reason for me to stick with go, it offers a nice balance to C. HIGH LEVEL FUNCTIONALITY, SAFETY AND GOROUTINES, VS LOW LEVEL FUNCTIONALITY, HYPER CONTROLL, LOW OVERHEAD. seems like a good pair of pros and cons. 

yeah this is great stuff. makes me realize that GO and C have great synergies. they lack the freeedom of python, but in there rigidity they off alot of balance to each other, not just in applications, but also in control vs safety, hyper scalibity vs low overhead. its an interesting relationship im happy to explore. 


we finsidhed the generator.c, lets unpack this line by line to undserstand function. 

Opening File Descriptor:
     int serial_fd = open(TARGET_PORT, O_WRONLY); 

open():everytthing is a file" we are opening the ttyV0 where we have socat running as the wire in raw tty. 
O_WRONLY: tells the kernel to only push data, no read. 
this will return an integer FD, this integer is like an index that points to our socat tty 


Stack Memory: 
    while(1) { 
     TelemetryPacket packet; 

inside the infinite loop, we decalre this. A decliration inside a function. the comilier allocated 10 bytes of space on the stack. every loop itiration, this variable get rewritten with new data. 


Struck field Population:
    packet.sync_byte = FRAME_SYNC_BYTE; 
    packet.motor_rpm = current_rpm; 

because we used #pragma pack(push 1) in the protocol.h file the compilier layouts these variables in memory sequentially, back-toback, with no empty spacees. 
Offset 0 = 1-byte RPM marker (0x02) 
OFfset 1 = 4-byte RPM integer 
Offset 5 = 2-byte Voltage Integer 
Offset 7 = 2-byte signed Temp integer 

Pointer Cast Trick for CRC: 
    packet.crc8 = compute_crc8((const uint8_t *) &packet, 9); 

pure low level C here, %packet gets the memory address at the start of the struct. (const uint8_t tells the compilier to treat TelemetryPacket struct as a single, flat array of 1-byte integers (uint8_t). we then send this array to compute_crc8 telling it to read exactly 9 bytes( everything execpt the last crc slot). this function loop dose the famous polynomial math and spits out a 1 byte answer which becomes the 10th byte of the struct.  superrrrr coooool function.  

Raw Bytes to Kernel: 
    ssize_t Bytes_written = write(serial_fd, &packet, sizeof(TelemetryPacket)); 

write() is a linux system call. sizeof(TelemetryPacket) is 10-bytes 
the kernel copies this 10-bytes stright out of our program memory and moves them to pipe managed by socat. 

Throttling with usleep: 
    usleep(DELAY_MS); 

without this while(1) the loop would run as fast as the i7 cpu could process and exicute. millions of times per seccond. usleep(50000) tells the kernel put this program to sleep for 500 millisecconds, wake me up when the time has passed. this is where RTOS and industrial clocks are important. we can talk about that more! 


