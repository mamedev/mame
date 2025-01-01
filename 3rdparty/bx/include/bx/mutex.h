/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_MUTEX_H_HEADER_GUARD
#define BX_MUTEX_H_HEADER_GUARD

#include "bx.h"

namespace bx
{
	///
	class Mutex
	{
		BX_CLASS(Mutex
			, NO_COPY
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
		BX_ALIGN_DECL(16, uint8_t) m_internal[64];
	};

	///
	class MutexScope
	{
		BX_CLASS(MutexScope
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		///
		MutexScope(Mutex& _mutex);

		///
		~MutexScope();

	private:
		Mutex& m_mutex;
	};

} // namespace bx

#include "inline/mutex.inl"

#endif // BX_MUTEX_H_HEADER_GUARD
