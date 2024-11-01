/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Private API: Error handling and Checks
 *
 *****************************************************************************/

#ifndef KERR_H
#define KERR_H

VOID ITM_SendValue(UINT32 value);
VOID kErrHandler(K_FAULT fault);

#endif /* K_ERR_H*/
