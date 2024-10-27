/*
 * ksystasks.h
 *
 *  Created on: Oct 22, 2024
 *      Author: anton
 */

#ifndef INC_KSYSTASKS_H_
#define INC_KSYSTASKS_H_

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* Tasks */
extern UINT32 idleStack[STACKSIZE];
extern UINT32 timerHandlerStack[STACKSIZE];

void IdleTask(void);
void TimerHandlerTask(void);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* Deferred Handlers */
void kTimerHandler(void);

#endif /* INC_KSYSTASKS_H_ */
