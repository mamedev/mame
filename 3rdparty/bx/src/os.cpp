/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/string.h>
#include <bx/os.h>
#include <bx/uint32_t.h>

#if BX_CRT_MSVC
#	include <direct.h>
#else
#	include <unistd.h>
#endif // BX_CRT_MSVC

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
#	include <windows.h>
#	include <psapi.h>
#elif  BX_PLATFORM_ANDROID    \
	|| BX_PLATFORM_BSD        \
	|| BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_HAIKU      \
	|| BX_PLATFORM_HURD       \
	|| BX_PLATFORM_IOS        \
	|| BX_PLATFORM_LINUX      \
	|| BX_PLATFORM_NX         \
	|| BX_PLATFORM_OSX        \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_RPI        \
	|| BX_PLATFORM_STEAMLINK
#	include <sched.h> // sched_yield
#	if BX_PLATFORM_BSD       \
	|| BX_PLATFORM_HAIKU     \
	|| BX_PLATFORM_IOS       \
	|| BX_PLATFORM_OSX       \
	|| BX_PLATFORM_PS4       \
	|| BX_PLATFORM_STEAMLINK
#		include <pthread.h> // mach_port_t
#	endif // BX_PLATFORM_*

#	include <time.h> // nanosleep
#	if !BX_PLATFORM_PS4
#		include <dlfcn.h> // dlopen, dlclose, dlsym
#	endif // !BX_PLATFORM_PS4

#	if BX_PLATFORM_ANDROID
#		include <malloc.h> // mallinfo
#	elif   BX_PLATFORM_LINUX     \
		|| BX_PLATFORM_RPI       \
		|| BX_PLATFORM_STEAMLINK
#		include <stdio.h>  // fopen
#		include <unistd.h> // syscall
#		include <sys/syscall.h>
#	elif   BX_PLATFORM_HAIKU
#		include <stdio.h>  // fopen
#		include <unistd.h> // syscall
#	elif BX_PLATFORM_OSX
#		include <mach/mach.h> // mach_task_basic_info
#	elif BX_PLATFORM_HURD
#		include <stdio.h>           // fopen
#		include <pthread/pthread.h> // pthread_self
#	elif BX_PLATFORM_ANDROID
#		include "debug.h" // getTid is not implemented...
#	endif // BX_PLATFORM_ANDROID
#endif // BX_PLATFORM_

namespace bx
{
	void sleep(uint32_t _ms)
	{
#if BX_PLATFORM_WINDOWS
		::Sleep(_ms);
#elif  BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT   \
	|| BX_CRT_NONE
		BX_UNUSED(_ms);
		debugOutput("sleep is not implemented"); debugBreak();
#else
		timespec req = { (time_t)_ms/1000, (long)( (_ms%1000)*1000000) };
		timespec rem = { 0, 0 };
		::nanosleep(&req, &rem);
#endif // BX_PLATFORM_
	}

	void yield()
	{
#if BX_PLATFORM_WINDOWS
		::SwitchToThread();
#elif  BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT   \
	|| BX_CRT_NONE
		debugOutput("yield is not implemented"); debugBreak();
#else
		::sched_yield();
#endif // BX_PLATFORM_
	}

	uint32_t getTid()
	{
#if BX_PLATFORM_WINDOWS
		return ::GetCurrentThreadId();
#elif  BX_PLATFORM_LINUX \
	|| BX_PLATFORM_RPI   \
	|| BX_PLATFORM_STEAMLINK
		return (pid_t)::syscall(SYS_gettid);
#elif  BX_PLATFORM_IOS \
	|| BX_PLATFORM_OSX
		return (mach_port_t)::pthread_mach_thread_np(pthread_self() );
#elif BX_PLATFORM_BSD
		return *(uint32_t*)::pthread_self();
#elif BX_PLATFORM_HURD
		return (pthread_t)::pthread_self();
#else
		debugOutput("getTid is not implemented"); debugBreak();
		return 0;
#endif // BX_PLATFORM_
	}

