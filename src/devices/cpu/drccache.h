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

#include <algorithm>
#include <cstdlib>
#include <list>
#include <optional>
#include <utility>


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


class drc_cache
{
public:
	// construction/destruction
	drc_cache(std::size_t bytes) noexcept;
	~drc_cache();

	// setup
	void set_size(std::size_t bytes) ATTR_COLD;
	void allocate_cache(bool rwx) ATTR_COLD;

	// getters
	drccodeptr near() const { return m_near; }
	drccodeptr top() const { return m_top; }

	// pointer checking
	bool contains_pointer(const void *ptr) const { return ((const drccodeptr)ptr >= m_near && (const drccodeptr)ptr < m_near + m_size); }
	bool contains_near_pointer(const void *ptr) const { return ((const drccodeptr)ptr >= m_near && (const drccodeptr)ptr < m_neartop); }
	bool generating_code() const { return (m_codegen != nullptr); }

	// cache memory allocation
	void flush() noexcept;
	void *alloc(std::size_t bytes, std::align_val_t align) noexcept;
	void *alloc_near(std::size_t bytes, std::align_val_t align) noexcept;
	void *alloc_temporary(std::size_t bytes, std::align_val_t align) noexcept;
	void dealloc(void *memory, std::size_t bytes) noexcept;

	template <typename T, typename... Params>
	T *alloc(Params &&... args)
	{
		auto const result = reinterpret_cast<T *>(alloc(sizeof(T), std::align_val_t(alignof(T))));
		if (result)
		{
			try
			{
				new (result) T(std::forward<Params>(args)...);
			}
			catch (...)
			{
				dealloc(result, sizeof(T));
				throw;
			}
		}
		return result;
	}

	template <typename T, typename... Params>
	T *alloc_near(Params &&... args)
	{
		auto const result = reinterpret_cast<T *>(alloc_near(sizeof(T), std::align_val_t(alignof(T))));
		if (result)
		{
			try
			{
				new (result) T(std::forward<Params>(args)...);
			}
			catch (...)
			{
				dealloc(result, sizeof(T));
				throw;
			}
		}
		return result;
	}

	// codegen helpers
	drccodeptr *begin_codegen(uint32_t reserve_bytes) noexcept;
	drccodeptr end_codegen();
	void request_oob_codegen(drc_oob_delegate &&callback, void *param1 = nullptr, void *param2 = nullptr);

	void codegen_complete() noexcept
	{
		if (!m_rwx && (m_top > m_rwbase))
			make_executable();
	}

private:
	// largest block of code that can be generated at once
	static constexpr std::size_t CODEGEN_MAX_BYTES = 128 * 1024;

	// minimum alignment, in bytes (must be power of 2)
	static constexpr std::size_t CACHE_ALIGNMENT = std::max<std::size_t>(alignof(std::max_align_t), 8);

	// largest permanent allocation we allow
	static constexpr std::size_t MAX_PERMANENT_ALLOC = 1024;

	// size of "near" area at the base of the cache
	static constexpr std::size_t NEAR_CACHE_SIZE = 128 * 1024;

	struct oob_handler
	{
		drc_oob_delegate    m_callback;     // callback function
		void *              m_param1;       // 1st pointer parameter
		void *              m_param2;       // 2nd pointer parameter
	};

	void ensure_writable(drccodeptr ptr) noexcept;
	void make_executable() noexcept;

	std::optional<osd::virtual_memory_allocation> m_cache;

	struct free_link
	{
		free_link *         m_next;         // pointer to the next guy
	};

	// core parameters
	drccodeptr  m_near;             // pointer to the near part of the cache
	drccodeptr  m_neartop;          // unallocated area of near cache
	drccodeptr  m_base;             // end of near cache
	drccodeptr  m_top;              // end of temporary allocations and code
	drccodeptr  m_rwbase;           // start of writable portion of cache
	drccodeptr  m_limit;            // limit for temporary allocations and code (page-aligned)
	drccodeptr  m_end;              // first allocated byte in cache
	drccodeptr  m_codegen;          // start of current generated code block
	std::size_t m_size;             // size of the cache in bytes
	bool        m_rwx;              // whether pages can be simultaneously writable and executable

	// oob management
	std::list<oob_handler> m_oob_list;      // list of active oob handlers
	std::list<oob_handler> m_oob_free;      // list of recyclable oob handlers

	// free lists
	free_link *         m_free[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
	free_link *         m_nearfree[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];

	// stats
	std::size_t m_max_temporary;
	uint32_t    m_flush_count;
#if defined(MAME_DEBUG)
	uint32_t    m_near_allocated;
	uint32_t    m_near_oversize;
	uint32_t    m_near_freed;
	uint32_t	m_near_reused;
	uint32_t    m_cache_allocated;
	uint32_t    m_cache_oversize;
	uint32_t    m_cache_freed;
	uint32_t    m_cache_reused;
#endif
};

#endif // MAME_CPU_DRCCACHE_H
