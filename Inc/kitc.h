/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
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

/* publicised only needed prototypes */

K_ERR kTaskPend( TICK);
K_ERR kTaskSignal( K_TASK* const);

#if(K_DEF_MUTEX==ON)
K_ERR kMutexLock( K_MUTEX* const, TICK tmeout);
K_ERR kMutexUnlock( K_MUTEX* const);
#endif

#if (K_DEF_EVENT==ON)
K_ERR kEventInit( K_EVENT* const);
K_ERR kEventWake( K_EVENT* const);
K_ERR kEventSignal( K_EVENT* const);
K_ERR kEventSleep( K_EVENT*, TICK);
#endif

#if (K_DEF_EVENT_FLAGS==ON)

K_ERR kEventFlagsSet( K_EVENT* const, ULONG, ULONG*, ULONG);

K_ERR kEventFlagsGet( K_EVENT* const, ULONG, ULONG*, ULONG, TICK);

ULONG kEventFlagsQuery( K_EVENT* const);

#endif
/**
 * \brief A task pends on its own binary semaphore
 * \param timeout Suspension time until signalled
 * \return K_SUCCESS or specific error
 */
K_ERR kTaskPend( TICK timeout);

/**
 * \brief Signal a task's binary semaphore
 * \param taskHandlePtr Pointer to task handle
 * \return K_SUCCESS or specific error
 */
K_ERR kTaskSignal( K_TASK *const taskHandlePtr);


#if (K_DEF_TASK_FLAGS == ON)

K_ERR kTaskFlagsPost( K_TASK *const taskHandlerPtr, ULONG flagMask,
		ULONG *updatedFlagsPtr, ULONG option);

ULONG kTaskFlagsGet( ULONG flagMask, ULONG option, TICK timeout);
#endif
#if ((K_DEF_EVENT == ON) && (K_DEF_MUTEX == ON))
/* this is a helper for condition variables to perform the wait atomically
 unlocking the mutex and going to sleep
 for signal and wake one can use EventSignal and EventWake */
__attribute__((always_inline))
  inline K_ERR kCondVarWait( K_EVENT *eventPtr,
		K_MUTEX *mutexPtr, TICK timeout)
{
	K_ERR err;
	/* atomic */
	K_CR_AREA
	K_CR_ENTER
	kMutexUnlock( mutexPtr);
	err = kEventSleep( eventPtr, timeout);
	K_CR_EXIT
	return (err);
	/* upon returning (after wake) the condition variable must lock the
	 * the mutex and loop around the condition */
}
__attribute__((always_inline))
  inline K_ERR kCondVarSignal( K_EVENT *eventPtr)
{
	return (kEventSignal( eventPtr));
}
__attribute__((always_inline))
  inline K_ERR kCondVarBroad( K_EVENT *eventPtr)
{
	return (kEventWake( eventPtr));
}
#endif
#ifdef __cplusplus
}
#endif
#endif /* K_ITC_H */
