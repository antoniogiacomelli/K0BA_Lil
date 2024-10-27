/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Private API: Error handling and Checks
 *
 *****************************************************************************/

#ifndef INC_KERR_H_
#define INC_KERR_H_

VOID ITM_SendValue(UINT32 value);
VOID kErrHandler(K_FAULT fault);



#endif /* INC_KERR_H_ */