	size_t getProcessMemoryUsed()
	{
#if BX_PLATFORM_ANDROID
		struct mallinfo mi = mallinfo();
		return mi.uordblks;
#elif  BX_PLATFORM_LINUX \
	|| BX_PLATFORM_HURD
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
#	if defined(MACH_TASK_BASIC_INFO)
		mach_task_basic_info info;
		mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

		int const result = task_info(mach_task_self()
				, MACH_TASK_BASIC_INFO
				, (task_info_t)&info
				, &infoCount
				);
#	else
		task_basic_info info;
		mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;

		int const result = task_info(mach_task_self()
				, TASK_BASIC_INFO
				, (task_info_t)&info
				, &infoCount
				);
#	endif // defined(MACH_TASK_BASIC_INFO)
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

	void* dlopen(const FilePath& _filePath)
	{
#if BX_PLATFORM_WINDOWS
		return (void*)::LoadLibraryA(_filePath.getCPtr() );
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_XBOXONE    \
	|| BX_PLATFORM_WINRT      \
	|| BX_CRT_NONE
		BX_UNUSED(_filePath);
		return NULL;
#else
		return ::dlopen(_filePath.getCPtr(), RTLD_LOCAL|RTLD_LAZY);
#endif // BX_PLATFORM_
	}

	void dlclose(void* _handle)
	{
#if BX_PLATFORM_WINDOWS
		::FreeLibrary( (HMODULE)_handle);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_XBOXONE    \
	|| BX_PLATFORM_WINRT      \
	|| BX_CRT_NONE
		BX_UNUSED(_handle);
#else
		::dlclose(_handle);
#endif // BX_PLATFORM_
	}

	void* dlsym(void* _handle, const StringView& _symbol)
	{
		const int32_t symbolMax = _symbol.getLength()+1;
		char* symbol = (char*)alloca(symbolMax);
		bx::strCopy(symbol, symbolMax, _symbol);

#if BX_PLATFORM_WINDOWS
		return (void*)::GetProcAddress( (HMODULE)_handle, symbol);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_XBOXONE    \
	|| BX_PLATFORM_WINRT      \
	|| BX_CRT_NONE
		BX_UNUSED(_handle, symbol);
		return NULL;
#else
		return ::dlsym(_handle, symbol);
#endif // BX_PLATFORM_
	}

	bool getEnv(char* _out, uint32_t* _inOutSize, const StringView& _name)
	{
		const int32_t nameMax = _name.getLength()+1;
		char* name = (char*)alloca(nameMax);
		bx::strCopy(name, nameMax, _name);

#if BX_PLATFORM_WINDOWS
		DWORD len = ::GetEnvironmentVariableA(name, _out, *_inOutSize);
		bool result = len != 0 && len < *_inOutSize;
		*_inOutSize = len;
		return result;
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_XBOXONE    \
	|| BX_PLATFORM_WINRT      \
	|| BX_CRT_NONE
		BX_UNUSED(name, _out, _inOutSize);
		return false;
#else
		const char* ptr = ::getenv(name);
		uint32_t len = 0;
		bool result = false;
		if (NULL != ptr)
		{
			len = (uint32_t)strLen(ptr);

			result = len != 0 && len < *_inOutSize;
			if (len < *_inOutSize)
			{
				strCopy(_out, *_inOutSize, ptr);
			}
		}

		*_inOutSize = len;
		return result;
#endif // BX_PLATFORM_
	}

	void setEnv(const StringView& _name, const StringView& _value)
	{
		const int32_t nameMax = _name.getLength()+1;
		char* name = (char*)alloca(nameMax);
		bx::strCopy(name, nameMax, _name);

		char* value = NULL;
		if (!_value.isEmpty() )
		{
			int32_t valueMax = _value.getLength()+1;
			value = (char*)alloca(valueMax);
			bx::strCopy(value, valueMax, _value);
		}

#if BX_PLATFORM_WINDOWS
		::SetEnvironmentVariableA(name, value);
#elif  BX_PLATFORM_EMSCRIPTEN \
	|| BX_PLATFORM_PS4        \
	|| BX_PLATFORM_XBOXONE    \
	|| BX_PLATFORM_WINRT      \
	|| BX_CRT_NONE
		BX_UNUSED(name, value);
#else
		if (NULL != value)
		{
			::setenv(name, value, 1);
		}
		else
		{
			::unsetenv(name);
		}
#endif // BX_PLATFORM_
	}

	int chdir(const char* _path)
	{
#if BX_PLATFORM_PS4     \
 || BX_PLATFORM_XBOXONE \
 || BX_PLATFORM_WINRT   \
 || BX_CRT_NONE
		BX_UNUSED(_path);
		return -1;
#elif BX_CRT_MSVC
		return ::_chdir(_path);
#else
		return ::chdir(_path);
#endif // BX_COMPILER_
	}

	void* exec(const char* const* _argv)
	{
#if BX_PLATFORM_LINUX \
 || BX_PLATFORM_HURD
		pid_t pid = fork();

		if (0 == pid)
		{
			int result = execvp(_argv[0], const_cast<char *const*>(&_argv[1]) );
			BX_UNUSED(result);
			return NULL;
		}

		return (void*)uintptr_t(pid);
#elif BX_PLATFORM_WINDOWS
		STARTUPINFOA si;
		memSet(&si, 0, sizeof(STARTUPINFOA) );
		si.cb = sizeof(STARTUPINFOA);

		PROCESS_INFORMATION pi;
		memSet(&pi, 0, sizeof(PROCESS_INFORMATION) );

		int32_t total = 0;
		for (uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			total += (int32_t)strLen(_argv[ii]) + 1;
		}

		char* temp = (char*)alloca(total);
		int32_t len = 0;
		for(uint32_t ii = 0; NULL != _argv[ii]; ++ii)
		{
			len += snprintf(&temp[len], bx::uint32_imax(0, total-len)
				, "%s "
				, _argv[ii]
				);
		}

		bool ok = !!CreateProcessA(_argv[0]
			, temp
			, NULL
			, NULL
			, false
			, 0
			, NULL
			, NULL
			, &si
			, &pi
			);
		if (ok)
		{
			return pi.hProcess;
		}

		return NULL;
#else
		BX_UNUSED(_argv);
		return NULL;
#endif // BX_PLATFORM_LINUX || BX_PLATFORM_HURD
	}

} // namespace bx
