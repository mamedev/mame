// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drccache.cpp

    Universal dynamic recompiler cache management.

***************************************************************************/

#include "emu.h"
#include "drccache.h"

#include <algorithm>
#include <numeric>


namespace {

template <typename T, typename U>
constexpr T *ALIGN_PTR_UP(T *p, U align)
{
	return reinterpret_cast<T *>((uintptr_t(p) + (align - 1)) & ~uintptr_t(align - 1));
}

template <typename T, typename U>
constexpr T *ALIGN_PTR_DOWN(T *p, U align)
{
	return reinterpret_cast<T *>(uintptr_t(p) & ~uintptr_t(align - 1));
}

} // anonymous namespace



//**************************************************************************
//  DRC CACHE
//**************************************************************************

//-------------------------------------------------
//  construction/destruction
//-------------------------------------------------

drc_cache::drc_cache(std::size_t bytes) noexcept
	: m_near(nullptr)
	, m_neartop(nullptr)
	, m_base(nullptr)
	, m_top(nullptr)
	, m_limit(nullptr)
	, m_end(nullptr)
	, m_codegen(nullptr)
	, m_size(bytes)
	, m_executable(false)
	, m_rwx(false)
	, m_max_temporary(0)
	, m_flush_count(0)
#if defined(MAME_DEBUG)
	, m_near_allocated(0)
	, m_near_oversize(0)
	, m_near_freed(0)
	, m_near_reused(0)
	, m_cache_allocated(0)
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
			osd_printf_verbose("drc_cache: Statistics:\nFlush count %u, near cache use %u, permanent cache use %u/%u, maximum temporary cache use %u\n",
					m_flush_count,
					m_neartop - m_near,
					m_size - (m_end - m_near),
					m_size - (m_limit - m_near),
					m_max_temporary);
