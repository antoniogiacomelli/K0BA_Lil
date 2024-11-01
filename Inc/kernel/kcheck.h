/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Compile-time error checks
 *
 *****************************************************************************/

#ifndef KCHECK_H
#define KCHECK_H

#ifndef __GNUC__
#   error "You need GCC as your compiler!"
#endif

#if (K_DEF_MESGQ == ON)
#if (K_DEF_N_MESGBUFF <=0 )
#	error "Number of message buffers must be greater than 0"
#endif
#endif

#ifndef K_DEF_VERSION
#error "Mising K0BA version"
#endif

#if (!defined(_STDDEF_H) && !defined(_STDDEF_H_) && !defined(_ANSI_STDDEF_H) \
     && !defined(__STDDEF_H__))
#error "You need a compiler with stddef.h library."
#endif

#endif /* KCHECK_H */
