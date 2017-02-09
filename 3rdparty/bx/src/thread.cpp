/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/thread.h>

#if BX_CONFIG_SUPPORTS_THREADING

namespace bx
{
	Thread::Thread()
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
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

	Thread::~Thread()
	{
		if (m_running)
		{
			shutdown();
		}
	}

	void Thread::init(ThreadFn _fn, void* _userData, uint32_t _stackSize, const char* _name)
	{
		BX_CHECK(!m_running, "Already running!");

		m_fn = _fn;
		m_userData = _userData;
		m_stackSize = _stackSize;
		m_running = true;

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE
		m_handle = ::CreateThread(NULL
				, m_stackSize
				, (LPTHREAD_START_ROUTINE)threadFunc
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
#else
#	error "Not implemented!"
#endif // BX_PLATFORM_

		m_sem.wait();

		if (NULL != _name)
		{
			setThreadName(_name);
		}
	}

	void Thread::shutdown()
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

	bool Thread::isRunning() const
	{
		return m_running;
	}

	int32_t Thread::getExitCode() const
	{
		return m_exitCode;
	}

	void Thread::setThreadName(const char* _name)
	{
#if BX_PLATFORM_OSX || BX_PLATFORM_IOS
		pthread_setname_np(_name);
#elif (BX_CRT_GLIBC >= 21200) && ! BX_PLATFORM_HURD
		pthread_setname_np(m_handle, _name);
#elif BX_PLATFORM_LINUX
		prctl(PR_SET_NAME,_name, 0, 0, 0);
#elif BX_PLATFORM_BSD
#	ifdef __NetBSD__
		pthread_setname_np(m_handle, "%s", (void*)_name);
#	else
		pthread_set_name_np(m_handle, _name);
#	endif // __NetBSD__
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

	int32_t Thread::entry()
	{
#if BX_PLATFORM_WINDOWS
		m_threadId = ::GetCurrentThreadId();
#endif // BX_PLATFORM_WINDOWS

		m_sem.post();
		return m_fn(m_userData);
	}

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
	DWORD WINAPI Thread::threadFunc(LPVOID _arg)
	{
		Thread* thread = (Thread*)_arg;
		int32_t result = thread->entry();
		return result;
	}
#else
	void* Thread::threadFunc(void* _arg)
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

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING
