#ifndef K_TIMER_H
#define K_TIMER_H
#include "kmem.h"
#include "kitc.h"
/* Timer Reload / Oneshot optionss */

#define RELOAD      1
#define ONESHOT     0

extern K_TIMER *dTimReloadList; /* periodic timers */
extern K_TIMER *dTimOneShotList; /* one-shot timers */
extern K_TIMER *dTimSleepList; /* sleep delay list */

extern K_TIMEOUT_NODE* timeOutListHeadPtr ;
VOID kTimeOut(K_TIMEOUT_NODE, TICK);
VOID kRemoveTaskFromMbox(ADDR);
VOID kRemoveTaskFromSema(ADDR);
VOID kRemoveTaskFromMutex(ADDR);
VOID kRemoveTaskFromQueue(ADDR);
BOOL kHandleTimeoutList(VOID);
VOID kRemoveTaskFromEvent(ADDR);

extern struct kRunTime runTime; /* record of run time */

K_ERR kTimerInit(STRING, TICK const, CALLOUT const,
		ADDR const, BOOL const);

VOID kBusyDelay(TICK const);

#define BUSY(t) kBusyDelay(t)

VOID kSleep(TICK const);

#define SLEEP(t) kSleep(t)

TICK kTickGet(VOID);

BOOL kTimerHandler(VOID);

K_ERR kTimerPut(K_TIMER* const);

#if (K_DEF_SCH_TSLICE==OFF)

VOID kSleepUntil(TICK const);

#endif

#endif
