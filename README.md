
# K0BA Lite
*Kernel 0 For emBedded ARM* aims to be a microkernel for ARM Cortex-M microprocessors. 
The figure below shows the target architecture of an entire embedded software system using this
microkernel.

<img width="450" alt="microkernel" src="https://user-images.githubusercontent.com/62488903/209377097-07d90421-afe0-4749-adac-3a875641aa51.png">

The current features are:
* Preemptive Scheduling (Fixed priority with time-slice) 
* Synchronization mechanisms: sleep/wakeup, semaphores and mutexes
* ITC Mechanisms: Synchronous/Asynchronous Message Passing, PIPEs and FIFO queues.

To be implemented:
* Event flags
* Time-triggered scheduler option
* Memory pools

To be improved:
* Configuration
* Return Values
* Housekeeping/error checking
* Portability (other compilers and HAL APIs)

Major dependencies:
* ARM GCC 
* CMSIS HAL

The version available here uses a single stack pointer. Application threads and kernel threads share the same address
space, there is no memory virtualization. Kernel calls are then simple function calls. This is an acceptable approach
for small embedded systems.
An implementation with lightweight processes is under development.

So far this kernel has been tested on MCUs based on: ARM Cortex-MO+, ARM Cortex-M3 and ARM Cortex-M7.

*Demo Application, on an STM32 board using the vendor provided HAL and BSP:*
```c

  /******************************************************************************
  * @file           : main.c
  * @brief          : Main program body of a system using K0BA
  ******************************************************************************/

#include "app/main.h"
#include "app/tasks.h"
#include "kernel/kernel.h"
#include <assert.h>
#include <string.h>
#include "../../Inc/services/usart_services.h"

USART_Interface serviceUSART = {0}; /*USART interface instance*/

int main(void)
{
	/*********************************/
	/* HAL dependent initializations */
	/*********************************/
	HAL_Init(); // HAL API init
	SystemClock_Config(); // HAL call to config system clock
	SysTick_Config(SystemCoreClock / 1000); /* BSP call: Configuring tick for 1ms*/
 	/*CMSIS HAL calls*/
	NVIC_SetPriority(SysTick_IRQn, 0);
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(USART3_IRQn); /* USART service is interrupt-driven */
	MX_GPIO_Init(); /* BSP Call for GPIO init*/
	__disable_irq();
	/*************************/
	/* Kernel Initialization */
	/************************/
	kMbufInitAll(); /* kernel call to initialize the Message Buffer pool */
	kPipeInitAll(); /* kernel call to initialize the PIPEs pool */
        /* kAddTask(TaskFunction, void* arguments, unique ID, priority [highest=0]) */
	assert(kAddTask(TaskIdle, (void*)0, 0, 2) == OK); /* Adding task idle */
	assert(kAddTask(Task1, (void*)0, 1, 1) == OK); 
	assert(kAddTask(Task2, (void*)0, 2, 1) == OK); 
	assert(kAddTask(Task3, (void*)0, 3, 1) == OK);
	/* Adding a server-task - this task receives calls through synchronous                                                          
   	message passing from client tasks - it has highest priority */
	assert(kAddTask(UART_Server_Task, (void*)0, 4, 0) == OK); 
	sUSART_Create(&serviceUSART); /*Creating UART service */
	serviceUSART.init();  /*Initializing uart service */
	serviceUSART.puts((const uint8_t*)"K0BA 0.1L is up\n\r");
	
	/* Boot K0BA */
	__enable_irq();
	kStart(); 
    	while(1)
    	;
}

/******************************************************************************
* @file           : tasks.c
* @brief          : Demo Application execution units
******************************************************************************/
#include "kernel/kernel.h"
#include "app/tasks.h"
#include "stm32f7xx_hal.h"
#include "../../Inc/services/usart_services.h"
#include <assert.h>

#define UART_SERVER 4 // server task id
USART_Interface serviceUSART; // UART service interface
SEMA_t task3SEMA; 

/* TaskIdle is in kernel.c */

void Task1(void* args)
{
	kSemaInit(&task3SEMA, 0);
	const uint8_t msg1[]="Task 1\n\r";
	volatile uint8_t t3Counter = 0;
	while(1)
	{
		/* send msg1 to be printed by UART Server task (it only does that)
                   if more commands were to be executed by this server an APDU
                   message could be used, for instance
		*/
		if (OK == kSendMsg(msg1, UART_SERVER))
		{
			t3Counter++;
			if (t3Counter >= 5)
			{
				t3Counter = 0;
				/* since the message passing is synchronous, there is no need
				   to add (another) mutual exclusion to the shared resource */
				assert(kSendMsg((const uint8_t*)"Signaling task3SEMA\n\r",
					UART_SERVER) == 0);
				kSemaSignal(&task3SEMA);
			}
			kYield(); /* yields cpu cooperatively */
		}
	}
}
void Task2(void* args)
{
	const uint8_t msg2[]="Task 2\n\r";

	while(1)
	{
		if (OK == kSendMsg(msg2, UART_SERVER))
			kYield();
	}
}
void Task3(void* args)
{
	const uint8_t msg3[] = "Task 3 is going to sleep 3000 ticks\n\r";
	const uint8_t msg3_1[] = "Task 3 resumed\n\r";

	while(1)
	{
		assert(kSendMsg((const uint8_t*)"Task 3 will block on semaphore\n\r",
			UART_SERVER) == OK);
		kSemaWait(&task3SEMA);
		assert(kSendMsg(msg3, UART_SERVER) == OK);
		kSleepTicks(3000);
		assert(kSendMsg(msg3_1, UART_SERVER) == OK);
		/* wait for preemption */
	}
}
void UART_Server_Task(void* args)
{
	uint8_t rcvd_msg[MSG_SIZE] = {'\0'};
	int8_t ret = NOK;
	while(1)
	{
		ret = kRcvMsg(rcvd_msg);
		if (ret != NOK)
		{
			serviceUSART.puts(rcvd_msg);
			HAL_Delay(500);
		}
		kYield();
	}
}


```

*Demo output:*

![image](https://github.com/antoniogiacomelli/K0BA_Kernel/assets/62488903/54114066-2ad8-4bfb-a5e3-d7058adbd6af)

