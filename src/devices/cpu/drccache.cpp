// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drccache.cpp

    Universal dynamic recompiler cache management.

***************************************************************************/

#include "emu.h"
#include "drccache.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>


namespace {

template <typename T, typename U>
constexpr T *align_ptr_up(T *p, U align)
{
	return !(align & (align - 1))
			? reinterpret_cast<T *>((uintptr_t(p) + (align - 1)) & ~uintptr_t(align - 1))
			: reinterpret_cast<T *>(((uintptr_t(p) + (align - 1)) / align) * align);
}

template <typename T, typename U>
constexpr T *align_ptr_down(T *p, U align)
{
	return !(align & (align - 1))
			? reinterpret_cast<T *>(uintptr_t(p) & ~uintptr_t(align - 1))
			: reinterpret_cast<T *>((uintptr_t(p) / align) * align);
}

template <typename T, typename U>
constexpr bool is_ptr_aligned(T *p, U align)
{
	return !(align & (align - 1))
			? !(uintptr_t(p) & (uintptr_t(align) - 1))
			: !(uintptr_t(p) % uintptr_t(p));
}

} // anonymous namespace



//**************************************************************************
//  DRC CACHE
//**************************************************************************

//-------------------------------------------------
//  helpers
//-------------------------------------------------

inline std::size_t drc_cache::free_link_bucket(std::size_t bytes) noexcept
{
	return ((bytes + (CACHE_ALIGNMENT - 1)) / CACHE_ALIGNMENT) - 1;
}


inline void drc_cache::ensure_writable(drccodeptr ptr) noexcept
{
	if (!m_rwx && (ptr < m_rwbase))
	{
		drccodeptr const top = align_ptr_down(ptr, m_cache->page_size());
		assert(top >= m_base);
		m_cache->set_access(top - m_near, m_rwbase - top, osd::virtual_memory_allocation::READ_WRITE);
		m_rwbase = top;
	}
}


void drc_cache::make_executable() noexcept
{
	drccodeptr const top = align_ptr_up(m_top, m_cache->page_size());
	assert(top <= m_limit);
	m_cache->set_access(m_rwbase - m_near, top - m_rwbase, osd::virtual_memory_allocation::READ_EXECUTE);
	m_rwbase = top;
}


//-------------------------------------------------
//  construction/destruction
//-------------------------------------------------

drc_cache::drc_cache(std::size_t bytes) noexcept
	: m_near(nullptr)
	, m_neartop(nullptr)
	, m_base(nullptr)
	, m_invartop(nullptr)
	, m_top(nullptr)
	, m_rwbase(nullptr)
	, m_limit(nullptr)
	, m_end(nullptr)
	, m_codegen(nullptr)
	, m_size(bytes)
	, m_rwx(false)
	, m_invargen(false)
	, m_max_temporary(0)
	, m_flush_count(0)
#if defined(MAME_DEBUG)
	, m_near_allocated(0)
	, m_near_padding(0)
	, m_near_oversize(0)
	, m_near_freed(0)
	, m_near_reused(0)
	, m_cache_allocated(0)
	, m_cache_padding(0)
	, m_cache_oversize(0)
	, m_cache_freed(0)
	, m_cache_reused(0)
#endif
{
	// alignment must be power of two
	static_assert(!(CACHE_ALIGNMENT & (CACHE_ALIGNMENT - 1)));

	std::fill(std::begin(m_free), std::end(m_free), nullptr);
	std::fill(std::begin(m_nearfree), std::end(m_nearfree), nullptr);
}

drc_cache::~drc_cache()
{
	if (m_cache)
	{
		try
		{
			m_max_temporary = std::max<std::size_t>(m_max_temporary, m_top - m_invartop);
			osd_printf_verbose(
					"drc_cache: Statistics:\nFlush count %u, near cache use %u, permanent cache use %u/%u, invariant cache use %u, maximum transient cache use %u\n",
					m_flush_count,
					m_neartop - m_near,
					m_size - (m_end - m_near),
					m_size - (m_limit - m_near),
					m_invartop - m_base,
					m_max_temporary);
#if defined(MAME_DEBUG)
			osd_printf_verbose(
					"Near cache allocated %u (%u oversize, %u alignment padding), freed %u, reused %u\n"
					"Permanent cache allocated %u (%u oversize, %u alignment padding), freed %u, reused %u\n",
					m_near_allocated,
					m_near_oversize,
					m_near_padding,
					m_near_freed,
					m_near_reused,
					m_cache_allocated,
					m_cache_oversize,
					m_cache_padding,
					m_cache_freed,
					m_cache_reused);
#endif
		}
		catch (...)
		{
			// ignore exceptions dumping statistics
		}
	}
}


//-------------------------------------------------
//  setup
//-------------------------------------------------

