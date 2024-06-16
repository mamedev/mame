/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/os.h>
#include <bx/thread.h>

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_WINDOWS && !BX_CRT_NONE
#	include <bx/string.h>
#endif

#if BX_CRT_NONE
#	include <bx/crt0.h>
#elif  BX_PLATFORM_ANDROID \
	|| BX_PLATFORM_LINUX   \
	|| BX_PLATFORM_IOS     \
	|| BX_PLATFORM_OSX     \
	|| BX_PLATFORM_PS4     \
	|| BX_PLATFORM_RPI     \
	|| BX_PLATFORM_NX      \
	|| BX_PLATFORM_VISIONOS
#	include <pthread.h>
#	if BX_PLATFORM_LINUX && (BX_CRT_GLIBC < 21200)
#		include <sys/prctl.h>
#	endif // BX_PLATFORM_
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <limits.h>
#	include <errno.h>
#endif // BX_PLATFORM_

namespace bx
{
	static AllocatorI* getAllocator()
	{
		static DefaultAllocator s_allocator;
		return &s_allocator;
	}

	struct ThreadInternal
	{
#if BX_CRT_NONE
		static int32_t threadFunc(void* _arg);
		int32_t m_handle;
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
		static DWORD WINAPI threadFunc(LPVOID _arg);
		HANDLE m_handle;
		DWORD  m_threadId;
#elif BX_PLATFORM_POSIX
		static void* threadFunc(void* _arg);
		pthread_t m_handle;
#endif // BX_PLATFORM_
	};

#if BX_CRT_NONE
	int32_t ThreadInternal::threadFunc(void* _arg)
	{
		Thread* thread = (Thread*)_arg;
		int32_t result = thread->entry();
		return result;
	}
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
	DWORD WINAPI ThreadInternal::threadFunc(LPVOID _arg)
	{
		Thread* thread = (Thread*)_arg;
		int32_t result = thread->entry();
		return result;
	}
#else
	void* ThreadInternal::threadFunc(void* _arg)
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

	Thread::Thread()
		: m_fn(NULL)
		, m_userData(NULL)
		, m_queue(getAllocator() )
		, m_stackSize(0)
		, m_exitCode(kExitSuccess)
		, m_running(false)
	{
		BX_STATIC_ASSERT(sizeof(ThreadInternal) <= sizeof(m_internal) );

		ThreadInternal* ti = (ThreadInternal*)m_internal;
#if BX_CRT_NONE
		ti->m_handle = -1;
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
		ti->m_handle   = INVALID_HANDLE_VALUE;
		ti->m_threadId = UINT32_MAX;
#elif BX_PLATFORM_POSIX
		ti->m_handle = 0;
#endif // BX_PLATFORM_
	}

	Thread::~Thread()
	{
		if (m_running)
		{
			shutdown();
		}
	}

	bool Thread::init(ThreadFn _fn, void* _userData, uint32_t _stackSize, const char* _name)
	{
		BX_ASSERT(!m_running, "Already running!");

		m_fn = _fn;
		m_userData = _userData;
		m_stackSize = _stackSize;

		if (NULL != _name)
		{
			BX_WARN(strLen(_name) < int32_t(BX_COUNTOF(m_name) )-1, "Truncating thread name.");
			strCopy(m_name, BX_COUNTOF(m_name), _name);
		}
		else
		{
			m_name[0] = '\0';
		}

		ThreadInternal* ti = (ThreadInternal*)m_internal;
#if BX_CRT_NONE
		ti->m_handle = crt0::threadCreate(&ti->threadFunc, _userData, m_stackSize, _name);

		if (-1 == ti->m_handle)
		{
			return false;
		}
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
		ti->m_handle = ::CreateThread(NULL
				, m_stackSize
				, (LPTHREAD_START_ROUTINE)ti->threadFunc
				, this
				, 0
				, NULL
				);
		if (NULL == ti->m_handle)
		{
			return false;
		}
#elif BX_PLATFORM_POSIX
		int result;
		BX_UNUSED(result);

		pthread_attr_t attr;
		result = pthread_attr_init(&attr);
		BX_WARN(0 == result, "pthread_attr_init failed! %d", result);
		if (0 != result)
		{
			return false;
		}

		if (0 != m_stackSize)
		{
			result = pthread_attr_setstacksize(&attr, m_stackSize);
			BX_WARN(0 == result, "pthread_attr_setstacksize failed! %d", result);

			if (0 != result)
			{
				return false;
			}
		}

		result = pthread_create(&ti->m_handle, &attr, &ti->threadFunc, this);
		BX_WARN(0 == result, "pthread_attr_setschedparam failed! %d", result);
		if (0 != result)
		{
			return false;
		}
#else
#	error "Not implemented!"
#endif // BX_PLATFORM_

		m_running = true;
		m_sem.wait();

		return true;
	}

