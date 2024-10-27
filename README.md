# K0BA Lite 

*Kernel 0 For Embedded Applications* is a Real-Time Kernel for constrained devices. 

This is a light-weight version with no memory isolation. Currently it supports ARMv7M architecture.

*A version with memory isolation will be made public soon*.

> [!NOTE]
> This is a work in progress.

- See the wiki for usage examples: https://github.com/antoniogiacomelli/K0BA_Lite/wiki
- The API is mostly documented on: https://antoniogiacomelli.github.io/K0BA_Lite/

*Logical Architecture:*

<img width="450" alt="kernel" src="https://github.com/antoniogiacomelli/K0BA_Lite/blob/main/layeredkernel.png">

## FEATURES
- **Clean API**
   - Standardised easy-to-grasp API convention
 
- **Minimal overhead**
   - This "Lite" version is tuned for minimal overhead, focused on reducing indirection levels and
     function calls. Rather than "a microkernel approach with opaque objects" the kernel is a *modular monolith*.
   
- **Priority Preemptive Scheduler with Rate-Monothonic Scheduling**
  
- **Inter-Task Synchronisation:**
  - Sleep/WakeUp on Events
  - Semaphores
  - Mutexes (with priority inheritance to avoid priority-inversion)
  - Condition Variables for finer-grained waiting logic
  - Direct Task Signal
  
- **Inter-Task Communication:**
  - Mailboxes (with extended rendez-vous) for fully synchronous exchange
  - Message Queues with variable item-size
  - PIPEs:
    - Simple FIFOs: these are simple circular buffers for one-producer to one-consumer.
    - "Extended" Pipes: these are pipes that can be managed by several readers and writers. 

- **Memory Management:**
  - Memory Pools with fixed block-size. This approach is deterministic and hinders fragmentation.

- **Application Timers**
    - Periodic/One Shot timers with delta queues for efficient handling

**Major dependencies:**
- ARM GCC 
- CMSIS HAL

