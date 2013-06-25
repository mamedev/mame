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

#ifndef ROCKETCOREPLATFORM_H
#define ROCKETCOREPLATFORM_H

#if defined __WIN32__ || defined _WIN32
	#define ROCKET_PLATFORM_WIN32
	#define ROCKET_PLATFORM_NAME "win32"
	#if !defined(__MINGW32__)
		#pragma warning(disable:4355)
	#endif
#elif defined __APPLE_CC__
	#define ROCKET_PLATFORM_UNIX
	#define ROCKET_PLATFORM_MACOSX
	#define ROCKET_PLATFORM_NAME "macosx"
#else
	#define ROCKET_PLATFORM_UNIX
	#define ROCKET_PLATFORM_LINUX
	#define ROCKET_PLATFORM_NAME "linux"
#endif

#if !defined NDEBUG && !defined ROCKET_DEBUG
	#define ROCKET_DEBUG
#endif

#if defined __x86_64__ || defined _M_X64 || defined __powerpc64__ || defined __alpha__ || defined __ia64__ || defined __s390__ || defined __s390x__
    #define ROCKET_ARCH_64
#else
    #define ROCKET_ARCH_32
#endif


#if defined(ROCKET_PLATFORM_WIN32) && !defined(__MINGW32__)
	// alignment of a member was sensitive to packing
	#pragma warning(disable : 4121)

	// <type> needs to have dll-interface to be used by clients
	#pragma warning(disable : 4251)

	// assignment operator could not be generated
	#pragma warning(disable : 4512)

	// <function> was declared deprecated
	#pragma warning(disable : 4996)

	#if !defined _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE
	#endif
#endif

#define ROCKET_UNUSED(x)

#endif
