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

#if (K_DEF_TASK_SIGNAL_BIN_SEMA==ON)
K_ERR kTaskPend(TICK);
K_ERR kTaskSignal(K_TASK_HANDLE* const);
#else
K_ERR kTaskPend(VOID);
K_ERR kTaskSignal(K_TASK_HANDLE* const);
#endif

#ifdef __cplusplus
}
#endif
#endif /* K_ITC_H */
