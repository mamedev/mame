// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drccache.c

    Universal dynamic recompiler cache management.

***************************************************************************/

#include "emu.h"
#include "drccache.h"

#include <algorithm>


namespace {

template <typename T, typename U> constexpr T *ALIGN_PTR_UP(T *p, U align)
{
	return reinterpret_cast<T *>((uintptr_t(p) + (align - 1)) & ~uintptr_t(align - 1));
}

template <typename T, typename U> constexpr T *ALIGN_PTR_DOWN(T *p, U align)
{
	return reinterpret_cast<T *>(uintptr_t(p) & ~uintptr_t(align - 1));
}

} // anonymous namespace



//**************************************************************************
//  DRC CACHE
//**************************************************************************

//-------------------------------------------------
//  drc_cache - constructor
//-------------------------------------------------

drc_cache::drc_cache(size_t bytes) :
	m_cache({ NEAR_CACHE_SIZE, bytes - NEAR_CACHE_SIZE }),
	m_near(reinterpret_cast<drccodeptr>(m_cache.get())),
	m_neartop(m_near),
	m_base(ALIGN_PTR_UP(m_near + NEAR_CACHE_SIZE, m_cache.page_size())),
	m_top(m_base),
	m_limit(m_near + m_cache.size()),
	m_end(m_limit),
	m_codegen(nullptr),
	m_size(m_cache.size()),
	m_executable(false)
{
	// alignment and page size must be powers of two, cache must be page-aligned
	assert(!(CACHE_ALIGNMENT & (CACHE_ALIGNMENT - 1)));
	assert(!(m_cache.page_size() & (m_cache.page_size() - 1)));
	assert(!(uintptr_t(m_near) & (m_cache.page_size() - 1)));
	assert(m_cache.page_size() >= CACHE_ALIGNMENT);

	std::fill(std::begin(m_free), std::end(m_free), nullptr);
	std::fill(std::begin(m_nearfree), std::end(m_nearfree), nullptr);

	m_cache.set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
}


//-------------------------------------------------
//  ~drc_cache - destructor
//-------------------------------------------------

drc_cache::~drc_cache()
{
}



//-------------------------------------------------
//  flush - flush the cache contents
//-------------------------------------------------

void drc_cache::flush()
{
	// can't flush in the middle of codegen
	assert(!m_codegen);

	// just reset the top back to the base and re-seed
	m_top = m_base;
	if (m_executable)
	{
		m_cache.set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
		m_executable = false;
	}
}


//-------------------------------------------------
//  alloc - allocate permanent memory from the
//  cache
//-------------------------------------------------

void *drc_cache::alloc(size_t bytes)
{
	assert(bytes > 0);

	// pick first from the free list
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **const linkptr = &m_free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *const link = *linkptr;
		if (link)
		{
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr const ptr = ALIGN_PTR_DOWN(m_end - bytes, CACHE_ALIGNMENT);
	drccodeptr const limit = ALIGN_PTR_DOWN(ptr, m_cache.page_size());
	if (m_top > limit)
		return nullptr;

	// otherwise update the end of the cache
	m_limit = limit;
	m_end = ptr;
	return ptr;
}


//-------------------------------------------------
//  alloc_near - allocate permanent memory from
//  the near part of the cache
//-------------------------------------------------

void *drc_cache::alloc_near(size_t bytes)
{
	assert(bytes > 0);

	// pick first from the free list
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **const linkptr = &m_nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *const link = *linkptr;
		if (link)
		{
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr const ptr = ALIGN_PTR_UP(m_neartop, CACHE_ALIGNMENT);
	if ((ptr + bytes) > m_base)
		return nullptr;

	// otherwise update the top of the near part of the cache
	m_neartop = ptr + bytes;
	return ptr;
}


//-------------------------------------------------
//  alloc_temporary - allocate temporary memory
//  from the cache
//-------------------------------------------------

void *drc_cache::alloc_temporary(size_t bytes)
{
	// can't allocate in the middle of codegen
	assert(!m_codegen);

	// if no space, we just fail
	drccodeptr const ptr = m_top;
	if ((ptr + bytes) > m_limit)
		return nullptr;

	// otherwise, update the cache top
	if (m_executable)
	{
		m_cache.set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
		m_executable = false;
	}
	m_top = ALIGN_PTR_UP(ptr + bytes, CACHE_ALIGNMENT);
	return ptr;
}


//-------------------------------------------------
//  free - release permanent memory allocated from
//  the cache
//-------------------------------------------------

void drc_cache::dealloc(void *memory, size_t bytes)
{
	drccodeptr const mem = reinterpret_cast<drccodeptr>(memory);
	assert(bytes < MAX_PERMANENT_ALLOC);
	assert(((mem >= m_near) && (mem < m_base)) || ((mem >= m_end) && (mem < (m_near + m_size))));

	// determine which free list to add to
	free_link **const linkptr = &((mem < m_base) ? m_nearfree : m_free)[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];

	// link is into the free list for our size
	free_link *const link = reinterpret_cast<free_link *>(memory);
	link->m_next = *linkptr;
	*linkptr = link;
}


void drc_cache::codegen_init()
{
	if (m_executable)
	{
		m_cache.set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
		m_executable = false;
	}
}


void drc_cache::codegen_complete()
{
	if (!m_executable)
	{
		m_cache.set_access(m_base - m_near, ALIGN_PTR_UP(m_top, m_cache.page_size()) - m_base, osd::virtual_memory_allocation::READ_EXECUTE);
		m_executable = true;
	}
}


//-------------------------------------------------
//  begin_codegen - begin code generation
//-------------------------------------------------

drccodeptr *drc_cache::begin_codegen(uint32_t reserve_bytes)
{
	// can't restart in the middle of codegen
	assert(!m_codegen);
	assert(m_oob_list.empty());

	// if no space, we just fail
	if ((m_top + reserve_bytes) > m_limit)
		return nullptr;

	// otherwise, return a pointer to the cache top
	if (m_executable)
	{
		m_cache.set_access(0, m_size, osd::virtual_memory_allocation::READ_WRITE);
		m_executable = false;
	}
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