void drc_cache::set_size(std::size_t bytes)
{
	if (m_cache)
		throw emu_fatalerror("drc_cache: Cannot reconfigure size after allocating");

	if (bytes < NEAR_CACHE_SIZE)
		throw emu_fatalerror("drc_cache: Requested size %u is smaller than near cache size %u", bytes, NEAR_CACHE_SIZE);

	m_size = bytes;
}

void drc_cache::allocate_cache(bool rwx)
{
	if (m_cache)
		throw emu_fatalerror("drc_cache: Cannot reallocate cache");

	m_cache.emplace({ NEAR_CACHE_SIZE, m_size - NEAR_CACHE_SIZE }, osd::virtual_memory_allocation::READ_WRITE_EXECUTE);
	m_near = reinterpret_cast<drccodeptr>(m_cache->get());
	m_neartop = m_near;
	m_base = align_ptr_up(m_near + NEAR_CACHE_SIZE, m_cache->page_size());
	m_invartop = m_base;
	m_top = m_base;
	m_limit = m_near + m_cache->size();
	m_end = m_limit;
	m_rwbase = m_base;
	m_size = m_cache->size();
	m_rwx = false;

	if (!*m_cache)
	{
		throw emu_fatalerror("drc_cache: Error allocating virtual memory");
	}
	else if (!m_cache->set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE))
	{
		throw emu_fatalerror("drc_cache: Error marking cache read/write");
	}
	else if (rwx && m_cache->set_access(m_base - m_near, m_end - m_base, osd::virtual_memory_allocation::READ_WRITE_EXECUTE))
	{
		osd_printf_verbose("drc_cache: RWX pages supported\n");
		m_rwx = true;
	}
	else
	{
		osd_printf_verbose("drc_cache: Using W^X mode\n");
		m_rwx = false;
	}

	// page size must be power of two, cache must be page-aligned
	assert(!(m_cache->page_size() & (m_cache->page_size() - 1)));
	assert(!(uintptr_t(m_near) & (m_cache->page_size() - 1)));
	assert(m_cache->page_size() >= CACHE_ALIGNMENT);
}


//-------------------------------------------------
//  flush - flush the cache contents
//-------------------------------------------------

void drc_cache::flush() noexcept
{
	// can't flush in the middle of codegen
	assert(!m_codegen);

	// just reset the top back to the base and re-seed
	m_max_temporary = std::max<std::size_t>(m_max_temporary, m_top - m_invartop);
	++m_flush_count;
	m_top = m_invartop;
}


//-------------------------------------------------
//  alloc - allocate permanent memory from the
//  cache
//-------------------------------------------------

void *drc_cache::alloc(std::size_t bytes, std::align_val_t align) noexcept
{
	assert(bytes);
	assert(std::size_t(align));

	// pick first from the free list
	std::size_t const bucket = free_link_bucket(bytes);
	if (bucket < m_free.size())
	{
		free_link **link;
		for (link = &m_free[bucket]; *link; link = &(*link)->m_next)
		{
			if (is_ptr_aligned(*link, std::size_t(align)))
				break;
		}

		if (*link)
		{
#if defined(MAME_DEBUG)
			++m_cache_allocated;
			++m_cache_reused;
#endif
			return std::exchange(*link, (*link)->m_next);
		}
	}

	// if no space, we just fail
	drccodeptr const ptr = align_ptr_down(m_end - bytes, std::lcm(std::size_t(align), CACHE_ALIGNMENT));
	drccodeptr const limit = align_ptr_down(ptr, m_cache->page_size());
	if (m_top > limit)
		return nullptr;

	drccodeptr const end = align_ptr_up(ptr + bytes, CACHE_ALIGNMENT);
	if (end < m_end)
	{
#if defined(MAME_DEBUG)
		++m_cache_padding;
#endif
		std::size_t const padbucket = free_link_bucket(m_end - end);
		if (padbucket < m_free.size())
		{
			free_link *const link = reinterpret_cast<free_link *>(end);
			link->m_next = m_free[bucket];
			m_free[bucket] = link;
		}
	}

	// otherwise update the end of the cache
#if defined(MAME_DEBUG)
	++m_cache_allocated;
	if (bucket >= m_free.size())
		++m_cache_oversize;
#endif
	m_limit = limit;
	m_end = ptr;
	return ptr;
}


//-------------------------------------------------
//  alloc_near - allocate permanent memory from
//  the near part of the cache
//-------------------------------------------------

