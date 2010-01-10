/***************************************************************************

    drccache.h

    Universal dynamic recompiler cache management.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "drccache.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* largest block of code that can be generated at once */
#define CODEGEN_MAX_BYTES		65536

/* minimum alignment, in bytes (must be power of 2) */
#define CACHE_ALIGNMENT			8

/* largest permanent allocation we allow */
#define MAX_PERMANENT_ALLOC		1024

/* size of "near" area at the base of the cache */
#define NEAR_CACHE_SIZE			65536



/***************************************************************************
    MACROS
***************************************************************************/

/* ensure that all memory allocated is aligned to an 8-byte boundary */
#define ALIGN_PTR_UP(p)			((void *)(((FPTR)(p) + (CACHE_ALIGNMENT - 1)) & ~(CACHE_ALIGNMENT - 1)))
#define ALIGN_PTR_DOWN(p)		((void *)((FPTR)(p) & ~(CACHE_ALIGNMENT - 1)))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* out-of-bounds codegen handlers */
typedef struct _oob_handler oob_handler;
struct _oob_handler
{
	oob_handler *		next;				/* next handler */
	drccache_oob_func	callback;			/* callback function */
	void *				param1;				/* 1st pointer parameter */
	void *				param2;				/* 2nd pointer parameter */
	void *				param3;				/* 3rd pointer parameter */
};


/* a linked list of free items */
typedef struct _free_link free_link;
struct _free_link
{
	free_link *			next;				/* pointer to the next guy */
};


/* cache state */
struct _drccache
{
	/* core parameters */
	drccodeptr			near;				/* pointer to the near part of the cache */
	drccodeptr			neartop;			/* top of the near part of the cache */
	drccodeptr			base;				/* base pointer to the compiler cache */
	drccodeptr			top;				/* current top of cache */
	drccodeptr			end;				/* end of cache memory */
	drccodeptr			codegen;			/* start of generated code */
	size_t				size;				/* size of the cache in bytes */

	/* oob management */
	oob_handler *		ooblist;			/* list of oob handlers */
	oob_handler **		oobtail;			/* pointer to tail oob pointer */

	/* free lists */
	free_link *			free[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
	free_link *			nearfree[MAX_PERMANENT_ALLOC / CACHE_ALIGNMENT];
};



/***************************************************************************
    INITIALIZATION/TEARDOWN
***************************************************************************/

/*-------------------------------------------------
    drccache_alloc - allocate the cache itself
-------------------------------------------------*/

drccache *drccache_alloc(size_t bytes)
{
	drccache cache, *cacheptr;

	assert(bytes >= sizeof(cache) + NEAR_CACHE_SIZE);

	/* build a local structure first */
	memset(&cache, 0, sizeof(cache));
	cache.near = (drccodeptr)osd_alloc_executable(bytes);
	cache.neartop = cache.near;
	cache.base = cache.near + NEAR_CACHE_SIZE;
	cache.top = cache.base;
	cache.end = cache.near + bytes;
	cache.size = bytes;

	/* now allocate the cache structure itself from that */
	cacheptr = (drccache *)drccache_memory_alloc(&cache, sizeof(cache));
	*cacheptr = cache;

	/* return the allocated result */
	return cacheptr;
}


/*-------------------------------------------------
    drccache_free - free the cache
-------------------------------------------------*/

void drccache_free(drccache *cache)
{
	/* release the memory; this includes the cache object itself */
	osd_free_executable(cache->near, cache->size);
}



/***************************************************************************
    CACHE INFORMATION
***************************************************************************/

/*-------------------------------------------------
    drccache_contains_pointer - return true if a
    pointer is within the cache
-------------------------------------------------*/

int drccache_contains_pointer(drccache *cache, const void *ptr)
{
	return ((const drccodeptr)ptr >= cache->near && (const drccodeptr)ptr < cache->near + cache->size);
}


/*-------------------------------------------------
    drccache_contains_near_pointer - return true
    if a pointer is within the cache
-------------------------------------------------*/

int drccache_contains_near_pointer(drccache *cache, const void *ptr)
{
	return ((const drccodeptr)ptr >= cache->near && (const drccodeptr)ptr < cache->neartop);
}


/*-------------------------------------------------
    drccache_near - return a pointer to the near
    part of the cache
-------------------------------------------------*/

drccodeptr drccache_near(drccache *cache)
{
	return cache->near;
}


/*-------------------------------------------------
    drccache_base - return a pointer to the base
    of the cache
-------------------------------------------------*/

drccodeptr drccache_base(drccache *cache)
{
	return cache->base;
}


/*-------------------------------------------------
    drccache_top - return the current top of the
    cache
-------------------------------------------------*/

drccodeptr drccache_top(drccache *cache)
{
	return cache->top;
}



/***************************************************************************
    MEMORY MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    drccache_flush - flush the cache contents
-------------------------------------------------*/

void drccache_flush(drccache *cache)
{
	/* can't flush in the middle of codegen */
	assert(cache->codegen == NULL);

	/* just reset the top back to the base and re-seed */
	cache->top = cache->base;
}


/*-------------------------------------------------
    drccache_memory_alloc - allocate permanent
    memory from the cache
-------------------------------------------------*/

void *drccache_memory_alloc(drccache *cache, size_t bytes)
{
	drccodeptr ptr;

	assert(bytes > 0);

	/* pick first from the free list */
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **linkptr = &cache->free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *link = *linkptr;
		if (link != NULL)
		{
			*linkptr = link->next;
			return link;
		}
	}

