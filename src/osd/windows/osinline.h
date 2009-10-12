//============================================================
//
//  osinline.h
//
//  Inline implementations for non-GCC Win32 compilers
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
