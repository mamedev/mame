/*
empty
*/
//============================================================
//
//  osinline.h - GNU C inline functions
//
//============================================================

#ifndef __OSINLINE__
#define __OSINLINE__

//============================================================
//  INLINE FUNCTIONS
//============================================================

#if defined(__i386__) || defined(__x86_64__)
#define osd_yield_processor() __asm__ __volatile__ ( " rep ; nop ;" )
#if defined(__x86_64__)
#define _osd_exchange64(ptr, exchange) (register INT64 ret; __asm__ __volatile__ (" lock ; xchg %[exchange], %[ptr] ;": [ptr]      "+m" (*ptr), [ret]      "=r" (ret): [exchange] "1"  (exchange)); ret)
#define osd_exchange64 _osd_exchange64
#endif /* __x86_64__ */
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
#define osd_yield_pocessor() __asm__ __volatile__ ( " nop \n nop \n" )
#if defined(__ppc64__) || defined(__PPC64__)
#define _osd_exchange64(ptr, exchange) (register INT64 ret; __asm__ __volatile__ ("1: ldarx  %[ret], 0, %[ptr]      \n""   stdcx. %[exchange], 0, %[ptr] \n""   bne-   1b                     \n": [ret]      "=&r" (ret): [ptr]      "r"   (ptr), [exchange] "r"   (exchange): "cr0"); ret)
#define osd_exchange64 _osd_exchange64
#endif /* __ppc64__ || __PPC64__ */
#else
#ifndef YieldProcessor
#define YieldProcessor() do {} while (0)
#define osd_yield_processor() YieldProcessor()
#endif
#endif

#ifndef RETRO_AND
#define mul_32x32(a, b) ((INT64)a * (INT64)b)
#define mulu_32x32(a, b) ((UINT64)a * (UINT64)b)
#define mul_32x32_hi(a, b) ((UINT32)(((INT64)a * (INT64)b) >> 32))
#define mul_32x32_shift(a, b, shift) ((INT32)(((INT64)a * (INT64)b) >> shift))
#define mulu_32x32_shift(a, b, shift) ((UINT32)(((UINT64)a * (UINT64)b) >> shift))
#define div_64x32(a, b) (a / (INT64)b)
#define divu_64x32(a, b) (a / (UINT64)b)

#define div_32x32_shift(a, b, shift) (((INT64)a << shift) / (INT64)b)
#define divu_32x32_shift(a, b, shift) (((UINT64)a << shift) / (UINT64)b)
#define mod_64x32(a, b) (a - (b * div_64x32(a, b)))
#define modu_64x32(a, b) (a - (b * divu_64x32(a, b)))
#define recip_approx(value) (1.0f / value)

#define atomic_add32(ptr, delta) ((*ptr += delta))
#define atomic_increment32(ptr) (atomic_add32(ptr, 1))
#define atomic_decrement32(ptr) (atomic_add32(ptr, -1))
#define get_profile_ticks() (osd_ticks())
#endif

#include "eminline.h"


#endif /* __OSINLINE__ */
