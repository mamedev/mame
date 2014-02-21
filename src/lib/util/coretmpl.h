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

// TEMPORARY helper to catch is_pod assertions in the debugger
#if 0
#undef assert
#define assert(x) do { if (!(x)) { printf("Assert: %s\n", #x); asm("int $3"); } } while (0)
#endif

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
	dynamic_array(int initial = 0, int clearvalue = -1)
		: m_array(NULL),
			m_count(0),
			m_allocated(0) { if (initial != 0) expand_internal(initial); m_count = initial; if (clearvalue != -1) clear(clearvalue); }
	virtual ~dynamic_array() { reset(); }

	// operators
	operator _ElementType *() { return &m_array[0]; }
	operator const _ElementType *() const { return &m_array[0]; }
	_ElementType &operator[](int index) { assert(index < m_count); return m_array[index]; }
	const _ElementType &operator[](int index) const { assert(index < m_count); return m_array[index]; }

	// simple getters
	int count() const { return m_count; }

	// core operations
	const _ElementType &append(const _ElementType &element) { if (m_count == m_allocated) expand_and_keep_internal((m_allocated == 0) ? 16 : (m_allocated << 1)); m_array[m_count++] = element; return element; }
	void reset() { delete[] m_array; m_array = NULL; m_count = m_allocated = 0; }
	void resize(int count) { if (count > m_allocated) expand_internal(count); m_count = count; }
	void resize_keep(int count) { if (count > m_allocated) expand_and_keep_internal(count); m_count = count; }
	void clear(UINT8 data = 0) { clear_internal(0, m_count, data); }

	// compound operations
	void resize_and_clear(int count, UINT8 data = 0) { resize(count); clear(data); }
	void resize_keep_and_clear_new(int count, UINT8 data = 0) { int oldcount = m_count; resize_keep(count); if (oldcount < m_count) clear_internal(oldcount, m_count - oldcount, data); }

private:
	// internal helpers
	void expand_internal(int count)
	{
		delete[] m_array;
		m_array = new _ElementType[count];
		m_allocated = count;
	}

	void expand_and_keep_internal(int count)
	{
		_ElementType *oldarray = m_array;
		int oldcount = m_count;
		m_array = new _ElementType[count];
		m_allocated = count;
		for (int index = 0; index < oldcount; index++)
			m_array[index] = oldarray[index];
		delete[] oldarray;
	}

#ifdef __GNUC__
	void clear_internal(UINT32 start, UINT32 count, UINT8 data) { assert(__is_pod(_ElementType)); memset(&m_array[start], data, count * sizeof(*m_array)); }
#else
	void clear_internal(UINT32 start, UINT32 count, UINT8 data) { memset(&m_array[start], data, count * sizeof(*m_array)); }
#endif

	// internal state
	_ElementType *  m_array;        // allocated array
	int             m_count;        // number of objects accessed in the list
	int             m_allocated;    // amount of space allocated for the array
};


// ======================> dynamic_buffer

typedef dynamic_array<UINT8> dynamic_buffer;



#endif
