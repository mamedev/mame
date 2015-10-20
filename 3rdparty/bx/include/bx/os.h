/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_OS_H_HEADER_GUARD
#define BX_OS_H_HEADER_GUARD

#include "bx.h"
#include "debug.h"

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
#	include <windows.h>
#	include <psapi.h>
#elif  BX_PLATFORM_ANDROID \
	|| BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_FREEBSD \
	|| BX_PLATFORM_IOS \
	|| BX_PLATFORM_LINUX \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_OSX \
	|| BX_PLATFORM_PS4 \
	|| BX_PLATFORM_RPI

#	include <sched.h> // sched_yield
#	if BX_PLATFORM_FREEBSD \
	|| BX_PLATFORM_IOS \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_OSX \
	|| BX_PLATFORM_PS4
#		include <pthread.h> // mach_port_t
#	endif // BX_PLATFORM_IOS || BX_PLATFORM_OSX || BX_PLATFORM_NACL

#	if BX_PLATFORM_NACL
#		include <sys/nacl_syscalls.h> // nanosleep
#	else
#		include <time.h> // nanosleep
#		if !BX_PLATFORM_PS4
#			include <dlfcn.h> // dlopen, dlclose, dlsym
#		endif // !BX_PLATFORM_PS4
#	endif // BX_PLATFORM_NACL

#	if BX_PLATFORM_ANDROID
#		include <malloc.h> // mallinfo
#	elif BX_PLATFORM_LINUX || BX_PLATFORM_RPI
#		include <unistd.h> // syscall
#		include <sys/syscall.h>
#	elif BX_PLATFORM_OSX
#		include <mach/mach.h> // mach_task_basic_info
#	elif BX_PLATFORM_ANDROID
#		include "debug.h" // getTid is not implemented...
#	endif // BX_PLATFORM_ANDROID
#endif // BX_PLATFORM_

#if BX_COMPILER_MSVC_COMPATIBLE
#	include <direct.h> // _getcwd
#else
#	include <unistd.h> // getcwd
#endif // BX_COMPILER_MSVC

#if BX_PLATFORM_OSX
#	define BX_DL_EXT "dylib"
#elif BX_PLATFORM_WINDOWS
#	define BX_DL_EXT "dll"
#else
#	define BX_DL_EXT "so"
#endif //

namespace bx
{
	inline void sleep(uint32_t _ms)
	{
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360
		::Sleep(_ms);
#elif BX_PLATFORM_WINRT
		BX_UNUSED(_ms);
		debugOutput("sleep is not implemented"); debugBreak();
#else
		timespec req = {(time_t)_ms/1000, (long)((_ms%1000)*1000000)};
		timespec rem = {0, 0};
		::nanosleep(&req, &rem);
#endif // BX_PLATFORM_
	}

	inline void yield()
	{
#if BX_PLATFORM_WINDOWS
		::SwitchToThread();
#elif BX_PLATFORM_XBOX360
		::Sleep(0);
#elif BX_PLATFORM_WINRT
		debugOutput("yield is not implemented"); debugBreak();
#else
		::sched_yield();
#endif // BX_PLATFORM_
	}

	inline uint32_t getTid()
	{
#if BX_PLATFORM_WINDOWS
		return ::GetCurrentThreadId();
#elif BX_PLATFORM_LINUX || BX_PLATFORM_RPI
		return (pid_t)::syscall(SYS_gettid);
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
		return (mach_port_t)::pthread_mach_thread_np(pthread_self() );
#elif BX_PLATFORM_FREEBSD || BX_PLATFORM_NACL
		// Casting __nc_basic_thread_data*... need better way to do this.
		return *(uint32_t*)::pthread_self();
#else
//#	pragma message "not implemented."
		debugOutput("getTid is not implemented"); debugBreak();
		return 0;
#endif //
	}

