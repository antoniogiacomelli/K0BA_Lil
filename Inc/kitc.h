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

K_ERR kPend(VOID);
K_ERR kSignal(K_TASK_HANDLE const);

#ifdef __cplusplus
}
#endif
#endif /* K_ITC_H */
