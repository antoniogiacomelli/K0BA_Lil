/************************************************************************
* @file		system.s
* @brief	Assembly routines for the K0BA Kernel
* @date		25/12/22
* @author	Antonio Giacomelli <antoniogiacomelli@protonmail.com>
*
** (C) Copyright 2022-23 www.antoniogiacomelli.com
*************************************************************************/

.syntax unified


.global SysTick_Handler
.type SysTick_Handler, %function
.global PendSV_Handler
.type PendSV_Handler, %function
.global kTaskSwitch
.type kTaskSwitch, %function

PendSV_Handler:
	PUSH	{R4-R11}  //push R4-R11
	LDR		R0,=RunPtr
	LDR		R1, [R0]
	STR		SP, [R1]
	B		Schedule

	Schedule:
	BL		kTaskSwitch
	B		Resume	    //resume kernel thread

	Resume:
	LDR		R1, =RunPtr			//R1 <- RunPtr updated
	LDR		R2, [R1]
	LDR		SP, [R2]
	POP		{R4-R11}
	MOV		LR, #0xFFFFFFF9
	BX		LR


.global kStart
.type kStart, %function
kStart:
	LDR		R0, =RunPtr
	LDR		R1, [R0]
	LDR		R2, [R1]
	MSR		MSP, R2			// MSP <- RunPtr.msp
	POP		{R4-R11}
	POP		{R0-R3}
	POP		{R12}
	ADD		SP, SP, #4
	POP		{LR}			//LR <- PC
	ADD		SP, SP, #4
	BX		LR


