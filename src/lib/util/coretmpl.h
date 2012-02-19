/***************************************************************************

    coretmpl.h

    Core templates for basic non-string types.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
	dynamic_array(int initial = 0) 
		: m_array(NULL),
		  m_count(0),
		  m_allocated(0) { if (initial != 0) expand_internal(initial); m_count = initial; }
	virtual ~dynamic_array() { reset(); }

	// operators
	operator _ElementType *() { return &m_array[0]; }
	operator const _ElementType *() const { return &m_array[0]; }
	_ElementType operator[](int index) const { assert(index < m_count); return m_array[index]; }
	_ElementType &operator[](int index) { assert(index < m_count); return m_array[index]; }

	// simple getters
	int count() const { return m_count; }
	
	// helpers
	void append(const _ElementType &element) { if (m_count == m_allocated) expand_internal((m_allocated == 0) ? 16 : (m_allocated << 1), true); m_array[m_count++] = element; }
	void reset() { delete[] m_array; m_array = NULL; m_count = m_allocated = 0; }
	void resize(int count, bool keepdata = false) { if (count > m_allocated) expand_internal(count, keepdata); m_count = count; }

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
	_ElementType *	m_array;		// allocated array
	int 			m_count;		// number of objects accessed in the list
	int				m_allocated;	// amount of space allocated for the array
};


// ======================> dynamic_buffer

typedef dynamic_array<UINT8> dynamic_buffer;



#endif
