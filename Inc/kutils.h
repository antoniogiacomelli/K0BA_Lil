/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]]
 *
 ******************************************************************************
 ******************************************************************************
 *  In this header:
 *                  o Misc utils
 *****************************************************************************/

#ifndef K_UTILS_H
#define K_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif


ULONG kStrLen(STRING s);
ULONG kMemCpy(ADDR destPtr, ADDR const srcPtr, ULONG size);

#ifdef K_DEF_PRINTF

extern UART_HandleTypeDef huart2;
int _write(int file, char* ptr, int len);

#endif

#ifdef __cplusplus
 }
#endif

#endif
