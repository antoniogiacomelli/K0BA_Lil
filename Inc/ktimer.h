#ifndef K_TIMER_H
#define K_TIMER_H

#if (K_DEF_CALLOUT_TIMER==(ON))
/* Timer Reload / Oneshot optionss */
#define K_RELOAD      1
#define K_ONESHOT     0
extern K_TIMER *dTimReloadList; /* periodic timers */
extern K_TIMER *dTimOneShotList; /* one-shot timers */
BOOL kTimerHandler( VOID);
K_ERR kTimerInit( K_TIMER*, TICK, TICK, CALLOUT, ADDR, BOOL);
extern K_TIMER* currTimerPtr;
VOID kRemoveTimerNode( K_TIMEOUT_NODE *);

#endif

extern volatile K_TIMEOUT_NODE *timeOutListHeadPtr;
extern volatile K_TIMEOUT_NODE *timerListHeadPtr;
K_ERR kTimeOut( K_TIMEOUT_NODE*, TICK);
BOOL kHandleTimeoutList( VOID);
VOID kRemoveTimeoutNode( K_TIMEOUT_NODE*);
extern struct kRunTime runTime; /* record of run time */
VOID kBusyDelay( TICK const);

#define BUSY(t) kBusyDelay(t)

VOID kSleep( TICK const);

#define SLEEP(t) kSleep(t)

TICK kTickGet( VOID);

#if (K_DEF_SCH_TSLICE==OFF)

VOID kSleepUntil( TICK const);

#endif

#endif
