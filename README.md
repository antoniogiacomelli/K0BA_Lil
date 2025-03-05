# K0BA Lil' 

**K0BA** (_Kernel 0 For emBedded Applications_) is a deterministic Real-Time Kernel aiming constrained MCU-based solutions. 

## Features
* Priority Preemptive Scheduler: O(1) time complexity
* Fast bitwise scheduling algorithm
* Time-Slice Scheduling option
* High-precision application timers: O(1) time complexity
* Synchronisation mechanisms: Semaphores, Mutexes (with Priority Inheritance), Condition Variables, Event Flags and Direct Task Signalling
* Flexible Message Passing Mechanisms (Synchronous, Asynchronous, by copy or reference)
* Fixed-size Block Memory Allocator: O(1) time complexity
* Highly modular
* Footprint as low as 6KB for minimal configuration.

See the docbook:

https://antoniogiacomelli.github.io/K0BA_Lil/


> [!NOTE]
> This is a work in progress. 

*Logical Architecture:*

<img width="450" alt="kernel" src="https://github.com/antoniogiacomelli/K0BA_Lite/blob/master/layeredkernel.png">

 
## Dependencies

Currently the kernel _depends_ on CMSIS:

* CMSIS-GCC
* CMSIS-Core
