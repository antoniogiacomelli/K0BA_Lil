/******************************************************************************
 *
 *     [[K0BA - Kernel 0 For Embedded Applications] | [VERSION: 1.1.0]]
 *
 ******************************************************************************
 ******************************************************************************
 * 	In this header:
 * 					o Kernel Version record definition
 * 					xx.xx.xx
 * 					major minor patch
 *
 *****************************************************************************/
#ifndef KVERSION_H
#define KVERSION_H
#define K_VALID_VERSION 0x010100
struct kversion
{
	unsigned char major;
	unsigned char minor;
	unsigned char patch;
};

unsigned int kGetVersion(void);

#endif /* KVERSION_H */