	/* if no space, we just fail */
	ptr = (drccodeptr)ALIGN_PTR_DOWN(cache->end - bytes);
	if (cache->top > ptr)
		return NULL;

	/* otherwise update the end of the cache */
	cache->end = ptr;
	return ptr;
}


/*-------------------------------------------------
    drccache_memory_alloc_near - allocate
    permanent memory from the near part of the
    cache
-------------------------------------------------*/

void *drccache_memory_alloc_near(drccache *cache, size_t bytes)
{
	drccodeptr ptr;

	assert(bytes > 0);

	/* pick first from the free list */
	if (bytes < MAX_PERMANENT_ALLOC)
	{
		free_link **linkptr = &cache->nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
		free_link *link = *linkptr;
		if (link != NULL)
		{
			*linkptr = link->next;
			return link;
		}
	}

	/* if no space, we just fail */
	ptr = (drccodeptr)ALIGN_PTR_UP(cache->neartop);
	if (ptr + bytes > cache->base)
		return NULL;

	/* otherwise update the top of the near part of the cache */
	cache->neartop = ptr + bytes;
	return ptr;
}


/*-------------------------------------------------
    drccache_memory_free - release permanent
    memory allocated from the cache
-------------------------------------------------*/

void drccache_memory_free(drccache *cache, void *memory, size_t bytes)
{
	free_link **linkptr;
	free_link *link = (free_link *)memory;

	assert(bytes < MAX_PERMANENT_ALLOC);
	assert(((drccodeptr)memory >= cache->near && (drccodeptr)memory < cache->base) || ((drccodeptr)memory >= cache->end && (drccodeptr)memory < cache->near + cache->size));

	/* determine which free list to add to */
	if ((drccodeptr)memory < cache->base)
		linkptr = &cache->nearfree[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];
	else
		linkptr = &cache->free[(bytes + CACHE_ALIGNMENT - 1) / CACHE_ALIGNMENT];

	/* link is into the free list for our size */
	link->next = *linkptr;
	*linkptr = link;
}


/*-------------------------------------------------
    drccache_memory_alloc_temporary - allocate
    temporary memory from the cache
-------------------------------------------------*/

void *drccache_memory_alloc_temporary(drccache *cache, size_t bytes)
{
	drccodeptr ptr = cache->top;

	/* can't allocate in the middle of codegen */
	assert(cache->codegen == NULL);

	/* if no space, we just fail */
	if (ptr + bytes >= cache->end)
		return NULL;

	/* otherwise, update the cache top */
	cache->top = (drccodeptr)ALIGN_PTR_UP(ptr + bytes);
	return ptr;
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    drccache_begin_codegen - begin code
    generation
-------------------------------------------------*/

drccodeptr *drccache_begin_codegen(drccache *cache, UINT32 reserve_bytes)
{
	drccodeptr ptr = cache->top;

	/* can't restart in the middle of codegen */
	assert(cache->codegen == NULL);
	assert(cache->ooblist == NULL);

	/* if still no space, we just fail */
	if (ptr + reserve_bytes >= cache->end)
		return NULL;

	/* otherwise, return a pointer to the cache top */
	cache->codegen = cache->top;
	cache->oobtail = &cache->ooblist;
	return &cache->top;
}


/*-------------------------------------------------
    drccache_end_codegen - complete code
    generation
-------------------------------------------------*/

drccodeptr drccache_end_codegen(drccache *cache)
{
	drccodeptr result = cache->codegen;

	/* run the OOB handlers */
	while (cache->ooblist != NULL)
	{
		/* remove us from the list */
		oob_handler *oob = cache->ooblist;
		cache->ooblist = oob->next;

		/* call the callback */
		(*oob->callback)(&cache->top, oob->param1, oob->param2, oob->param3);
		assert(cache->top - cache->codegen < CODEGEN_MAX_BYTES);

		/* release our memory */
		drccache_memory_free(cache, oob, sizeof(*oob));
	}

	/* update the cache top */
	cache->top = (drccodeptr)ALIGN_PTR_UP(cache->top);
	cache->codegen = NULL;

	return result;
}


/*-------------------------------------------------
    drccache_request_oob_codegen - request
    callback for out-of-band codegen
-------------------------------------------------*/

void drccache_request_oob_codegen(drccache *cache, drccache_oob_func callback, void *param1, void *param2, void *param3)
{
	oob_handler *oob;

	assert(cache->codegen != NULL);

	/* pull an item from the free list */
	oob = (oob_handler *)drccache_memory_alloc(cache, sizeof(*oob));
	assert(oob != NULL);

	/* fill it in */
	oob->next = NULL;
	oob->callback = callback;
	oob->param1 = param1;
	oob->param2 = param2;
	oob->param3 = param3;

	/* add to the tail */
	*cache->oobtail = oob;
	cache->oobtail = &oob->next;
}
