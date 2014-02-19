// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    coretmpl.h

    Core templates for basic non-string types.

***************************************************************************/

#pragma once

#ifndef __CORETMPL_H__
#define __CORETMPL_H__

#include <assert.h>
#include "osdcore.h"


// ======================> dynamic_array

// an array that is dynamically sized and can optionally auto-expand
template<class _ElementType>
class dynamic_array
{
private:
	// we don't support deep copying
	dynamic_array(const dynamic_array &);
	dynamic_array &operator=(const dynamic_array &);

public:
	// construction/destruction
	dynamic_array(int initial = 0, bool clearit = false)
		: m_array(NULL),
			m_count(0),
			m_allocated(0) { if (initial != 0) expand_internal(initial); m_count = initial; if (clearit) clear(); }
	virtual ~dynamic_array() { reset(); }

	// operators
	operator _ElementType *() { return &m_array[0]; }
	operator const _ElementType *() const { return &m_array[0]; }
	_ElementType &operator[](int index) { assert(index < m_count); return m_array[index]; }
	const _ElementType &operator[](int index) const { assert(index < m_count); return m_array[index]; }

	// simple getters
	int count() const { return m_count; }

	// helpers
	const _ElementType &append(const _ElementType &element) { if (m_count == m_allocated) expand_internal((m_allocated == 0) ? 16 : (m_allocated << 1), true); m_array[m_count++] = element; return element; }
	void reset() { delete[] m_array; m_array = NULL; m_count = m_allocated = 0; }
	void resize(int count, bool keepdata = false) { if (count > m_allocated) expand_internal(count, keepdata); m_count = count; }
#ifdef __GNUC__
	void clear() { assert(__is_pod(_ElementType)); memset(m_array, 0, m_count * sizeof(*m_array)); }
#else
	void clear() { memset(m_array, 0, m_count * sizeof(*m_array)); }
#endif
	void resize_and_clear(int count) { resize(count); clear(); }

private:
	// internal helpers
	void expand_internal(int count, bool keepdata = true)
	{
		// allocate a new array, copy the old one, and proceed
		m_allocated = count;
		_ElementType *newarray = new _ElementType[m_allocated];
		if (keepdata)
			for (int index = 0; index < m_count; index++)
				newarray[index] = m_array[index];
		delete[] m_array;
		m_array = newarray;
	}

	// internal state
	_ElementType *  m_array;        // allocated array
	int             m_count;        // number of objects accessed in the list
	int             m_allocated;    // amount of space allocated for the array
};


// ======================> dynamic_buffer

typedef dynamic_array<UINT8> dynamic_buffer;



#endif
