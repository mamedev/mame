/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_THREAD_H_HEADER_GUARD
#define BX_THREAD_H_HEADER_GUARD

#if BX_PLATFORM_POSIX
#	include <pthread.h>
#elif BX_PLATFORM_WINRT
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
#endif

#include "sem.h"

#if BX_CONFIG_SUPPORTS_THREADING

namespace bx
{
	typedef int32_t (*ThreadFn)(void* _userData);

	class Thread
	{
		BX_CLASS(Thread
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		Thread()
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_WINRT
			: m_handle(INVALID_HANDLE_VALUE)
			, m_threadId(UINT32_MAX)
#elif BX_PLATFORM_POSIX
			: m_handle(0)
#endif // BX_PLATFORM_
			, m_fn(NULL)
			, m_userData(NULL)
			, m_stackSize(0)
			, m_exitCode(0 /*EXIT_SUCCESS*/)
			, m_running(false)
		{
		}

		virtual ~Thread()
		{
			if (m_running)
			{
				shutdown();
			}
		}

		void init(ThreadFn _fn, void* _userData = NULL, uint32_t _stackSize = 0, const char* _name = NULL)
		{
			BX_CHECK(!m_running, "Already running!");

			m_fn = _fn;
			m_userData = _userData;
			m_stackSize = _stackSize;
			m_running = true;

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360
			m_handle = CreateThread(NULL
				, m_stackSize
				, threadFunc
				, this
				, 0
				, NULL
				);
#elif BX_PLATFORM_WINRT
			m_handle = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
			auto workItemHandler = ref new WorkItemHandler([=](IAsyncAction^)
			{
				m_exitCode = threadFunc(this);
				SetEvent(m_handle);
			}, CallbackContext::Any);

			ThreadPool::RunAsync(workItemHandler, WorkItemPriority::Normal, WorkItemOptions::TimeSliced);
#elif BX_PLATFORM_POSIX
			int result;
			BX_UNUSED(result);

			pthread_attr_t attr;
			result = pthread_attr_init(&attr);
			BX_CHECK(0 == result, "pthread_attr_init failed! %d", result);

			if (0 != m_stackSize)
			{
				result = pthread_attr_setstacksize(&attr, m_stackSize);
				BX_CHECK(0 == result, "pthread_attr_setstacksize failed! %d", result);
			}

// 			sched_param sched;
// 			sched.sched_priority = 0;
// 			result = pthread_attr_setschedparam(&attr, &sched);
// 			BX_CHECK(0 == result, "pthread_attr_setschedparam failed! %d", result);

			result = pthread_create(&m_handle, &attr, &threadFunc, this);
			BX_CHECK(0 == result, "pthread_attr_setschedparam failed! %d", result);
#endif // BX_PLATFORM_

			m_sem.wait();

			if (NULL != _name)
			{
				setThreadName(_name);
			}
		}

		void shutdown()
		{
			BX_CHECK(m_running, "Not running!");
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360
			WaitForSingleObject(m_handle, INFINITE);
			GetExitCodeThread(m_handle, (DWORD*)&m_exitCode);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_WINRT
			WaitForSingleObjectEx(m_handle, INFINITE, FALSE);
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_POSIX
			union
			{
				void* ptr;
				int32_t i;
			} cast;
			pthread_join(m_handle, &cast.ptr);
			m_exitCode = cast.i;
			m_handle = 0;
#endif // BX_PLATFORM_
			m_running = false;
		}

		bool isRunning() const
		{
			return m_running;
		}

		int32_t getExitCode() const
		{
			return m_exitCode;
		}

		void setThreadName(const char* _name)
		{
#if BX_PLATFORM_OSX || BX_PLATFORM_IOS
			pthread_setname_np(_name);
#elif (BX_PLATFORM_LINUX && defined(__GLIBC__)) || BX_PLATFORM_FREEBSD
			pthread_setname_np(m_handle, _name);
#elif BX_PLATFORM_WINDOWS && BX_COMPILER_MSVC
#	pragma pack(push, 8)
			struct ThreadName
			{
				DWORD  type;
				LPCSTR name;
				DWORD  id;
				DWORD  flags;
			};
#	pragma pack(pop)
			ThreadName tn;
			tn.type  = 0x1000;
			tn.name  = _name;
			tn.id    = m_threadId;
			tn.flags = 0;

			__try
			{
				RaiseException(0x406d1388
					, 0
					, sizeof(tn)/4
					, reinterpret_cast<ULONG_PTR*>(&tn)
					);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
			}
#else
			BX_UNUSED(_name);
#endif // BX_PLATFORM_
		}

	private:
		int32_t entry()
		{
#if BX_PLATFORM_WINDOWS
			m_threadId = ::GetCurrentThreadId();
#endif // BX_PLATFORM_WINDOWS

			m_sem.post();
			return m_fn(m_userData);
		}

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_WINRT
		static DWORD WINAPI threadFunc(LPVOID _arg)
		{
			Thread* thread = (Thread*)_arg;
			int32_t result = thread->entry();
			return result;
		}
#else
		static void* threadFunc(void* _arg)
		{
			Thread* thread = (Thread*)_arg;
			union
			{
				void* ptr;
				int32_t i;
			} cast;
			cast.i = thread->entry();
			return cast.ptr;
		}
#endif // BX_PLATFORM_

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_WINRT
		HANDLE m_handle;
		DWORD  m_threadId;
#elif BX_PLATFORM_POSIX
		pthread_t m_handle;
#endif // BX_PLATFORM_

		ThreadFn  m_fn;
		void*     m_userData;
		Semaphore m_sem;
		uint32_t  m_stackSize;
		int32_t   m_exitCode;
		bool      m_running;
	};

#if BX_PLATFORM_WINDOWS
	class TlsData
	{
	public:
		TlsData()
		{
			m_id = TlsAlloc();
			BX_CHECK(TLS_OUT_OF_INDEXES != m_id, "Failed to allocated TLS index (err: 0x%08x).", GetLastError() );
		}

		~TlsData()
		{
			BOOL result = TlsFree(m_id);
			BX_CHECK(0 != result, "Failed to free TLS index (err: 0x%08x).", GetLastError() ); BX_UNUSED(result);
		}

		void* get() const
		{
			return TlsGetValue(m_id);
		}

		void set(void* _ptr)
		{
			TlsSetValue(m_id, _ptr);
		}

	private:
		uint32_t m_id;
	};

#elif !(BX_PLATFORM_WINRT)

	class TlsData
	{
	public:
		TlsData()
		{
			int result = pthread_key_create(&m_id, NULL);
			BX_CHECK(0 == result, "pthread_key_create failed %d.", result); BX_UNUSED(result);
		}

		~TlsData()
		{
			int result = pthread_key_delete(m_id);
			BX_CHECK(0 == result, "pthread_key_delete failed %d.", result); BX_UNUSED(result);
		}

		void* get() const
		{
			return pthread_getspecific(m_id);
		}

		void set(void* _ptr)
		{
			int result = pthread_setspecific(m_id, _ptr);
			BX_CHECK(0 == result, "pthread_setspecific failed %d.", result); BX_UNUSED(result);
		}

	private:
		pthread_key_t m_id;
	};
#endif // BX_PLATFORM_WINDOWS

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_THREAD_H_HEADER_GUARD