#if defined(MAME_DEBUG)
			osd_printf_verbose("Near cache allocated %u (%u oversize), freed %u reused %u\nPermanent cache allocated %u (%u oversize), freed %u reused %u\n",
					m_near_allocated,
					m_near_oversize,
					m_near_freed,
					m_near_reused,
					m_cache_allocated,
					m_cache_oversize,
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
	m_base = ALIGN_PTR_UP(m_near + NEAR_CACHE_SIZE, m_cache->page_size());
	m_top = m_base;
	m_limit = m_near + m_cache->size();
	m_end = m_limit;
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

void drc_cache::flush()
{
	// can't flush in the middle of codegen
	assert(!m_codegen);

	// just reset the top back to the base and re-seed
	m_max_temporary = std::max<std::size_t>(m_max_temporary, m_top - m_base);
	++m_flush_count;
	m_top = m_base;
	codegen_init();
}


//-------------------------------------------------
//  alloc - allocate permanent memory from the
//  cache
//-------------------------------------------------

void *drc_cache::alloc(std::size_t bytes, std::align_val_t align) noexcept
{
	assert(bytes);
	assert(std::size_t(align));

	if (UNEXPECTED((std::size_t(align) > CACHE_ALIGNMENT) || (CACHE_ALIGNMENT % std::size_t(align))))
		osd_printf_error("drc_cache: Requested alignment %u not satisfied by cache alignment %u\n", std::size_t(align), CACHE_ALIGNMENT);

	// pick first from the free list
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **const linkptr = &m_free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *const link = *linkptr;
		if (link)
		{
#if defined(MAME_DEBUG)
			++m_cache_allocated;
			++m_cache_reused;
#endif
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr const ptr = ALIGN_PTR_DOWN(m_end - bytes, CACHE_ALIGNMENT);
	drccodeptr const limit = ALIGN_PTR_DOWN(ptr, m_cache->page_size());
	if (m_top > limit)
		return nullptr;

	// otherwise update the end of the cache
#if defined(MAME_DEBUG)
	++m_cache_allocated;
	if (bytes >= MAX_PERMANENT_ALLOC)
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

	if (UNEXPECTED(((std::size_t(align) > CACHE_ALIGNMENT) || (CACHE_ALIGNMENT % std::size_t(align)))))
		osd_printf_error("drc_cache: Requested alignment %u not satisfied by cache alignment %u\n", std::size_t(align), CACHE_ALIGNMENT);

	// pick first from the free list
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **const linkptr = &m_nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *const link = *linkptr;
		if (link)
		{
#if defined(MAME_DEBUG)
			++m_near_allocated;
			++m_near_reused;
#endif
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr const ptr = ALIGN_PTR_UP(m_neartop, CACHE_ALIGNMENT);
	if ((ptr + bytes) > m_base)
		return nullptr;

	// otherwise update the top of the near part of the cache
#if defined(MAME_DEBUG)
	++m_near_allocated;
	if (bytes >= MAX_PERMANENT_ALLOC)
		++m_near_oversize;
#endif
	m_neartop = ptr + bytes;
	return ptr;
}


//-------------------------------------------------
//  alloc_temporary - allocate temporary memory
//  from the cache
//-------------------------------------------------

void *drc_cache::alloc_temporary(std::size_t bytes, std::align_val_t align) noexcept
{
	// can't allocate in the middle of codegen
	assert(!m_codegen);

	assert(bytes);
	assert(std::size_t(align));

	// if no space, we just fail
	drccodeptr const ptr = m_top;
	if ((ptr + bytes) > m_limit)
		return nullptr;

	// otherwise, update the cache top
	codegen_init();
	m_top = ALIGN_PTR_UP(ptr + bytes, std::lcm(std::size_t(align), CACHE_ALIGNMENT));
	return ptr;
}


//-------------------------------------------------
//  free - release permanent memory allocated from
//  the cache
//-------------------------------------------------

void drc_cache::dealloc(void *memory, std::size_t bytes) noexcept
{
	drccodeptr const mem = reinterpret_cast<drccodeptr>(memory);
	assert(bytes < MAX_PERMANENT_ALLOC);
	assert(((mem >= m_near) && (mem < m_base)) || ((mem >= m_end) && (mem < (m_near + m_size))));

	// determine which free list to add to
	free_link **const linkptr = &((mem < m_base) ? m_nearfree : m_free)[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
#if defined(MAME_DEBUG)
	++((mem < m_base) ? m_near_freed : m_cache_freed);
#endif

	// link is into the free list for our size
	free_link *const link = reinterpret_cast<free_link *>(memory);
	link->m_next = *linkptr;
	*linkptr = link;
}


void drc_cache::codegen_init() noexcept
{
	if (m_executable)
	{
		if (!m_rwx)
			m_cache->set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
		m_executable = false;
	}
}


void drc_cache::codegen_complete() noexcept
{
	if (!m_executable)
	{
		if (!m_rwx)
			m_cache->set_access(m_base - m_near, ALIGN_PTR_UP(m_top, m_cache->page_size()) - m_base, osd::virtual_memory_allocation::READ_EXECUTE);
		m_executable = true;
	}
}


//-------------------------------------------------
//  begin_codegen - begin code generation
//-------------------------------------------------

drccodeptr *drc_cache::begin_codegen(uint32_t reserve_bytes) noexcept
{
	// can't restart in the middle of codegen
	assert(!m_codegen);
	assert(m_oob_list.empty());

	// if no space, we just fail
	if ((m_top + reserve_bytes) > m_limit)
		return nullptr;

	// otherwise, return a pointer to the cache top
	m_codegen = m_top;
	return &m_top;
}


//-------------------------------------------------
//  end_codegen - complete code generation
//-------------------------------------------------

drccodeptr drc_cache::end_codegen()
{
	drccodeptr const result = m_codegen;

	// run the OOB handlers
	while (!m_oob_list.empty())
	{
		// call the callback
		m_oob_list.front().m_callback(&m_top, m_oob_list.front().m_param1, m_oob_list.front().m_param2);
		assert((m_top - m_codegen) < CODEGEN_MAX_BYTES);

		// add it to the free list
		m_oob_free.splice(m_oob_free.begin(), m_oob_list, m_oob_list.begin());
	}

	// update the cache top
	osd::invalidate_instruction_cache(m_codegen, m_top - m_codegen);
	m_top = ALIGN_PTR_UP(m_top, CACHE_ALIGNMENT);
	m_codegen = nullptr;

	return result;
}


//-------------------------------------------------
//  request_oob_codegen - request callback for
//  out-of-band codegen
//-------------------------------------------------

void drc_cache::request_oob_codegen(drc_oob_delegate &&callback, void *param1, void *param2)
{
	assert(m_codegen);

	// pull an item from the free list
	std::list<oob_handler>::iterator oob;
	if (m_oob_free.empty())
	{
		oob = m_oob_list.emplace(m_oob_list.end());
	}
	else
	{
		oob = m_oob_free.begin();
		m_oob_list.splice(m_oob_list.end(), m_oob_free, oob);
	}

	// fill it in
	oob->m_callback = std::move(callback);
	oob->m_param1 = param1;
	oob->m_param2 = param2;
}
