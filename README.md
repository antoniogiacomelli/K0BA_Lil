# K0BA Lite v0.1.0-beta

*Kernel 0 For Embedded Applications* is a Real-Tim Kernel* 

This is a light-weight version with no memory isolation. Currently it supports ARMv7M architecture.

*Logical Architecture:*

<img width="450" alt="kernel" src="https://github.com/antoniogiacomelli/K0BA_Lite/blob/main/layeredkernel.png">

**F E A T U R E S:**

- **Priority Preemptive Scheduler with Rate-Monothonic Scheduling**
  
- **Synchronization:**:
  - Sleep/WakeUp on Events
  - Semaphores
  - Mutexes (with priority inheritance)
  - Posix-Like Condition Variables
  - Direct Task Signal
  
- **Inter-task Communication:**
  - Mailboxes (with extended rendez-vous)
  - Message Queues with variable item-size
  - Stream Buffers (Simple FIFOs and "UNIX-like" pipes)

- **Memory Pools with fixed block-size for dynamic memory allocation**

- **Application Timers (using delta queues for efficient handling)**


**Major dependencies:**
- ARM GCC 
- CMSIS HAL