void *drc_cache::alloc_near(std::size_t bytes, std::align_val_t align) noexcept
{
	assert(bytes);
	assert(std::size_t(align));

	// pick first from the free list
	std::size_t const bucket = free_link_bucket(bytes);
	if (bucket < m_nearfree.size())
	{
		free_link **link;
		for (link = &m_nearfree[bucket]; *link; link = &(*link)->m_next)
		{
			if (is_ptr_aligned(*link, std::size_t(align)))
				break;
		}

		if (*link)
		{
#if defined(MAME_DEBUG)
			++m_near_allocated;
			++m_near_reused;
#endif
			return std::exchange(*link, (*link)->m_next);
		}
	}

	// if no space, we just fail
	drccodeptr const top = align_ptr_up(m_neartop, CACHE_ALIGNMENT);
	drccodeptr const ptr = align_ptr_up(m_neartop, std::lcm(std::size_t(align), CACHE_ALIGNMENT));
	assert(ptr >= top);
	if ((ptr + bytes) > m_base)
		return nullptr;

	// add alignment padding to the free list
	if (ptr > top)
	{
#if defined(MAME_DEBUG)
		++m_near_padding;
#endif
		std::size_t const padbucket = free_link_bucket(ptr - top);
		if (padbucket < m_nearfree.size())
		{
			free_link *const link = reinterpret_cast<free_link *>(top);
			link->m_next = m_nearfree[bucket];
			m_nearfree[bucket] = link;
		}
	}

	// update the top of the near part of the cache
#if defined(MAME_DEBUG)
	++m_near_allocated;
	if (bucket >= m_nearfree.size())
		++m_near_oversize;
#endif
	m_neartop = ptr + bytes;
	return ptr;
}


//-------------------------------------------------
//  alloc_invariant - allocate invariant memory
//  from the cache
//-------------------------------------------------

void *drc_cache::alloc_invariant(std::size_t bytes, std::align_val_t align) noexcept
{
	if (UNEXPECTED(m_top > m_invartop))
	{
		osd_printf_error("drc_cache: cannot allocate invariant memory after allocating transient memory\n");
		std::abort();
	}

	auto const result = alloc_transient(bytes, align);
	m_invartop = m_top;
	return result;
}


//-------------------------------------------------
//  alloc_transient - allocate transient memory
//  from the cache
//-------------------------------------------------

void *drc_cache::alloc_transient(std::size_t bytes, std::align_val_t align) noexcept
{
	// can't allocate in the middle of codegen
	assert(!m_codegen);

	assert(bytes);
	assert(std::size_t(align));

	// if no space, we just fail
	drccodeptr const ptr = align_ptr_up(m_top, std::size_t(align));
	drccodeptr const end = align_ptr_up(ptr + bytes, CACHE_ALIGNMENT);
	if (end > m_limit)
		return nullptr;

	// otherwise, update the cache top
	m_top = end;
	ensure_writable(ptr);
	return ptr;
}


//-------------------------------------------------
//  free - release permanent memory allocated from
//  the cache
//-------------------------------------------------

void drc_cache::dealloc(void *memory, std::size_t bytes) noexcept
{
	drccodeptr const mem = reinterpret_cast<drccodeptr>(memory);
	assert(((mem >= m_near) && (mem < m_base)) || ((mem >= m_end) && (mem < (m_near + m_size))));

	// determine which free list to add to
	auto &freelist = (mem < m_base) ? m_nearfree : m_free;
	std::size_t const bucket = free_link_bucket(bytes);
	if (bucket >= freelist.size())
		return; // oversize allocations just leak

#if defined(MAME_DEBUG)
	++((mem < m_base) ? m_near_freed : m_cache_freed);
#endif

	// link is into the free list for our size
	free_link *const link = reinterpret_cast<free_link *>(memory);
	link->m_next = freelist[bucket];
	freelist[bucket] = link;
}


//-------------------------------------------------
//  begin_codegen - begin code generation
//-------------------------------------------------

drccodeptr *drc_cache::begin_codegen(uint32_t reserve_bytes) noexcept
{
	// can't restart in the middle of codegen
	assert(!m_codegen);

	// if no space, we just fail
	if ((m_top + reserve_bytes) > m_limit)
		return nullptr;

	// otherwise, return a pointer to the cache top
	m_codegen = m_top;
	ensure_writable(m_top);
	return &m_top;
}

drccodeptr *drc_cache::begin_codegen_invariant(uint32_t reserve_bytes) noexcept
{
	if (UNEXPECTED(m_top > m_invartop))
	{
		osd_printf_error("drc_cache: cannot allocate invariant memory after allocating transient memory\n");
		std::abort();
	}

	auto const result = begin_codegen(reserve_bytes);
	m_invargen = true;
	return result;
}


//-------------------------------------------------
//  end_codegen - complete code generation
//-------------------------------------------------

drccodeptr drc_cache::end_codegen()
{
	drccodeptr const result = m_codegen;

	// update the cache top
	osd::invalidate_instruction_cache(m_codegen, m_top - m_codegen);
	m_top = align_ptr_up(m_top, CACHE_ALIGNMENT);
	if (m_invargen)
		m_invartop = m_top;
	m_codegen = nullptr;
	m_invargen = false;

	return result;
}
