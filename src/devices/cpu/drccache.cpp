// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drccache.c

    Universal dynamic recompiler cache management.

***************************************************************************/

#include "emu.h"
#include "drccache.h"



//**************************************************************************
//  MACROS
//**************************************************************************

// ensure that all memory allocated is aligned to an 8-byte boundary
#define ALIGN_PTR_UP(p)         ((void *)(((uintptr_t)(p) + (CACHE_ALIGNMENT - 1)) & ~(CACHE_ALIGNMENT - 1)))
#define ALIGN_PTR_DOWN(p)       ((void *)((uintptr_t)(p) & ~(CACHE_ALIGNMENT - 1)))



//**************************************************************************
//  DRC CACHE
//**************************************************************************

//-------------------------------------------------
//  drc_cache - constructor
//-------------------------------------------------

drc_cache::drc_cache(size_t bytes)
	: m_near((drccodeptr)osd_alloc_executable(bytes)),
		m_neartop(m_near),
		m_base(m_near + NEAR_CACHE_SIZE),
		m_top(m_base),
		m_end(m_near + bytes),
		m_codegen(nullptr),
		m_size(bytes)
{
	memset(m_free, 0, sizeof(m_free));
	memset(m_nearfree, 0, sizeof(m_nearfree));
}


//-------------------------------------------------
//  ~drc_cache - destructor
//-------------------------------------------------

drc_cache::~drc_cache()
{
	// release the memory
	osd_free_executable(m_near, m_size);
}



//-------------------------------------------------
//  flush - flush the cache contents
//-------------------------------------------------

void drc_cache::flush()
{
	// can't flush in the middle of codegen
	assert(m_codegen == nullptr);

	// just reset the top back to the base and re-seed
	m_top = m_base;
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
		free_link **linkptr = &m_free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *link = *linkptr;
		if (link != nullptr)
		{
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr ptr = (drccodeptr)ALIGN_PTR_DOWN(m_end - bytes);
	if (m_top > ptr)
		return nullptr;

	// otherwise update the end of the cache
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
		free_link **linkptr = &m_nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *link = *linkptr;
		if (link != nullptr)
		{
			*linkptr = link->m_next;
			return link;
		}
	}

	// if no space, we just fail
	drccodeptr ptr = (drccodeptr)ALIGN_PTR_UP(m_neartop);
	if (ptr + bytes > m_base)
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
	assert(m_codegen == nullptr);

	// if no space, we just fail
	drccodeptr ptr = m_top;
	if (ptr + bytes >= m_end)
		return nullptr;

	// otherwise, update the cache top
	m_top = (drccodeptr)ALIGN_PTR_UP(ptr + bytes);
	return ptr;
}


//-------------------------------------------------
//  free - release permanent memory allocated from
//  the cache
//-------------------------------------------------

void drc_cache::dealloc(void *memory, size_t bytes)
{
	assert(bytes < MAX_PERMANENT_ALLOC);
	assert(((drccodeptr)memory >= m_near && (drccodeptr)memory < m_base) || ((drccodeptr)memory >= m_end && (drccodeptr)memory < m_near + m_size));

	// determine which free list to add to
	free_link **linkptr;
	if ((drccodeptr)memory < m_base)
		linkptr = &m_nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
	else
		linkptr = &m_free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];

	// link is into the free list for our size
	free_link *link = (free_link *)memory;
	link->m_next = *linkptr;
	*linkptr = link;
}


//-------------------------------------------------
//  begin_codegen - begin code generation
//-------------------------------------------------

drccodeptr *drc_cache::begin_codegen(uint32_t reserve_bytes)
{
	// can't restart in the middle of codegen
	assert(m_codegen == nullptr);
	assert(m_ooblist.first() == nullptr);

	// if still no space, we just fail
	drccodeptr ptr = m_top;
	if (ptr + reserve_bytes >= m_end)
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
	drccodeptr result = m_codegen;

	// run the OOB handlers
	oob_handler *oob;
	while ((oob = m_ooblist.detach_head()) != nullptr)
	{
		// call the callback
		oob->m_callback(&m_top, oob->m_param1, oob->m_param2);
		assert(m_top - m_codegen < CODEGEN_MAX_BYTES);

		// release our memory
		oob->~oob_handler();
		dealloc(oob, sizeof(*oob));
	}

	// update the cache top
	m_top = (drccodeptr)ALIGN_PTR_UP(m_top);
	m_codegen = nullptr;

	return result;
}


//-------------------------------------------------
//  request_oob_codegen - request callback for
//  out-of-band codegen
//-------------------------------------------------

void drc_cache::request_oob_codegen(drc_oob_delegate callback, void *param1, void *param2)
{
	assert(m_codegen != nullptr);

	// pull an item from the free list
	oob_handler *oob = (oob_handler *)alloc(sizeof(*oob));
	assert(oob != nullptr);
	new (oob) oob_handler();

	// fill it in
	oob->m_callback = std::move(callback);
	oob->m_param1 = param1;
	oob->m_param2 = param2;

	// add to the tail
	m_ooblist.append(*oob);
}
