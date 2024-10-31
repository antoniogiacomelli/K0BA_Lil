#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "kapi.h"

/**
 *******************************************************************************
 *
 * TASK STACKS EXTERN DECLARATION
 *
 ******************************************************************************/
#define STACKSIZE       	 128

extern UINT32 stack1[STACKSIZE];
extern UINT32 stack2[STACKSIZE];
extern UINT32 stack3[STACKSIZE];
extern UINT32 stack4[STACKSIZE];


/**
 *******************************************************************************
 *
 * TASKS ENTRY POINT PROTOTYPES
 *
 ******************************************************************************/
VOID Task1(VOID);
VOID Task2(VOID);
VOID Task3(VOID);
VOID Task4(VOID);
VOID Task5(VOID);

/******************************************************************************
 *
 * TASKS SHARED OBJECTS
 *
 * Declare them here as 'extern' objects
 *
 **/


#endif /* INC_APPLICATION_H_ */
