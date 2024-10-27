# K0BA
*Kernel 0 For Embedded ARM* aims to be a microkernel for ARM Cortex-M microprocessors. 

<img width="450" alt="microkernel" src="https://user-images.githubusercontent.com/62488903/209377097-07d90421-afe0-4749-adac-3a875641aa51.png">

The current features are:
* Preemptive Scheduling (Fixed priority with time-slice) 
* Cooperative Scheduling
* User space and Kernel space threads
* System Calls
* Synchronization mechanisms: sleep/wakeup, semaphores and mutexes
* ITC Mechanisms: Mailboxes, FIFOs and Synch Message Passing

To be Implemented:
* Nested interrupts support
* Priority Inheritance
* MPU configuration to split kernel memory from user memory

Major dependencies:
* ARM GCC 
* CMSIS HAL

_The system was implemented on the top of a BSP for an Arduino Due (SAM3X8E) provided by Atmel, 
but can be easily ported to any BSP that uses CMSIS HAL_
