/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MUTEX_H_HEADER_GUARD
#	error "Must be included from bx/mutex.h!"
#endif // BX_MUTEX_H_HEADER_GUARD

namespace bx
{
	inline MutexScope::MutexScope(Mutex& _mutex)
		: m_mutex(_mutex)
	{
		m_mutex.lock();
	}

	inline MutexScope::~MutexScope()
	{
		m_mutex.unlock();
	}

} // namespace bx
