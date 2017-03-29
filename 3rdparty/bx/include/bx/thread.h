/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_THREAD_H_HEADER_GUARD
#define BX_THREAD_H_HEADER_GUARD

#include "bx.h"

#if BX_PLATFORM_POSIX
#	include <pthread.h>
#	if defined(__FreeBSD__)
#		include <pthread_np.h>
#	endif
#	if BX_PLATFORM_LINUX && (BX_CRT_GLIBC < 21200)
#		include <sys/prctl.h>
#	endif // BX_PLATFORM_
#elif BX_PLATFORM_WINRT
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
#endif // BX_PLATFORM_

#include "sem.h"

#if BX_CONFIG_SUPPORTS_THREADING

namespace bx
{
	///
	typedef int32_t (*ThreadFn)(void* _userData);

	///
	class Thread
	{
		BX_CLASS(Thread
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		Thread();

		///
		virtual ~Thread();

		///
		void init(ThreadFn _fn, void* _userData = NULL, uint32_t _stackSize = 0, const char* _name = NULL);

		///
		void shutdown();

		///
		bool isRunning() const;

		///
		int32_t getExitCode() const;

		///
		void setThreadName(const char* _name);

	private:
		int32_t entry();

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		static DWORD WINAPI threadFunc(LPVOID _arg);
		HANDLE m_handle;
		DWORD  m_threadId;
#elif BX_PLATFORM_POSIX
		static void* threadFunc(void* _arg);
		pthread_t m_handle;
#endif // BX_PLATFORM_

		ThreadFn  m_fn;
		void*     m_userData;
		Semaphore m_sem;
		uint32_t  m_stackSize;
		int32_t   m_exitCode;
		bool      m_running;
	};

	///
	class TlsData
	{
	public:
		///
		TlsData();

		///
		~TlsData();

		///
		void* get() const;

		///
		void set(void* _ptr);

	private:
#if BX_PLATFORM_WINDOWS
		uint32_t m_id;
#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)
		pthread_key_t m_id;
#endif // BX_PLATFORM_*
	};

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING

#include "thread.inl"

#endif // BX_THREAD_H_HEADER_GUARD
