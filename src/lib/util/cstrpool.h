// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    cstrpool.h

    Constant string pool helper class.

*********************************************************************/

#pragma once

#ifndef __CSTRPOOL_H_
#define __CSTRPOOL_H_

#include "coretmpl.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> const_string_pool

// a pool to hold constant strings efficiently
class const_string_pool
{
public:
	// construction
	const_string_pool();

	// operations
	void reset() { m_chunklist.reset(); }
	const char *add(const char *string);
	bool contains(const char *string);

private:
	// shared string pool
	class pool_chunk
	{
		static const int POOL_SIZE = 4096;
		friend class simple_list<pool_chunk>;

	public:
		// construction
		pool_chunk();

		// getters
		pool_chunk *next() const { return m_next; }

		// operations
		const char *add(const char *string);
		bool contains(const char *string) const { return (string >= m_buffer && string < &m_buffer[POOL_SIZE]); }

	private:
		// internal state
		pool_chunk *            m_next;
		UINT32                  m_used;
		char                    m_buffer[POOL_SIZE];
	};
	simple_list<pool_chunk>     m_chunklist;
};


#endif
