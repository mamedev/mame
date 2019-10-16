/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/timer.h>

#if BX_CRT_NONE
#	include "crt0.h"
#elif BX_PLATFORM_ANDROID
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
	int64_t getHPCounter()
	{
#if    BX_CRT_NONE
		int64_t i64 = crt0::getHPCounter();
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		int64_t i64 = li.QuadPart;
#elif BX_PLATFORM_ANDROID
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t i64 = now.tv_sec*INT64_C(1000000000) + now.tv_nsec;
#elif BX_PLATFORM_EMSCRIPTEN
		int64_t i64 = int64_t(1000.0f * emscripten_get_now() );
#elif !BX_PLATFORM_NONE
		struct timeval now;
		gettimeofday(&now, 0);
		int64_t i64 = now.tv_sec*INT64_C(1000000) + now.tv_usec;
#else
		BX_CHECK(false, "Not implemented!");
		int64_t i64 = UINT64_MAX;
#endif // BX_PLATFORM_
		return i64;
	}

	int64_t getHPFrequency()
	{
#if    BX_CRT_NONE
		return INT64_C(1000000000);
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
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
