/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module       : Version
 *  Depends on   : System Singletons
 *  Provides to  : All
 *  Public API   : Yes
 *
 * 	It is a good practice to keep a firmware version on a dedicated
 * 	translation unit.
 *
 *	In this unit:
 * 		o Kernel Version record and retrieval
 *
 *****************************************************************************/

#include "kversion.h"
#ifndef K_DEF_VERSION
#define K_DEF_VERSION
struct kversion KVERSION = { 1, 1, 0 };
#endif
unsigned int kGetVersion(void)
{
	unsigned int version = (KVERSION.major << 16 | KVERSION.minor << 8
			| KVERSION.patch << 0);
	return version;
}
