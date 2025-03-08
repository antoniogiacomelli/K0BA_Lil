/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.4.0]]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module            : Low-level Scheduler/Start-Up.
 *  Provides to       : High-level Scheduler / Main application.
 *  Public API        : No
 *
 *  o In this unit:
 *                Low-level kernel routines.
 *
 *****************************************************************************/


/*@file klow.s */
.syntax unified /* thumb2 */
.text
.align 4

/* task status values */
.equ READY,   0x01
.equ RUNNING, 0x02

/* armv7m reg addresses */
.equ SCB_ICSR,   0xE000ED04 /* base icsr */
.equ STICK_CTRL, 0xE000E010 /* systick csr */

/* arvm7m reg values */
.equ ISCR_SETPSV, (1 << 28)
.equ ISCR_CLRPSV, (1 << 27)
.equ STICK_CLRP, (1 << 30)
.equ STICK_ON,  0x0F
.equ STICK_OFF, 0x00

/* kernel specifics  */
.equ APP_START_UP_IMM,   0xAA
.equ USR_CTXT_SWTCH,     0xC5
.equ FAULT_SVC, 0xFF
.equ SP_OFFSET, 0
.equ STATUS_OFFSET, 4
.equ RUNCNTR_OFFSET, 8

.global __getReadyPrio
.type __getReadyPrio, %function
.thumb_func
__getReadyPrio:
CLZ  R12, R0
MOV  R0, R12
NEG  R0, R0
ADD  R0, #31
BX LR


.global kApplicationInit
.type kApplicationInit, %function
.thumb_func

.global kErrHandler  /* defined in kerr.c */
.type kErrHandler, %function
.thumb_func

.global SVC_Handler
.type SVC_Handler, %function
.thumb_func
SVC_Handler:       /* we start-up from msp               */
    TST LR, #4     /* for MSP, LR here shall end with a x9               */
    BNE ERR
    MRS R12, MSP
    LDR R1, [R12, #24] /* <-  the instruction that brought us here       */
    LDRB R12, [R1,#-2]
    CMP R12, #APP_START_UP_IMM
    BEQ USRSTARTUP
    ERR:
    MOV R0, #FAULT_SVC
    BL  kErrHandler

/* start user application */
.global USRSTARTUP
.type USRSTARTUP, %function
.thumb_func
 USRSTARTUP:                  /* get camera-ready:                        */
    CPSID I
    MOVS R0, #2               /* 0b10 = PSP, PRIV                         */
    MSR CONTROL, R0
    DSB
    LDR R1, =runPtr
    LDR R2, [R1, #SP_OFFSET]
    MOVS R12, #RUNNING
    STR R12, [R2, #STATUS_OFFSET]
    MOVS R12, #0x01
    STR R12, [R2, #RUNCNTR_OFFSET]
    LDR R2, [R2, #SP_OFFSET] /* base address == &(runPtr->sp)              */
    LDMIA R2!, {R4-R11}      /* 'POP' R4-R11 as    assembled on kInitTcb_  */
    MSR PSP, R2              /* update PSP after 'popping '                */
    MOV LR, #0xFFFFFFFD      /* set LR to indicate we choose PSP          */
    LDR R0, =STICK_CTRL
    MOVS R1, #STICK_ON       /* tick on                                    */
    STR R1, [R0]
    DSB
    CPSIE I
    ISB
    BX LR                    /*...running, running, running...             */

.global SysTick_Handler
.type SysTick_Handler, %function
.thumb_func
SysTick_Handler:
    CPSID I
    PUSH {R0, LR}
    TICKHANDLER:
    BL kTickHandler        /* always run kTickHandler, result in R0        */
    CMP R0, #1
    BEQ SWITCH
    POP {R0, LR}
    CMP LR, 0xFFFFFFF1
    BEQ RESUMEISR
    RESUMETASK:
    MOV LR, #0xFFFFFFFD
    DSB
    CPSIE I
    ISB
    BX LR
    RESUMEISR:
    CPSIE I
    ISB
    BX LR
    SWITCH:
    POP {R0, LR}
    B SWITCHTASK

/* deferred context switching */
.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func
PendSV_Handler:
    SWITCHTASK:
    MRS R12, PSP
    STMDB R12!, {R4-R11}
    MSR PSP, R12
    LDR R0, =runPtr
    LDR R1, [R0]
    STR R12, [R1]
    DSB
    BL kSchSwtch
    LDR R0, =runPtr
    LDR R1, [R0]
    LDR R3, [R1, #RUNCNTR_OFFSET]
    ADDS R3, #1
    STR R3, [R1, #RUNCNTR_OFFSET]
    MOVS R12, #2
    STR R12, [R1, #STATUS_OFFSET]
    LDR R2, [R1]
    LDMIA R2!, {R4-R11}
    MSR PSP, R2
    MOV LR, #0xFFFFFFFD
    DSB
    CPSIE I
    ISB
    BX LR

