
#ifndef KENV_H
#define KENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* include C libs and HAL here 
   then set CUSTOM_ENV to (1)
   in kconfig.h
*/


/*#define K_DEF_PRINTF*/
#ifdef K_DEF_PRINTF
/* extern data, as peripheral handlers declarations, etc*/
extern UART_HandleTypeDef huart2;
#endif

#ifdef __cplusplus
}
#endif
#endif /*ENV_H*/
