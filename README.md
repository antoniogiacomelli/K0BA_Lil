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

To be implemented:
* Event flags
* Time-triggered scheduling
* Memory pools

To be improved:
* Configuration
* Return Values
* Housekeeping/error checking
  
* The version available here uses a single stack pointer. Application threads and kernel threads share the same address
space, there is no memory virtualization. Kernel calls are then simple function calls. This is an acceptable approach
for small embedded systems.
An implementation with lightweight is processes under development.

So far this kernel has been tested on the following MCUs: ARM Cortex-MO+, ARM Cortex-M3 and ARM Cortex-M7.

###Initialization example, on an STM32 board using the provided HAL and BSP:
```c
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body 
  ******************************************************************************

#include "app/main.h"
#include "app/tasks.h"
#include "kernel/kernel.h"
#include <assert.h>
#include <string.h>
#include "../../Inc/services/usart_services.h" 
USART_Interface serviceUSART = {0}; /*USART interface instance*/

int main(void)
{
	HAL_Init(); // HAL API init
	SystemClock_Config(); // HAL call to config system clock
	__disable_irq();
	MX_GPIO_Init(); /* BSP Call for GPIO init*/
	kMbufInitAll(); /* kernel call to initialize the Message Buffer pool */
	kPipeInitAll(); /* kernel call to initialize the PIPEs pool */
	assert(kAddTask(TaskIdle, (void*)0, 0, 2) == OK); /* Adding task idle */
	assert(kAddTask(Task1, (void*)0, 1, 1) == OK); 
	assert(kAddTask(Task2, (void*)0, 2, 1) == OK); 
	assert(kAddTask(Task3, (void*)0, 3, 1) == OK);
	/* Adding a server-task - this task receives calls through synchronous                                                          
   	message passing from client tasks - it has max priority */
	assert(kAddTask(UART_Server_Task, (void*)0, 4, 0) == OK); 
	SysTick_Config(SystemCoreClock / 1000); /* BSP call: Configuring tick for 1ms*/
	NVIC_SetPriority(SysTick_IRQn, 0); /*CMSIS HAL calls*/
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(USART3_IRQn);
	sUSART_Create(&serviceUSART); /*Creating UART service */
	serviceUSART.init();  /*Initializing uart service */
	serviceUSART.puts((const uint8_t*)"K0BA 0.1L is up\n\r");
	__enable_irq();
	kStart(); //initializes kernel
    	while(1)
    	;
}


Major dependencies:
* ARM GCC 
* CMSIS HAL
