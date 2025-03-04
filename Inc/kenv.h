
#ifndef KENV_H
#define KENV_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************
 Place HAL, Compiler and other system includes here
 e.g: 

#include <stm32f3xx_hal.h>
#include <cmsis_gcc.h>
#include <stdio.h>

Then, in kconfig.h set the CUSTOM_ENV define to (1).
*******************************************************/

#define SystemCoreClock (SystemCoreClock)

/* If defined, customise it to your own UART. 
   The _write() backend syscall is at kutils.c
*/
//#define K_DEF_PRINTF

#ifdef K_DEF_PRINTF
/* extern data, as peripheral handlers declarations, etc*/
extern UART_HandleTypeDef huart2;
#endif

#ifdef __cplusplus
}
#endif
#endif /*ENV_H*/
