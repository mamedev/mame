/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SEM_H_HEADER_GUARD
#define BX_SEM_H_HEADER_GUARD

#include "bx.h"

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_POSIX
#	include <errno.h>
#	include <semaphore.h>
#	include <time.h>
#	include <pthread.h>
#elif BX_PLATFORM_XBOX360 || BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT || BX_PLATFORM_XBOXONE
#	include <windows.h>
#	include <limits.h>
#	if BX_PLATFORM_XBOXONE
#		include <synchapi.h>
#	endif // BX_PLATFORM_XBOXONE
#endif // BX_PLATFORM_

#include "mutex.h"

namespace bx
{
	///
	class Semaphore
	{
		BX_CLASS(Semaphore
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		Semaphore();

		///
		~Semaphore();

		///
		void post(uint32_t _count = 1);

		///
		bool wait(int32_t _msecs = -1);

	private:
#if BX_PLATFORM_POSIX
#	if BX_CONFIG_SEMAPHORE_PTHREAD
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
		int32_t m_count;
#	else
		sem_t m_handle;
#	endif // BX_CONFIG_SEMAPHORE_PTHREAD
#elif BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
		HANDLE m_handle;
#endif // BX_PLATFORM_
	};

} // namespace bx

#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_SEM_H_HEADER_GUARD
