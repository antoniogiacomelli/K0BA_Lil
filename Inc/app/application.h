#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include <kapi.h>

/**
 *******************************************************************************
 *
 * TASK STACKS EXTERN DECLARATION
 *
 ******************************************************************************/
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
void Task1(void);
void Task2(void);
void Task3(void);
void Task4(void);
void Task5(void);

/**
 ******************************************************************************
 *
 *  KERNEL DEPENDENT APPLICATION-SPECIFIC OBJECTS 
 *
 * You shall declare them here as 'extern' objects
 *
 **/

struct MESG { int a; int b; }__attribute__((aligned)); /* a custom message */
extern struct MESG mesgPool[10];   /* a pool of custom messages */
extern K_MEM		MESGmem;           /* a memory control block of custom messages */


#endif /* INC_APPLICATION_H_ */
