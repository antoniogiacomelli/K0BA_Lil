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
K_ERR kMutexLock( K_MUTEX* const, TICK tmeout);
VOID kMutexUnlock( K_MUTEX* const);

#ifdef __cplusplus
}
#endif
#endif /* K_ITC_H */
