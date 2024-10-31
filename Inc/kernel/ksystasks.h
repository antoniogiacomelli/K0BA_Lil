/*
 * ksystasks.h
 *
 *  Created on: Oct 22, 2024
 *      Author: anton
 */

#ifndef INC_KSYSTASKS_H
#define INC_KSYSTASKS_H

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* Tasks */
extern UINT32 idleStack[IDLE_STACKSIZE];
extern UINT32 timerHandlerStack[TIMHANDLER_STACKSIZE];

void IdleTask(void);
void TimerHandlerTask(void);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/* Deferred Handlers */
void kTimerHandler(void);

#endif /* INC_KSYSTASKS_H_ */
