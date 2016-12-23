/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MUTEX_H_HEADER_GUARD
#define BX_MUTEX_H_HEADER_GUARD

#include "bx.h"
#include "cpu.h"
#include "os.h"
#include "sem.h"

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_NACL || BX_PLATFORM_LINUX || BX_PLATFORM_ANDROID || BX_PLATFORM_OSX
#	include <pthread.h>
#elif BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_WINRT
#	include <errno.h>
#endif // BX_PLATFORM_

namespace bx
{
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
	typedef CRITICAL_SECTION pthread_mutex_t;
	typedef unsigned pthread_mutexattr_t;

	inline int pthread_mutex_lock(pthread_mutex_t* _mutex)
	{
		EnterCriticalSection(_mutex);
		return 0;
	}

	inline int pthread_mutex_unlock(pthread_mutex_t* _mutex)
	{
		LeaveCriticalSection(_mutex);
		return 0;
	}

	inline int pthread_mutex_trylock(pthread_mutex_t* _mutex)
	{
		return TryEnterCriticalSection(_mutex) ? 0 : EBUSY;
	}

	inline int pthread_mutex_init(pthread_mutex_t* _mutex, pthread_mutexattr_t* /*_attr*/)
	{
#if BX_PLATFORM_WINRT
		InitializeCriticalSectionEx(_mutex, 4000, 0);   // docs recommend 4000 spincount as sane default
#else
		InitializeCriticalSection(_mutex);
#endif
		return 0;
	}

	inline int pthread_mutex_destroy(pthread_mutex_t* _mutex)
	{
		DeleteCriticalSection(_mutex);
		return 0;
	}
#endif // BX_PLATFORM_

	class Mutex
	{
		BX_CLASS(Mutex
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		Mutex()
		{
			pthread_mutexattr_t attr;
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
#else
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif // BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_WINRT
			pthread_mutex_init(&m_handle, &attr);
		}

		~Mutex()
		{
			pthread_mutex_destroy(&m_handle);
		}

		void lock()
		{
			pthread_mutex_lock(&m_handle);
		}

		void unlock()
		{
			pthread_mutex_unlock(&m_handle);
		}

	private:
		pthread_mutex_t m_handle;
	};

	class MutexScope
	{
		BX_CLASS(MutexScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		MutexScope(Mutex& _mutex)
			: m_mutex(_mutex)
		{
			m_mutex.lock();
		}

		~MutexScope()
		{
			m_mutex.unlock();
		}

	private:
		Mutex& m_mutex;
	};

	typedef Mutex LwMutex;

	class LwMutexScope
	{
		BX_CLASS(LwMutexScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		LwMutexScope(LwMutex& _mutex)
			: m_mutex(_mutex)
		{
			m_mutex.lock();
		}

		~LwMutexScope()
		{
			m_mutex.unlock();
		}

	private:
		LwMutex& m_mutex;
	};

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_MUTEX_H_HEADER_GUARD
