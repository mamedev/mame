//============================================================
//
//  eivc.h
//
//  Inline implementations for MSVC compiler.
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#ifndef __EIVC__
#define __EIVC__

#if (_MSC_VER >= 1400)

// need to ignore 'nonstandard extension used' warning in setjmp.h
#pragma warning(push)
#pragma warning(disable: 4987)
#include <intrin.h>
#pragma warning(pop)

#else
extern "C" long __cdecl _InterlockedIncrement(long volatile *);
extern "C" long __cdecl _InterlockedDecrement(long volatile *);
extern "C" long _InterlockedExchange(long volatile *, long);
extern "C" long _InterlockedCompareExchange (long volatile *, long, long);
extern "C" long _InterlockedExchangeAdd(long volatile *, long);
extern "C" unsigned char _BitScanReverse(unsigned long *Index, unsigned long Mask);
#endif

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#if (_MSC_VER >= 1310)
#pragma intrinsic(_BitScanReverse)
#endif


/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
#define count_leading_zeros _count_leading_zeros
INLINE UINT8 _count_leading_zeros(UINT32 value)
{
	UINT32 index;
	return _BitScanReverse((unsigned long *)&index, value) ? (index ^ 31) : 32;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
#define count_leading_ones _count_leading_ones
INLINE UINT8 _count_leading_ones(UINT32 value)
{
	UINT32 index;
	return _BitScanReverse((unsigned long *)&index, ~value) ? (index ^ 31) : 32;
}
#endif



/***************************************************************************
    INLINE SYNCHRONIZATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    compare_exchange32 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#ifndef compare_exchange32
#define compare_exchange32 _compare_exchange32
INLINE INT32 _compare_exchange32(INT32 volatile *ptr, INT32 compare, INT32 exchange)
{
	return _InterlockedCompareExchange((volatile long *)ptr, exchange, compare);
}
#endif


/*-------------------------------------------------
    compare_exchange64 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#ifdef PTR64
#ifndef compare_exchange64
#define compare_exchange64 _compare_exchange64
INLINE INT64 _compare_exchange64(INT64 volatile *ptr, INT64 compare, INT64 exchange)
{
	return _InterlockedCompareExchange64(ptr, exchange, compare);
}
#endif
#endif


/*-------------------------------------------------
    atomic_exchange32 - atomically exchange the
    exchange value with the memory at 'ptr',
    returning the original value.
-------------------------------------------------*/

#ifndef atomic_exchange32
#define atomic_exchange32 _atomic_exchange32
INLINE INT32 _atomic_exchange32(INT32 volatile *ptr, INT32 exchange)
{
	return _InterlockedExchange((volatile long *)ptr, exchange);
}
#endif


/*-------------------------------------------------
    atomic_add32 - atomically add the delta value
    to the memory at 'ptr', returning the final
    result.
-------------------------------------------------*/

#ifndef atomic_add32
#define atomic_add32 _atomic_add32
INLINE INT32 _atomic_add32(INT32 volatile *ptr, INT32 delta)
{
	return _InterlockedExchangeAdd((volatile long *)ptr, delta) + delta;
}
#endif


/*-------------------------------------------------
    atomic_increment32 - atomically increment the
    32-bit value in memory at 'ptr', returning the
    final result.
-------------------------------------------------*/

#ifndef atomic_increment32
#define atomic_increment32 _atomic_increment32
INLINE INT32 _atomic_increment32(INT32 volatile *ptr)
{
	return _InterlockedIncrement((volatile long *)ptr);
}
#endif


/*-------------------------------------------------
    atomic_decrement32 - atomically decrement the
    32-bit value in memory at 'ptr', returning the
    final result.
-------------------------------------------------*/

#ifndef atomic_decrement32
#define atomic_decrement32 _atomic_decrement32
INLINE INT32 _atomic_decrement32(INT32 volatile *ptr)
{
	return _InterlockedDecrement((volatile long *)ptr);
}
#endif


#endif /* __EIVC__ */
