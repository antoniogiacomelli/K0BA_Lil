/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 ******************************************************************************
 ******************************************************************************
 *  Module            : Version
 *  Provides to       : All
 *  Public API        : Yes
 *
 *  It is a good practice to keep a firmware version on a dedicated
 *  translation unit.
 *
 *  In this unit:
 *      o Kernel Version record and retrieval
 *
 *****************************************************************************/

#include "kversion.h"

/* no file system, no NVM map, this is the best we can do */
struct kversion const KVERSION =
{ 0, 3, 1 };

unsigned int kGetVersion( void)
{
	unsigned int version = (KVERSION.major << 16 | KVERSION.minor << 8
			| KVERSION.patch << 0);
	return (version);
}

unsigned int kIsValidVersion( void)
{
	return (kGetVersion() == K_VALID_VERSION );
}
