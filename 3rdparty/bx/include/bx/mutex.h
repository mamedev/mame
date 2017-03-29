/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MUTEX_H_HEADER_GUARD
#define BX_MUTEX_H_HEADER_GUARD

#include "bx.h"
#include "cpu.h"
#include "os.h"
#include "sem.h"

#if BX_CONFIG_SUPPORTS_THREADING

#if 0 \
	|| BX_PLATFORM_ANDROID \
	|| BX_PLATFORM_LINUX \
	|| BX_PLATFORM_NACL \
	|| BX_PLATFORM_IOS \
	|| BX_PLATFORM_OSX
#	include <pthread.h>
#endif //

namespace bx
{
	///
	class Mutex
	{
		BX_CLASS(Mutex
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		Mutex();

		///
		~Mutex();

		///
		void lock();

		///
		void unlock();

	private:
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
		CRITICAL_SECTION m_handle;
#else
		pthread_mutex_t m_handle;
#endif // BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360 || BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
	};

	///
	class MutexScope
	{
		BX_CLASS(MutexScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		MutexScope(Mutex& _mutex);

		///
		~MutexScope();

	private:
		Mutex& m_mutex;
	};

	typedef Mutex LwMutex;

	///
	class LwMutexScope
	{
		BX_CLASS(LwMutexScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			, NO_ASSIGNMENT
			);

	public:
		///
		LwMutexScope(LwMutex& _mutex);

		///
		~LwMutexScope();

	private:
		LwMutex& m_mutex;
	};

} // namespace bx

#include "mutex.inl"

#endif // BX_CONFIG_SUPPORTS_THREADING

#endif // BX_MUTEX_H_HEADER_GUARD