	void Thread::shutdown()
	{
		BX_ASSERT(m_running, "Not running!");
		ThreadInternal* ti = (ThreadInternal*)m_internal;
#if BX_CRT_NONE
		crt0::threadJoin(ti->m_handle, NULL);
#elif BX_PLATFORM_WINDOWS
		WaitForSingleObject(ti->m_handle, INFINITE);
		GetExitCodeThread(ti->m_handle, (DWORD*)&m_exitCode);
		CloseHandle(ti->m_handle);
		ti->m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_WINRT || BX_PLATFORM_XBOXONE
		WaitForSingleObjectEx(ti->m_handle, INFINITE, FALSE);
		CloseHandle(ti->m_handle);
		ti->m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_POSIX
		union
		{
			void* ptr;
			int32_t i;
		} cast;
		pthread_join(ti->m_handle, &cast.ptr);
		m_exitCode = cast.i;
		ti->m_handle = 0;
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
		ThreadInternal* ti = (ThreadInternal*)m_internal;
		BX_UNUSED(ti);
#if BX_CRT_NONE
		BX_UNUSED(_name);
#elif  BX_PLATFORM_OSX \
	|| BX_PLATFORM_IOS   \
	|| BX_PLATFORM_VISIONOS
		pthread_setname_np(_name);
#elif (BX_CRT_GLIBC >= 21200)
		pthread_setname_np(ti->m_handle, _name);
#elif BX_PLATFORM_LINUX
		prctl(PR_SET_NAME,_name, 0, 0, 0);
#elif BX_PLATFORM_WINDOWS
		// Try to use the new thread naming API from Win10 Creators update onwards if we have it
		typedef HRESULT (WINAPI *SetThreadDescriptionProc)(HANDLE, PCWSTR);
		SetThreadDescriptionProc SetThreadDescription = dlsym<SetThreadDescriptionProc>((void*)GetModuleHandleA("Kernel32.dll"), "SetThreadDescription");

		if (NULL != SetThreadDescription)
		{
			uint32_t length = (uint32_t)strLen(_name)+1;
			uint32_t size = length*sizeof(wchar_t);
			wchar_t* name = (wchar_t*)alloca(size);
			mbstowcs(name, _name, size-2);
			SetThreadDescription(ti->m_handle, name);
		}
#	if BX_COMPILER_MSVC
#		pragma pack(push, 8)
			struct ThreadName
			{
				DWORD  type;
				LPCSTR name;
				DWORD  id;
				DWORD  flags;
			};
#		pragma pack(pop)
			ThreadName tn;
			tn.type  = 0x1000;
			tn.name  = _name;
			tn.id    = ti->m_threadId;
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
#	endif // BX_COMPILER_MSVC
#else
		BX_UNUSED(_name);
#endif // BX_PLATFORM_
	}

	void Thread::push(void* _ptr)
	{
		m_queue.push(_ptr);
	}

	void* Thread::pop()
	{
		void* ptr = m_queue.pop();
		return ptr;
	}

	int32_t Thread::entry()
	{
#if BX_PLATFORM_WINDOWS
		ThreadInternal* ti = (ThreadInternal*)m_internal;
		ti->m_threadId = ::GetCurrentThreadId();
#endif // BX_PLATFORM_WINDOWS

		m_sem.post();
		setThreadName(m_name);
		int32_t result = m_fn(this, m_userData);
		return result;
	}

	struct TlsDataInternal
	{
#if BX_CRT_NONE
#elif BX_PLATFORM_WINDOWS
		uint32_t m_id;
#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)
		pthread_key_t m_id;
#endif // BX_PLATFORM_*
	};

#if BX_CRT_NONE
	TlsData::TlsData()
	{
		BX_STATIC_ASSERT(sizeof(TlsDataInternal) <= sizeof(m_internal) );

		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		BX_UNUSED(ti);
	}

	TlsData::~TlsData()
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		BX_UNUSED(ti);
	}

	void* TlsData::get() const
	{
		return NULL;
	}

	void TlsData::set(void* _ptr)
	{
		BX_UNUSED(_ptr);

		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		BX_UNUSED(ti);
	}
#elif BX_PLATFORM_WINDOWS
	TlsData::TlsData()
	{
		BX_STATIC_ASSERT(sizeof(TlsDataInternal) <= sizeof(m_internal) );

		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		ti->m_id = TlsAlloc();
		BX_ASSERT(TLS_OUT_OF_INDEXES != ti->m_id, "Failed to allocated TLS index (err: 0x%08x).", GetLastError() );
	}

	TlsData::~TlsData()
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		BOOL result = TlsFree(ti->m_id);
		BX_ASSERT(0 != result, "Failed to free TLS index (err: 0x%08x).", GetLastError() ); BX_UNUSED(result);
	}

	void* TlsData::get() const
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		return TlsGetValue(ti->m_id);
	}

	void TlsData::set(void* _ptr)
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		TlsSetValue(ti->m_id, _ptr);
	}

#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)

	TlsData::TlsData()
	{
		BX_STATIC_ASSERT(sizeof(TlsDataInternal) <= sizeof(m_internal) );

		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		int result = pthread_key_create(&ti->m_id, NULL);
		BX_ASSERT(0 == result, "pthread_key_create failed %d.", result); BX_UNUSED(result);
	}

	TlsData::~TlsData()
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		int result = pthread_key_delete(ti->m_id);
		BX_ASSERT(0 == result, "pthread_key_delete failed %d.", result); BX_UNUSED(result);
	}

	void* TlsData::get() const
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		return pthread_getspecific(ti->m_id);
	}

	void TlsData::set(void* _ptr)
	{
		TlsDataInternal* ti = (TlsDataInternal*)m_internal;
		int result = pthread_setspecific(ti->m_id, _ptr);
		BX_ASSERT(0 == result, "pthread_setspecific failed %d.", result); BX_UNUSED(result);
	}
#endif // BX_PLATFORM_*

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING
