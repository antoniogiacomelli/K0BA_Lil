/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Private Interface: Scheduler
 *
 *****************************************************************************/

#ifndef INC_KSCH_H_
#define INC_KSCH_H_

/*
 * brief Determines if the scheduler needs to reschedule a task
 * param readyTCBPtr Pointer to the new task control block
 * return TRUE if rescheduling is needed, otherwise FALSE
 */
BOOL kSchNeedReschedule(K_TCB* readyTCBPtr);

/*
 * brief Switch the scheduler to the next task
 */
VOID kSchSwtch(VOID);
/*
 * brief Enter a critical region
 * return The previous critical region state
 */
UINT32 kEnterCR(VOID);

/*
 * brief Exit a critical region
 * param crState The previous critical region state
 */
VOID kExitCR(UINT32 crState);


/*
 * brief Check for priority inversion
 */
VOID kErrCheckPrioInversion(VOID);
/*
 * brief Initializes the kernel
 */
VOID kInitKernel(VOID);

VOID kApplicationInit(ADDR* args);


#endif /* INC_KSCH_H_ */
