/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]]
 *
 ******************************************************************************
 ******************************************************************************
 *  In this header:
 *                  o Inter-task synchronisation and communication kernel
 *                    module.
 *
 *****************************************************************************/

#ifndef K_ITC_H
#define K_ITC_H

#ifdef __cplusplus
extern "C" {
#endif

K_ERR kTaskPend( TICK);
K_ERR kTaskSignal( K_TASK* const);

#if(K_DEF_MUTEX==ON)
K_ERR kMutexLock( K_MUTEX* const, TICK tmeout);
VOID kMutexUnlock( K_MUTEX* const);
#endif
/***************************************************************************/
/* CONDITION VARIABLES                                                     */
/***************************************************************************/

#if ((K_DEF_MUTEX==ON) && (K_DEF_EVENT==ON))
VOID kEventSignal( K_EVENT *);
VOID kEventWake( K_EVENT *);
K_ERR kEventSleep( K_EVENT *, TICK );

/* this is a helper for condition variables to perform the wait atomically
 unlocking the mutex and going to sleep
 for signal and wake one can use EventSignal and EventWake */
inline K_ERR kCondVarWait( K_EVENT *eventPtr, K_MUTEX *mutexPtr, TICK timeout)
{
	K_ERR err;
	/* atomic */
	K_CR_AREA
	K_CR_ENTER
	kMutexUnlock( mutexPtr);
	err = kEventSleep( eventPtr, timeout);
	K_CR_EXIT
	return (err);
	/* upon returning (after wake) the condition variable must loop lock the
	 * the mutex and loop around the condition */
}
inline VOID kCondVarSignal( K_EVENT *eventPtr)
{
	return (kEventSignal( eventPtr));
}
inline VOID kCondVarBroad( K_EVENT *eventPtr)
{
	return (kEventWake( eventPtr));
}
#endif
#ifdef __cplusplus
}
#endif
#endif /* K_ITC_H */