	inline size_t getProcessMemoryUsed()
	{
#if BX_PLATFORM_ANDROID
		struct mallinfo mi = mallinfo();
		return mi.uordblks;
#elif BX_PLATFORM_LINUX
		FILE* file = fopen("/proc/self/statm", "r");
		if (NULL == file)
		{
			return 0;
		}

		long pages = 0;
		int items = fscanf(file, "%*s%ld", &pages);
		fclose(file);
		return 1 == items
			? pages * sysconf(_SC_PAGESIZE)
			: 0
			;
#elif BX_PLATFORM_OSX
		mach_task_basic_info info;
		mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

		int result = task_info(mach_task_self()
				, MACH_TASK_BASIC_INFO
				, (task_info_t)&info
				, &infoCount
				);
		if (KERN_SUCCESS != result)
		{
			return 0;
		}

		return info.resident_size;
#elif BX_PLATFORM_WINDOWS
		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(GetCurrentProcess()
			, &pmc
			, sizeof(pmc)
			);
		return pmc.WorkingSetSize;
#else
		return 0;
#endif // BX_PLATFORM_*
	}

	inline void* dlopen(const char* _filePath)
	{
#if BX_PLATFORM_WINDOWS
		return (void*)::LoadLibraryA(_filePath);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_PS4 \
	|| BX_PLATFORM_WINRT
		BX_UNUSED(_filePath);
		return NULL;
#else
		return ::dlopen(_filePath, RTLD_LOCAL|RTLD_LAZY);
#endif // BX_PLATFORM_
	}

	inline void dlclose(void* _handle)
	{
#if BX_PLATFORM_WINDOWS
		::FreeLibrary( (HMODULE)_handle);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_PS4 \
	|| BX_PLATFORM_WINRT
		BX_UNUSED(_handle);
#else
		::dlclose(_handle);
#endif // BX_PLATFORM_
	}

	inline void* dlsym(void* _handle, const char* _symbol)
	{
#if BX_PLATFORM_WINDOWS
		return (void*)::GetProcAddress( (HMODULE)_handle, _symbol);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_PS4 \
	|| BX_PLATFORM_WINRT
		BX_UNUSED(_handle, _symbol);
		return NULL;
#else
		return ::dlsym(_handle, _symbol);
#endif // BX_PLATFORM_
	}

	inline void setenv(const char* _name, const char* _value)
	{
#if BX_PLATFORM_WINDOWS
		::SetEnvironmentVariableA(_name, _value);
#elif  BX_PLATFORM_PS4 \
	|| BX_PLATFORM_WINRT
		BX_UNUSED(_name, _value);
#else
		::setenv(_name, _value, 1);
#endif // BX_PLATFORM_
	}

	inline void unsetenv(const char* _name)
	{
#if BX_PLATFORM_WINDOWS
		::SetEnvironmentVariableA(_name, NULL);
#elif  BX_PLATFORM_PS4 \
	|| BX_PLATFORM_WINRT
		BX_UNUSED(_name);
#else
		::unsetenv(_name);
#endif // BX_PLATFORM_
	}

	inline int chdir(const char* _path)
	{
#if BX_PLATFORM_PS4 \
 || BX_PLATFORM_WINRT
		BX_UNUSED(_path);
		return -1;
#elif BX_COMPILER_MSVC_COMPATIBLE
		return ::_chdir(_path);
#else
		return ::chdir(_path);
#endif // BX_COMPILER_
	}

	inline char* pwd(char* _buffer, uint32_t _size)
	{
#if BX_PLATFORM_PS4 \
 || BX_PLATFORM_WINRT
		BX_UNUSED(_buffer, _size);
		return NULL;
#elif BX_COMPILER_MSVC_COMPATIBLE
		return ::_getcwd(_buffer, (int)_size);
#else
		return ::getcwd(_buffer, _size);
#endif // BX_COMPILER_
	}

} // namespace bx

#endif // BX_OS_H_HEADER_GUARD
