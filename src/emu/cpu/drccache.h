/***************************************************************************

    drccache.h

    Universal dynamic recompiler cache management.

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

#ifndef __DRCCACHE_H__
#define __DRCCACHE_H__



//**************************************************************************
//  MACROS
//**************************************************************************

// ensure that a given pointer is within the cache boundaries
#define assert_in_cache(c,p)        assert((c).contains_pointer(p))
#define assert_in_near_cache(c,p)   assert((c).contains_near_pointer(p))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// generic code pointer
typedef UINT8 *drccodeptr;


// helper template for oob codegen
typedef delegate<void (drccodeptr *, void *, void *)> drc_oob_delegate;


// drc_cache
class drc_cache
{
public:
	// construction/destruction
	drc_cache(size_t bytes);
	~drc_cache();

	// getters
	drccodeptr near() const { return m_near; }
	drccodeptr base() const { return m_base; }
	drccodeptr top() const { return m_top; }

	// pointer checking
	bool contains_pointer(const void *ptr) const { return ((const drccodeptr)ptr >= m_near && (const drccodeptr)ptr < m_near + m_size); }
	bool contains_near_pointer(const void *ptr) const { return ((const drccodeptr)ptr >= m_near && (const drccodeptr)ptr < m_neartop); }
	bool generating_code() const { return (m_codegen != NULL); }

	// memory management
	void flush();
	void *alloc(size_t bytes);
	void *alloc_near(size_t bytes);
	void *alloc_temporary(size_t bytes);
	void dealloc(void *memory, size_t bytes);

	// codegen helpers
	drccodeptr *begin_codegen(UINT32 reserve_bytes);
	drccodeptr end_codegen();
	void request_oob_codegen(drc_oob_delegate callback, void *param1 = NULL, void *param2 = NULL);

private:
	// largest block of code that can be generated at once
	static const size_t CODEGEN_MAX_BYTES = 65536;

	// minimum alignment, in bytes (must be power of 2)
	static const size_t CACHE_ALIGNMENT = 8;

	// largest permanent allocation we allow
	static const size_t MAX_PERMANENT_ALLOC = 1024;

	// size of "near" area at the base of the cache
	static const size_t NEAR_CACHE_SIZE = 65536;

	// core parameters
	drccodeptr          m_near;             // pointer to the near part of the cache
	drccodeptr          m_neartop;          // top of the near part of the cache
	drccodeptr          m_base;             // base pointer to the compiler cache
	drccodeptr          m_top;              // current top of cache
	drccodeptr          m_end;              // end of cache memory
	drccodeptr          m_codegen;          // start of generated code
	size_t              m_size;             // size of the cache in bytes

	// oob management
	struct oob_handler
	{
		oob_handler *next() const { return m_next; }

		oob_handler *   m_next;             // next handler
		drc_oob_delegate m_callback;        // callback function
		void *          m_param1;           // 1st pointer parameter
		void *          m_param2;           // 2nd pointer parameter
	};
	simple_list<oob_handler> m_ooblist;     // list of oob handlers

	// free lists
	struct free_link
	{
		free_link *         m_next;         // pointer to the next guy
	};
	free_link *         m_free[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
	free_link *         m_nearfree[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
};


#endif /* __DRCCACHE_H__ */
