/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module: Nucleus
 * 	Sub-module: Version
 * 	It is a good practice to keep a firmware version retrieval on a dedicated
 * 	code unit.
 * 	In this unit:
 * 		o Returns Kernel Version
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
