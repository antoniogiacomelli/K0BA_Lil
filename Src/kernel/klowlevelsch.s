/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module            : Low-level Scheduler/Start-Up.
 *  Provides to       : High-level Scheduler / Main application.
 *  Application API   : No
 *
 *  o In this unit:
 *                Low-level kernel routines.
 *
 *****************************************************************************/


/*@file system.s */
.syntax unified // needed for Thumb2
.text
.align 4

/* these are offsets from the TCB */
.equ SP_OFFSET, 0
.equ STATUS_OFFSET, 4
.equ RUNCNTR_OFFSET, 8
.equ SCB_ICSR, 0xE000ED04
.equ ISCR_SETPSV, (1 << 28)
.equ ISCR_CLRPSV, (1 << 27)
.equ STICK_CTRL, 0xE000E010

.global SAVEUCONTEXT
.type SAVEUCONTEXT, %function
.thumb_func
SAVEUCONTEXT:
    MRS R12, PSP              /* Get the current process stack pointer (PSP) */
    STMDB R12!, {R4-R11}      /* Save R4-R11 on the process stack */
    MSR PSP, R12              /* Update PSP with the new stack pointer */
    LDR R0, =runPtr           /* Load address of runPtr into R0 */
    LDR R1, [R0]              /* Load value of runPtr (pointer to task context) into R1 */
    STR R12, [R1]             /* Store updated stack pointer (PSP) into runPtr->sp */
    DSB
    BX LR                     /* Return from function */

.global RESTOREUCONTEXT
.type RESTOREUCONTEXT, %function
.thumb_func
RESTOREUCONTEXT:
    LDR R0, =runPtr                 /* Load address of runPtr into R0 */
    LDR R1, [R0]                   /* Load value of runPtr (pointer to task context) into R1 */
    LDR R3, [R1, #RUNCNTR_OFFSET]  /* Load run counter (runPtr->runCounter) */
    ADDS R3, #1               /* Increment run counter */
    STR R3, [R1, #RUNCNTR_OFFSET]  /* Store updated run counter back */

    MOVS R12, #2              /* Load RUNNING status (2) */
    STR R12, [R1, #STATUS_OFFSET]  /* Update status to RUNNING (runPtr->status) */

    LDR R2, [R1]              /* Load saved stack pointer (runPtr->sp) */
    LDMIA R2!, {R4-R11}       /* Restore registers R4-R11 */
    MSR PSP, R2               /* Update PSP with the new stack pointer */
    DSB
    BX LR                     /* Return from function */


.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func
PendSV_Handler:
                /* R0-R3, R12, LR and xPSR from the interrupted task are pushed
                on stack  */
    SWITCHTASK:
    CPSID I             /* Critical section */
    BL SAVEUCONTEXT     /* Save user context */
    BL kSchSwtch       /* Call scheduler */
    BL RESTOREUCONTEXT  /* Resume scheduled task */
    LDR R0, =SCB_ICSR
    LDR R1, =ISCR_CLRPSV
    STR R1, [R0]            /* Clear pendsv */
    MOV LR, #0xFFFFFFFD     /* Return to thread mode using PSP */
    DSB
    CPSIE I                 /* CR end */
    ISB                     /* Flush pipeline */
    BX LR                   /* Dispatch scheduled task */

.global SysTick_Handler
.type SysTick_Handler, %function
.thumb_func
SysTick_Handler:
    CPSID I
    CMP LR, #0xFFFFFFF1
    BEQ RESUMEHANDLER
    BNE TICKHANDLER
    RESUMEHANDLER:
    CPSIE I
    BX LR
    TICKHANDLER:
    PUSH {LR}              /* Save LR */
    BL kTickHandler        /* Call kTickHandler, result in R0 */
    POP {LR}               /* Restore LR */
    CMP R0, #1             /* Compare result to 1 */
    BEQ SETPENDSV          /* If equal, set PendSV */
    EXIT:                  /* Fallthrough to exit if not equal */
    MOV LR, #0xFFFFFFFD     /* If here, a blocking task was interrupted
                               return to PSP*/
    CPSIE I                /* Enable interrupts */
    ISB                    /* Instruction Synchronization Barrier */
    BX LR                  /* Return */
    SETPENDSV:
    LDR R0, =SCB_ICSR      /* Load SCB_ICSR address */
    LDR R1, =ISCR_SETPSV   /* Load ISCR_SETPSV value */
    STR R1, [R0]           /* Set PendSV */
    DSB
    CPSIE I
    ISB
    BX LR


.global SVC_Handler
.type SVC_Handler, %function
.thumb_func
SVC_Handler:
    STARTUP:
    LDR R0, =STICK_CTRL     /* Load SysTick Control Status Register */
    MOVS R1, #0x0F          /* MOV 0b1111 to R1 */
    STR R1, [R0]            /* Store 0b1111 SysTick CSR */
    DSB
    B kStartUp              /* Branch to start-up */

.global kStartUp
.type kStartUp, %function
.thumb_func
 kStartUp:
    CPSID I                  /* (Critical Section Begin) */
    BL kApplicationInit
    MOVS R0, #2               /* Load value RUNNING into R0 */
    MSR CONTROL, R0          /* Set CONTROL register (switch to PSP) */
    LDR R1, =runPtr          /* R1 = &runPtr */
    LDR R2, [R1, #SP_OFFSET] /* Load runPtr->sp */
    MOVS R12, #0x02          /* Set runPtr->status = RUNNING */
    STR R12, [R2, #STATUS_OFFSET]
    MOVS R12, #0x01          /* Set runCtr = 1 */
    STR R12, [R2, #RUNCNTR_OFFSET]
    LDR R2, [R2, #SP_OFFSET] /* Base address = &(runPtr->sp) */
    LDMIA R2!, {R4-R11}      /* POP R4-R11 from runPtr's saved stack */
    MSR PSP, R2              /* Update PSP with new stack pointer after LDMIA */
    MOV LR, #0xFFFFFFFD      /* Return to thread mode using PSP */
    DSB
    CPSIE I                  /* (End Critical Section) */
    ISB
    BX LR                    /* Dispatch first task */

