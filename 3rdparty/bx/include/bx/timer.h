/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_TIMER_H_HEADER_GUARD
#define BX_TIMER_H_HEADER_GUARD

#include "bx.h"

#if BX_PLATFORM_ANDROID
#	include <time.h> // clock, clock_gettime
#elif BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#elif BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
#	include <windows.h>
#else
#	include <sys/time.h> // gettimeofday
#endif // BX_PLATFORM_

namespace bx
{
	inline int64_t getHPCounter()
	{
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		LARGE_INTEGER li;
		// Performance counter value may unexpectedly leap forward
		// http://support.microsoft.com/kb/274323
		QueryPerformanceCounter(&li);
		int64_t i64 = li.QuadPart;
#elif BX_PLATFORM_ANDROID
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t i64 = now.tv_sec*INT64_C(1000000000) + now.tv_nsec;
#elif BX_PLATFORM_EMSCRIPTEN
		int64_t i64 = int64_t(1000.0f * emscripten_get_now() );
#else
		struct timeval now;
		gettimeofday(&now, 0);
		int64_t i64 = now.tv_sec*INT64_C(1000000) + now.tv_usec;
#endif // BX_PLATFORM_
		return i64;
	}

	inline int64_t getHPFrequency()
	{
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		return li.QuadPart;
#elif BX_PLATFORM_ANDROID
		return INT64_C(1000000000);
#elif BX_PLATFORM_EMSCRIPTEN
		return INT64_C(1000000);
#else
		return INT64_C(1000000);
#endif // BX_PLATFORM_
	}

} // namespace bx

#endif // BX_TIMER_H_HEADER_GUARD
