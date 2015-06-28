// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.c
 *
 */

#include "pconfig.h"
#include "palloc.h"

#if (PSTANDALONE)
#include <stdlib.h>
#include <xmmintrin.h>

class pmemory_pool
{
public:
	pmemory_pool() {}
};

static pmemory_pool sppool;

pmemory_pool *ppool = &sppool;

void* operator new(std::size_t size, pmemory_pool *pool) throw (std::bad_alloc)
{
	//printf("here new\n");
	return palloc_raw(size);;
}

void operator delete(void *ptr, pmemory_pool *pool)
{
	//printf("here delete\n");
	if (ptr != NULL)
		pfree_raw(ptr);
}

void *palloc_raw(const size_t size)
{
	return _mm_malloc(size, 64);
}

void pfree_raw(void *p)
{
	_mm_free(p);
}
#endif
