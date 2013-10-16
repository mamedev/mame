// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  osinline.h
//
//  Inline implementations for non-GCC Win32 compilers
//
//============================================================

#ifndef __OSINLINE__
#define __OSINLINE__

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#include "eivcx86.h"
#endif

#if defined(_MSC_VER)
#include "eivc.h"
#endif

INT32 win_compare_exchange32(INT32 volatile *ptr, INT32 compare, INT32 exchange);
INT32 win_atomic_exchange32(INT32 volatile *ptr, INT32 exchange);
INT32 win_atomic_add32(INT32 volatile *ptr, INT32 delta);

#ifdef PTR64
INT64 win_compare_exchange64(INT64 volatile *ptr, INT64 compare, INT64 exchange);
#endif


#ifndef compare_exchange32
#define compare_exchange32 win_compare_exchange32
#endif /* compare_exchange32 */

#ifdef PTR64
#ifndef compare_exchange64
#define compare_exchange64 win_compare_exchange64
#endif /* compare_exchange64 */
#endif

#ifndef atomic_exchange32
#define atomic_exchange32 win_atomic_exchange32
#endif /* atomic_exchange32 */


#ifndef atomic_add32
#define atomic_add32 win_atomic_add32
#endif /* atomic_add32 */


#endif /* __OSINLINE__ */
