// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.c
 *
 */

#include "pconfig.h"

#if (PSTANDALONE)
#include <stdlib.h>
#include <xmmintrin.h>

void *palloc_raw(const size_t size)
{
	return _mm_malloc(size, 64);
}

void pfree_raw(void *p)
{
	_mm_free(p);
}
#endif
