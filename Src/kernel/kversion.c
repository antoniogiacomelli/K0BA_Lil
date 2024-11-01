/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this unit:
 * 		o Returns Kernel Version
 *
 *****************************************************************************/

#include "ksys.h"

#ifdef K_DEF_VERSION
unsigned int kGetVersion(void)
{
	unsigned int version = (KVERSION.major << 16 | KVERSION.minor << 8 | KVERSION.patch << 0);
	return  version;
}
#endif
