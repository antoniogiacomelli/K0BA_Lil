/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o System Tasks and Deferred Handlers function
 * 					  prototypes.
 *
 *****************************************************************************/
#ifndef KSYSTASKS_H
#define KSYSTASKS_H
#ifdef __cplusplus
extern "C" {
#endif

VOID IdleTask(VOID);
VOID TimerHandlerTask(VOID);
BOOL kTimerHandler(VOID);

#ifdef __cplusplus
 }
#endif
#endif /* KSYSTASKS_H */
