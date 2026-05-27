# Industrial Telemetry Bridge (ITB)

A high-performance, hardware-agnostic edge gateway designed to bridge legacy, deterministic industrial serial communication with modern, concurrent Linux environments. 

This project serves as an architectural exploration of deterministic byte-packing, low-latency kernel manipulation, and decoupled high-level orchestration—**all emulated and observed within a single machine.**

---

##  The Lab Environment

To study latency, synchronization, and memory layout without physical PLC hardware, the entire industrial bus topology is virtualized on local hardware:

* **Host Machine:** HP Omen 15-ax252nr
* **CPU:** Intel® Core™ i7-7700HQ (4 Physical Cores, 8 Execution Threads @ 2.80GHz base)
* **Operating System:** Manjaro Linux (Kernel `6.12.85-1`)
* **Virtual Wire:** `socat` version `1.8.1.1-1`

---

##  Architectural Stack & Abstraction Layers

The data pipeline mimics a bare-metal industrial gateway, stepping cleanly through the operating system layers:
