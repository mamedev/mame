// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drccache.h

    Universal dynamic recompiler cache management.

***************************************************************************/
#ifndef MAME_CPU_DRCCACHE_H
#define MAME_CPU_DRCCACHE_H

#pragma once

#include "modules/lib/osdlib.h"


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
typedef uint8_t *drccodeptr;


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
	bool generating_code() const { return (m_codegen != nullptr); }

	// memory management
	void flush();
	void *alloc(size_t bytes);
	void *alloc_near(size_t bytes);
	void *alloc_temporary(size_t bytes);
	void dealloc(void *memory, size_t bytes);

	// codegen helpers
	void codegen_init();
	void codegen_complete();
	drccodeptr *begin_codegen(uint32_t reserve_bytes);
	drccodeptr end_codegen();
	void request_oob_codegen(drc_oob_delegate &&callback, void *param1 = nullptr, void *param2 = nullptr);

private:
	// largest block of code that can be generated at once
	static constexpr size_t CODEGEN_MAX_BYTES = 131072;

	// minimum alignment, in bytes (must be power of 2)
	static constexpr size_t CACHE_ALIGNMENT = alignof(std::max_align_t);

	// largest permanent allocation we allow
	static constexpr size_t MAX_PERMANENT_ALLOC = 1024;

	// size of "near" area at the base of the cache
	static constexpr size_t NEAR_CACHE_SIZE = 131072;

	osd::virtual_memory_allocation m_cache;

	// core parameters
	drccodeptr const    m_near;             // pointer to the near part of the cache
	drccodeptr          m_neartop;          // unallocated area of near cache
	drccodeptr const    m_base;             // end of near cache
	drccodeptr          m_top;              // end of temporary allocations and code
	drccodeptr          m_limit;            // limit for temporary allocations and code (page-aligned)
	drccodeptr          m_end;              // first allocated byte in cache
	drccodeptr          m_codegen;          // start of current generated code block
	size_t const        m_size;             // size of the cache in bytes
	bool                m_executable;       // whether cached code is currently executable

	// oob management
	struct oob_handler
	{
		drc_oob_delegate    m_callback;     // callback function
		void *              m_param1;       // 1st pointer parameter
		void *              m_param2;       // 2nd pointer parameter
	};
	std::list<oob_handler> m_oob_list;      // list of active oob handlers
	std::list<oob_handler> m_oob_free;      // list of recyclable oob handlers

	// free lists
	struct free_link
	{
		free_link *         m_next;         // pointer to the next guy
	};
	free_link *         m_free[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
	free_link *         m_nearfree[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
};

#endif // MAME_CPU_DRCCACHE_H
