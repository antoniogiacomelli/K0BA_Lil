/******************************************************************************
 *
 * [K0BA - Kernel 0 For Embedded Applications] | [VERSION: 0.3.1]
 *
 ******************************************************************************
 ******************************************************************************
 * 	Module          : Error Handler
 * 	Depends on      : Environment
 * 	Provides to     : All
 *  Public API      : No
 * 	In this unit:
 * 			   Error Handling
 *
 ******************************************************************************/

#include "kexecutive.h"

/*** Compile time errors */

#if !(CUSTOM_ENV)
#error "You need to custom your application environment and change CUSTOM_ENV\
         to (1)"
#endif

#ifndef __GNUC__
#   error "You need GCC as your compiler!"
#endif

#ifndef K_DEF_MINIMAL_VER
#error "Missing K0BA version"
#endif

#if (K_DEF_MIN_PRIO > 31)
#	error "Invalid minimal effective priority. (Max numerical value: 31)"
#endif

#if (K_DEF_N_TIMERS < K_DEF_N_USRTASKS+1)
#	error "Invalid number of application timers. Minimal is the number of user tasks + 1"
#endif


/******************************************************************************
 * ERROR HANDLING
 ******************************************************************************/
volatile K_FAULT faultID = 0;

/*police line do not cross*/
void kErrHandler(K_FAULT fault) /* generic error handler */
{
    /*TODO: before using   */
    /*#ifdef NDEBUG, guarantee these faults are returning correctly  */
    faultID = fault;
    asm volatile ("cpsid i" : : : "memory");
    while (1)
        ;
}
