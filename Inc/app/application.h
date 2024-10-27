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
 * TASKS SHARED OBJECTS
 *
 * You might want to declare them here as 'extern' objects
 *
 **/

struct MESG { int a; int b; }__attribute__((aligned));
extern struct MESG mesgPool[10];
extern K_MEM		MESGmem;


#endif /* INC_APPLICATION_H_ */
