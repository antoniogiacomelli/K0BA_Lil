# K0BA_Kernel
Kernel 0 for emBedded Arm
# K0BA
*Kernel 0 For Embedded ARM* aims to be a microkernel for ARM Cortex-M microprocessors. 
The figure below shows the target architecture of a whole software system using this
microkernel.

<img width="450" alt="microkernel" src="https://user-images.githubusercontent.com/62488903/209377097-07d90421-afe0-4749-adac-3a875641aa51.png">

The current features are:
* Preemptive Scheduling (Fixed priority with time-slice) 
* Cooperative Scheduling
* Synchronization mechanisms: sleep/wakeup, semaphores and mutexes
* ITC Mechanisms: Synchronous/Asynchronous Message Passing, PIPEs and FIFO queues.


Major dependencies:
* ARM GCC 
* CMSIS HAL
