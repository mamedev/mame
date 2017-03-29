/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/semaphore.h>

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_POSIX
#	include <errno.h>
#	include <pthread.h>
#	include <semaphore.h>
#	include <time.h>
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOX360 \
	|| BX_PLATFORM_XBOXONE
#	include <windows.h>
#	include <limits.h>
#	if BX_PLATFORM_XBOXONE
#		include <synchapi.h>
#	endif // BX_PLATFORM_XBOXONE
#endif // BX_PLATFORM_

#ifndef BX_CONFIG_SEMAPHORE_PTHREAD
#	define BX_CONFIG_SEMAPHORE_PTHREAD (0 \
			|| BX_PLATFORM_OSX            \
			|| BX_PLATFORM_IOS            \
			)
#endif // BX_CONFIG_SEMAPHORE_PTHREAD

namespace bx
{
	struct SemaphoreInternal
	{
#if BX_PLATFORM_POSIX
#	if BX_CONFIG_SEMAPHORE_PTHREAD
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
		int32_t m_count;
#	else
		sem_t m_handle;
#	endif // BX_CONFIG_SEMAPHORE_PTHREAD
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOX360 \
	|| BX_PLATFORM_XBOXONE
		HANDLE m_handle;
#endif // BX_PLATFORM_
	};

#if BX_PLATFORM_POSIX

#	if BX_CONFIG_SEMAPHORE_PTHREAD
	Semaphore::Semaphore()
	{
		BX_STATIC_ASSERT(sizeof(SemaphoreInternal) <= sizeof(m_internal) );

		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;
		si->m_count = 0;

		int result;

		result = pthread_mutex_init(&si->m_mutex, NULL);
		BX_CHECK(0 == result, "pthread_mutex_init %d", result);

		result = pthread_cond_init(&si->m_cond, NULL);
		BX_CHECK(0 == result, "pthread_cond_init %d", result);

		BX_UNUSED(result);
	}

	Semaphore::~Semaphore()
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int result;
		result = pthread_cond_destroy(&si->m_cond);
		BX_CHECK(0 == result, "pthread_cond_destroy %d", result);

		result = pthread_mutex_destroy(&si->m_mutex);
		BX_CHECK(0 == result, "pthread_mutex_destroy %d", result);

		BX_UNUSED(result);
	}

	void Semaphore::post(uint32_t _count)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int result = pthread_mutex_lock(&si->m_mutex);
		BX_CHECK(0 == result, "pthread_mutex_lock %d", result);

		for (uint32_t ii = 0; ii < _count; ++ii)
		{
			result = pthread_cond_signal(&si->m_cond);
			BX_CHECK(0 == result, "pthread_cond_signal %d", result);
		}

		si->m_count += _count;

		result = pthread_mutex_unlock(&si->m_mutex);
		BX_CHECK(0 == result, "pthread_mutex_unlock %d", result);

		BX_UNUSED(result);
	}

	bool Semaphore::wait(int32_t _msecs)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int result = pthread_mutex_lock(&si->m_mutex);
		BX_CHECK(0 == result, "pthread_mutex_lock %d", result);

#		if BX_PLATFORM_NACL || BX_PLATFORM_OSX
		BX_UNUSED(_msecs);
		BX_CHECK(-1 == _msecs, "NaCl and OSX don't support pthread_cond_timedwait at this moment.");
		while (0 == result
		&&     0 >= si->m_count)
		{
			result = pthread_cond_wait(&si->m_cond, &si->m_mutex);
		}
#		elif BX_PLATFORM_IOS
		if (-1 == _msecs)
		{
			while (0 == result
			&&     0 >= si->m_count)
			{
				result = pthread_cond_wait(&si->m_cond, &si->m_mutex);
			}
		}
		else
		{
			timespec ts;
			ts.tv_sec = _msecs/1000;
			ts.tv_nsec = (_msecs%1000)*1000;

			while (0 == result
			&&     0 >= si->m_count)
			{
				result = pthread_cond_timedwait_relative_np(&si->m_cond, &si->m_mutex, &ts);
			}
		}
#		else
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += _msecs/1000;
		ts.tv_nsec += (_msecs%1000)*1000;

		while (0 == result
		&&     0 >= si->m_count)
		{
			result = pthread_cond_timedwait(&si->m_cond, &si->m_mutex, &ts);
		}
#		endif // BX_PLATFORM_
		bool ok = 0 == result;

		if (ok)
		{
			--si->m_count;
		}

		result = pthread_mutex_unlock(&si->m_mutex);
		BX_CHECK(0 == result, "pthread_mutex_unlock %d", result);

		BX_UNUSED(result);

		return ok;
	}

#	else

	Semaphore::Semaphore()
	{
		BX_STATIC_ASSERT(sizeof(SemaphoreInternal) <= sizeof(m_internal) );

		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int32_t result = sem_init(&si->m_handle, 0, 0);
		BX_CHECK(0 == result, "sem_init failed. errno %d", errno);
		BX_UNUSED(result);
	}

	Semaphore::~Semaphore()
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int32_t result = sem_destroy(&si->m_handle);
		BX_CHECK(0 == result, "sem_destroy failed. errno %d", errno);
		BX_UNUSED(result);
	}

	void Semaphore::post(uint32_t _count)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		int32_t result;
		for (uint32_t ii = 0; ii < _count; ++ii)
		{
			result = sem_post(&si->m_handle);
			BX_CHECK(0 == result, "sem_post failed. errno %d", errno);
		}
		BX_UNUSED(result);
	}

	bool Semaphore::wait(int32_t _msecs)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

#		if BX_PLATFORM_NACL || BX_PLATFORM_OSX
		BX_CHECK(-1 == _msecs, "NaCl and OSX don't support sem_timedwait at this moment."); BX_UNUSED(_msecs);
		return 0 == sem_wait(&si->m_handle);
#		else
		if (0 > _msecs)
		{
			int32_t result;
			do
			{
				result = sem_wait(&si->m_handle);
			} // keep waiting when interrupted by a signal handler...
			while (-1 == result && EINTR == errno);
			BX_CHECK(0 == result, "sem_wait failed. errno %d", errno);
			return 0 == result;
		}

		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += _msecs/1000;
		ts.tv_nsec += (_msecs%1000)*1000;
		return 0 == sem_timedwait(&si->m_handle, &ts);
#		endif // BX_PLATFORM_
	}
#	endif // BX_CONFIG_SEMAPHORE_PTHREAD

#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOX360 \
	|| BX_PLATFORM_XBOXONE

	Semaphore::Semaphore()
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

#if BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		si->m_handle = CreateSemaphoreExW(NULL, 0, LONG_MAX, NULL, 0, SEMAPHORE_ALL_ACCESS);
#else
		si->m_handle = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
#endif
		BX_CHECK(NULL != si->m_handle, "Failed to create Semaphore!");
	}

	Semaphore::~Semaphore()
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		CloseHandle(si->m_handle);
	}

	void Semaphore::post(uint32_t _count)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		ReleaseSemaphore(si->m_handle, _count, NULL);
	}

	bool Semaphore::wait(int32_t _msecs)
	{
		SemaphoreInternal* si = (SemaphoreInternal*)m_internal;

		DWORD milliseconds = (0 > _msecs) ? INFINITE : _msecs;
#if BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		return WAIT_OBJECT_0 == WaitForSingleObjectEx(si->m_handle, milliseconds, FALSE);
#else
		return WAIT_OBJECT_0 == WaitForSingleObject(si->m_handle, milliseconds);
#endif
	}
#endif // BX_PLATFORM_

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING
