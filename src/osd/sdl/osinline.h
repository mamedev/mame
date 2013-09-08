//============================================================
//
//  osinline.h - GNU C inline functions
//
//============================================================

#ifndef __OSINLINE__
#define __OSINLINE__

#include "eminline.h"


//============================================================
//  INLINE FUNCTIONS
//============================================================

#if defined(__i386__) || defined(__x86_64__)


INLINE void ATTR_FORCE_INLINE
osd_yield_processor(void)
{
	__asm__ __volatile__ ( " rep ; nop ;" );
}


#if defined(__x86_64__)

//============================================================
//  osd_exchange64
//============================================================

INLINE INT64 ATTR_UNUSED ATTR_NONNULL(1) ATTR_FORCE_INLINE
_osd_exchange64(INT64 volatile *ptr, INT64 exchange)
{
	register INT64 ret;
	__asm__ __volatile__ (
		" lock ; xchg %[exchange], %[ptr] ;"
		: [ptr]      "+m" (*ptr)
		, [ret]      "=r" (ret)
		: [exchange] "1"  (exchange)
	);
	return ret;
}
#define osd_exchange64 _osd_exchange64

#endif /* __x86_64__ */


#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)


INLINE void ATTR_FORCE_INLINE
osd_yield_processor(void)
{
	__asm__ __volatile__ ( " nop \n nop \n" );
}



#if defined(__ppc64__) || defined(__PPC64__)

//============================================================
//  osd_exchange64
//============================================================

INLINE INT64 ATTR_UNUSED ATTR_NONNULL(1) ATTR_FORCE_INLINE
_osd_exchange64(INT64 volatile *ptr, INT64 exchange)
{
	register INT64 ret;
	__asm__ __volatile__ (
		"1: ldarx  %[ret], 0, %[ptr]      \n"
		"   stdcx. %[exchange], 0, %[ptr] \n"
		"   bne-   1b                     \n"
		: [ret]      "=&r" (ret)
		: [ptr]      "r"   (ptr)
		, [exchange] "r"   (exchange)
		: "cr0"
	);
	return ret;
}
#define osd_exchange64 _osd_exchange64

#endif /* __ppc64__ || __PPC64__ */

#else

#error "no matching assembler implementations found - please compile with NOASM=1"

#endif

#endif /* __OSINLINE__ */
