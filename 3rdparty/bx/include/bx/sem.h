/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SEM_H_HEADER_GUARD
#define BX_SEM_H_HEADER_GUARD

#include "bx.h"
#include "mutex.h"

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_POSIX
#	include <errno.h>
#	include <semaphore.h>
#	include <time.h>
#	include <pthread.h>
#elif BX_PLATFORM_XBOXONE
#	include <synchapi.h>
#elif BX_PLATFORM_XBOX360 || BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
#	include <windows.h>
#	include <limits.h>
#endif // BX_PLATFORM_

namespace bx
{
#if BX_PLATFORM_POSIX

#	if BX_CONFIG_SEMAPHORE_PTHREAD
	class Semaphore
	{
		BX_CLASS(Semaphore
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		Semaphore()
			: m_count(0)
		{
			int result;
			result = pthread_mutex_init(&m_mutex, NULL);
			BX_CHECK(0 == result, "pthread_mutex_init %d", result);

			result = pthread_cond_init(&m_cond, NULL);
			BX_CHECK(0 == result, "pthread_cond_init %d", result);

			BX_UNUSED(result);
		}

		~Semaphore()
		{
			int result;
			result = pthread_cond_destroy(&m_cond);
			BX_CHECK(0 == result, "pthread_cond_destroy %d", result);

			result = pthread_mutex_destroy(&m_mutex);
			BX_CHECK(0 == result, "pthread_mutex_destroy %d", result);

			BX_UNUSED(result);
		}

		void post(uint32_t _count = 1)
		{
			int result = pthread_mutex_lock(&m_mutex);
			BX_CHECK(0 == result, "pthread_mutex_lock %d", result);

			for (uint32_t ii = 0; ii < _count; ++ii)
			{
				result = pthread_cond_signal(&m_cond);
				BX_CHECK(0 == result, "pthread_cond_signal %d", result);
			}

			m_count += _count;

			result = pthread_mutex_unlock(&m_mutex);
			BX_CHECK(0 == result, "pthread_mutex_unlock %d", result);

			BX_UNUSED(result);
		}

		bool wait(int32_t _msecs = -1)
		{
			int result = pthread_mutex_lock(&m_mutex);
			BX_CHECK(0 == result, "pthread_mutex_lock %d", result);

#		if BX_PLATFORM_NACL || BX_PLATFORM_OSX
			BX_UNUSED(_msecs);
			BX_CHECK(-1 == _msecs, "NaCl and OSX don't support pthread_cond_timedwait at this moment.");
			while (0 == result
			&&	 0 >= m_count)
			{
				result = pthread_cond_wait(&m_cond, &m_mutex);
			}
#		elif BX_PLATFORM_IOS
			if (-1 == _msecs)
			{
				while (0 == result
				&&     0 >= m_count)
				{
					result = pthread_cond_wait(&m_cond, &m_mutex);
				}
			}
			else
			{
				timespec ts;
				ts.tv_sec = _msecs/1000;
				ts.tv_nsec = (_msecs%1000)*1000;

				while (0 == result
				&&     0 >= m_count)
				{
					result = pthread_cond_timedwait_relative_np(&m_cond, &m_mutex, &ts);
				}
			}
#		else
			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += _msecs/1000;
			ts.tv_nsec += (_msecs%1000)*1000;

			while (0 == result
			&&     0 >= m_count)
			{
				result = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
			}
#		endif // BX_PLATFORM_NACL || BX_PLATFORM_OSX
			bool ok = 0 == result;

			if (ok)
			{
				--m_count;
			}

			result = pthread_mutex_unlock(&m_mutex);
			BX_CHECK(0 == result, "pthread_mutex_unlock %d", result);

			BX_UNUSED(result);

			return ok;
		}

	private:
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
		int32_t m_count;
	};

#	else

	class Semaphore
	{
		BX_CLASS(Semaphore
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		Semaphore()
		{
			int32_t result = sem_init(&m_handle, 0, 0);
			BX_CHECK(0 == result, "sem_init failed. errno %d", errno);
			BX_UNUSED(result);
		}

		~Semaphore()
		{
			int32_t result = sem_destroy(&m_handle);
			BX_CHECK(0 == result, "sem_destroy failed. errno %d", errno);
			BX_UNUSED(result);
		}

		void post(uint32_t _count = 1)
		{
			int32_t result;
			for (uint32_t ii = 0; ii < _count; ++ii)
			{
				result = sem_post(&m_handle);
				BX_CHECK(0 == result, "sem_post failed. errno %d", errno);
			}
			BX_UNUSED(result);
		}

		bool wait(int32_t _msecs = -1)
		{
#		if BX_PLATFORM_NACL || BX_PLATFORM_OSX
			BX_CHECK(-1 == _msecs, "NaCl and OSX don't support sem_timedwait at this moment."); BX_UNUSED(_msecs);
			return 0 == sem_wait(&m_handle);
#		else
			if (0 > _msecs)
			{
				int32_t result;
				do
				{
					result = sem_wait(&m_handle);
				} // keep waiting when interrupted by a signal handler...
				while (-1 == result && EINTR == errno);
				BX_CHECK(0 == result, "sem_wait failed. errno %d", errno);
				return 0 == result;
			}

			timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += _msecs/1000;
			ts.tv_nsec += (_msecs%1000)*1000;
			return 0 == sem_timedwait(&m_handle, &ts);
#		endif // BX_PLATFORM_
		}

	private:
		sem_t m_handle;
	};
#	endif // BX_CONFIG_SEMAPHORE_PTHREAD

#elif BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT

	class Semaphore
	{
		BX_CLASS(Semaphore
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		Semaphore()
		{
#if BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
			m_handle = CreateSemaphoreExW(NULL, 0, LONG_MAX, NULL, 0, SEMAPHORE_ALL_ACCESS);
#else
			m_handle = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
#endif
			BX_CHECK(NULL != m_handle, "Failed to create Semaphore!");
		}

		~Semaphore()
		{
			CloseHandle(m_handle);
		}

		void post(uint32_t _count = 1) const
		{
			ReleaseSemaphore(m_handle, _count, NULL);
		}

		bool wait(int32_t _msecs = -1) const
		{
			DWORD milliseconds = (0 > _msecs) ? INFINITE : _msecs;
#if BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
			return WAIT_OBJECT_0 == WaitForSingleObjectEx(m_handle, milliseconds, FALSE);
#else
			return WAIT_OBJECT_0 == WaitForSingleObject(m_handle, milliseconds);
#endif
		}

	private:
		HANDLE m_handle;
	};

#endif // BX_PLATFORM_

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_SEM_H_HEADER_GUARD
