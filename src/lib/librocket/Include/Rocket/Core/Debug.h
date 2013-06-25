/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREDEBUG_H
#define ROCKETCOREDEBUG_H

#include <Rocket/Core/Header.h>

// Define for breakpointing.
#if defined (ROCKET_PLATFORM_WIN32)
	#if defined (__MINGW32__)
		#define ROCKET_BREAK asm("int $0x03")
	#else
		#define ROCKET_BREAK _asm { int 0x03 }
	#endif
#elif defined (ROCKET_PLATFORM_LINUX)
	#if defined __i386__ || defined __x86_64__
		#define ROCKET_BREAK asm ("int $0x03" )
	#else
		#define ROCKET_BREAK
	#endif
#elif defined (ROCKET_PLATFORM_MACOSX)
	#include <TargetConditionals.h>

	#if TARGET_OS_IPHONE
		#define ROCKET_BREAK
	#else
		#define ROCKET_BREAK {__asm__("int $3\n" : : );}
	#endif
#endif



// Define the LT_ASSERT and ROCKET_VERIFY macros.
#if !defined ROCKET_DEBUG
#define ROCKET_ASSERT(x)
#define ROCKET_ASSERTMSG(x, m)
#define ROCKET_ERROR
#define ROCKET_ERRORMSG(m)
#define ROCKET_VERIFY(x) x
#else
namespace Rocket {
namespace Core {

bool ROCKETCORE_API Assert(const char* message, const char* file, int line);
#define ROCKET_ASSERT(x) \
if (!(x)) \
{ \
	if (!Rocket::Core::Assert("ROCKET_ASSERT("#x")", __FILE__, __LINE__ )) \
	{ \
		ROCKET_BREAK; \
	} \
}
#define ROCKET_ASSERTMSG(x, m)	\
if (!(x)) \
{ \
	if (!Rocket::Core::Assert(m, __FILE__, __LINE__ )) \
	{ \
		ROCKET_BREAK; \
	} \
}
#define ROCKET_ERROR \
if (!Rocket::Core::Assert("ROCKET_ERROR", __FILE__, __LINE__)) \
{ \
	ROCKET_BREAK; \
}
#define ROCKET_ERRORMSG(m) \
if (!Rocket::Core::Assert(m, __FILE__, __LINE__)) \
{ \
	ROCKET_BREAK; \
}
#define ROCKET_VERIFY(x) ROCKET_ASSERT(x)

}
}
#endif

namespace Rocket {
namespace Core {

template <bool> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true>{};	
	
}
}
#define ROCKET_STATIC_ASSERT(cond, msg) Rocket::Core::STATIC_ASSERTION_FAILURE<cond> msg

#endif
