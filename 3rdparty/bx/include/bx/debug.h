/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_DEBUG_H_HEADER_GUARD
#define BX_DEBUG_H_HEADER_GUARD

#include "bx.h"

#if BX_PLATFORM_ANDROID
#	include <android/log.h>
#elif BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT || BX_PLATFORM_XBOX360
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
#	if defined(__OBJC__)
#		import <Foundation/NSObjCRuntime.h>
#	else
#		include <CoreFoundation/CFString.h>
extern "C" void NSLog(CFStringRef _format, ...);
#	endif // defined(__OBJC__)
#elif 0 // BX_PLATFORM_EMSCRIPTEN
#	include <emscripten.h>
#else
#	include <stdio.h>
#endif // BX_PLATFORM_WINDOWS

namespace bx
{
#if BX_COMPILER_CLANG_ANALYZER
	inline __attribute__((analyzer_noreturn)) void debugBreak();
#endif // BX_COMPILER_CLANG_ANALYZER

	inline void debugBreak()
	{
#if BX_COMPILER_MSVC
		__debugbreak();
#elif BX_CPU_ARM
		__builtin_trap();
//		asm("bkpt 0");
#elif !BX_PLATFORM_NACL && BX_CPU_X86 && (BX_COMPILER_GCC || BX_COMPILER_CLANG)
		// NaCl doesn't like int 3:
		// NativeClient: NaCl module load failed: Validation failure. File violates Native Client safety rules.
		__asm__ ("int $3");
#else // cross platform implementation
		int* int3 = (int*)3L;
		*int3 = 3;
#endif // BX
	}

	inline void debugOutput(const char* _out)
	{
#if BX_PLATFORM_ANDROID
#	ifndef BX_ANDROID_LOG_TAG
#		define BX_ANDROID_LOG_TAG ""
#	endif // BX_ANDROID_LOG_TAG
		__android_log_write(ANDROID_LOG_DEBUG, BX_ANDROID_LOG_TAG, _out);
#elif BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT || BX_PLATFORM_XBOX360
		OutputDebugStringA(_out);
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
#	if defined(__OBJC__)
		NSLog(@"%s", _out);
#	else
		NSLog(__CFStringMakeConstantString("%s"), _out);
#	endif // defined(__OBJC__)
#elif 0 // BX_PLATFORM_EMSCRIPTEN
		emscripten_log(EM_LOG_CONSOLE, "%s", _out);
#else
		fputs(_out, stdout);
		fflush(stdout);
#endif // BX_PLATFORM_
	}

} // namespace bx

#endif // BX_DEBUG_H_HEADER_GUARD
