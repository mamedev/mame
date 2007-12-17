/***************************************************************************

    restrack.c

    Core MAME resource tracking.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "restrack.h"
#include "pool.h"
#include "timer.h"
#include "state.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef enum _memory_block_overlap memory_block_overlap;
enum _memory_block_overlap
{
	OVERLAP_NONE,
	OVERLAP_PARTIAL,
	OVERLAP_FULL
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* pool list */
static object_pool *pools[64];

/* resource tracking */
int resource_tracking_tag = 0;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void astring_destructor(void *object, size_t size);
static void bitmap_destructor(void *object, size_t size);
static memory_block_overlap pool_contains_block(object_pool *pool, void *ptr, size_t size, void **found_block, size_t *found_block_size);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    malloc_or_die_file_line - allocate memory or die
    trying
-------------------------------------------------*/

void *malloc_or_die_file_line(size_t size, const char *file, int line)
{
	void *result;

	/* fail on attempted allocations of 0 */
	if (size == 0)
		fatalerror("Attempted to malloc zero bytes (%s:%d)", file, line);

	/* allocate and return if we succeeded */
#ifdef MALLOC_DEBUG
	result = malloc_file_line(size, file, line);
#else
	result = malloc(size);
#endif
	if (result != NULL)
	{
#ifdef MAME_DEBUG
		rand_memory(result, size);
#endif
		return result;
	}

	/* otherwise, die horribly */
	fatalerror("Failed to allocate %d bytes (%s:%d)", (int)size, file, line);
}


/*-------------------------------------------------
    init_resource_tracking - initialize the
    resource tracking system
-------------------------------------------------*/

void init_resource_tracking(void)
{
	resource_tracking_tag = 0;
}


/*-------------------------------------------------
    exit_resource_tracking - tear down the
    resource tracking system
-------------------------------------------------*/

void exit_resource_tracking(void)
{
	while (resource_tracking_tag != 0)
		end_resource_tracking();
}


/*-------------------------------------------------
    memory_error - report a memory error
-------------------------------------------------*/

static void memory_error(const char *message)
{
	fatalerror("%s", message);
}


/*-------------------------------------------------
    begin_resource_tracking - start tracking
    resources
-------------------------------------------------*/

void begin_resource_tracking(void)
{
	object_pool *new_pool;

	/* sanity check */
	assert_always(resource_tracking_tag < ARRAY_LENGTH(pools), "Too many memory pools");

	/* create a new pool */
	new_pool = pool_alloc(memory_error);
	if (!new_pool)
		fatalerror("Failed to allocate new memory pool");
	pools[resource_tracking_tag] = new_pool;

	/* add resource types */
	pool_type_register(new_pool, OBJTYPE_ASTRING, "String", astring_destructor);
	pool_type_register(new_pool, OBJTYPE_BITMAP, "Bitmap", bitmap_destructor);
	pool_type_register(new_pool, OBJTYPE_TIMER, "Timer", timer_destructor);
	pool_type_register(new_pool, OBJTYPE_STATEREG, "Save State Registration", state_destructor);

	/* increment the tag counter */
	resource_tracking_tag++;
}


/*-------------------------------------------------
    end_resource_tracking - stop tracking
    resources
-------------------------------------------------*/

void end_resource_tracking(void)
{
	/* decrement the tag counter */
	resource_tracking_tag--;

	/* free the memory pool */
	pool_free(pools[resource_tracking_tag]);
	pools[resource_tracking_tag] = NULL;
}


/*-------------------------------------------------
    current_pool - identifies the current memory
    pool
-------------------------------------------------*/

static object_pool *current_pool(void)
{
	object_pool *pool;
	assert_always((resource_tracking_tag > 0) && (resource_tracking_tag <= ARRAY_LENGTH(pools)), "Invalid resource_tracking_tag");
	pool = pools[resource_tracking_tag - 1];
	assert_always(pool != NULL, "current_pool() is NULL");
	return pool;
}


/*-------------------------------------------------
    restrack_register_object - registers an
    object with the current pool
-------------------------------------------------*/

void *restrack_register_object(object_type type, void *ptr, size_t size, const char *file, int line)
{
	return pool_object_add_file_line(current_pool(), type, ptr, size, file, line);
}


/*-------------------------------------------------
    auto_malloc_file_line - allocate auto-
    freeing memory
-------------------------------------------------*/

void *auto_malloc_file_line(size_t size, const char *file, int line)
{
	return pool_malloc_file_line(current_pool(), size, file, line);
}


/*-------------------------------------------------
    auto_realloc_file_line - reallocate auto-
    freeing memory
-------------------------------------------------*/

void *auto_realloc_file_line(void *ptr, size_t size, const char *file, int line)
{
	object_pool *pool = current_pool();
	if (ptr != NULL)
	{
		int tag = resource_tracking_tag;
		for (; tag > 0; tag--)
		{
			pool = pools[tag - 1];
			if (pool_object_exists(pool, OBJTYPE_MEMORY, ptr))
				break;
		}
		assert_always(tag > 0, "Failed to find alloc in pool");
	}

	return pool_realloc_file_line(pool, ptr, size, file, line);
}


/*-------------------------------------------------
    auto_strdup_file_line - allocate auto-freeing
    string
-------------------------------------------------*/

char *auto_strdup_file_line(const char *str, const char *file, int line)
{
	return pool_strdup_file_line(current_pool(), str, file, line);
}


/*-------------------------------------------------
    auto_strdup_allow_null_file_line - allocate
    auto-freeing string if str is null
-------------------------------------------------*/

char *auto_strdup_allow_null_file_line(const char *str, const char *file, int line)
{
	return (str != NULL) ? auto_strdup_file_line(str, file, line) : NULL;
}


/*-------------------------------------------------
    auto_astring_alloc_file_line - allocate
    auto-freeing astring
-------------------------------------------------*/

astring *auto_astring_alloc_file_line(const char *file, int line)
{
	return restrack_register_object(OBJTYPE_ASTRING, astring_alloc(), 0, file, line);
}


/*-------------------------------------------------
    auto_bitmap_alloc_file_line - allocate
    auto-freeing bitmap
-------------------------------------------------*/

bitmap_t *auto_bitmap_alloc_file_line(int width, int height, bitmap_format format, const char *file, int line)
{
	return restrack_register_object(OBJTYPE_BITMAP, bitmap_alloc(width, height, format), width * height, file, line);
}


/*-------------------------------------------------
    validate_auto_malloc_memory - validate that a
    block of memory has been allocated by auto_malloc()
-------------------------------------------------*/

void validate_auto_malloc_memory(void *memory, size_t memory_size)
{
	memory_block_overlap overlap = OVERLAP_NONE;
	void *block_base = NULL;
	size_t block_size = 0;
	int i;

	/* scan all pools for an overlapping block */
	for (i = 0; overlap == OVERLAP_NONE && i < resource_tracking_tag; i++)
		overlap = pool_contains_block(pools[i], memory, memory_size, &block_base, &block_size);

	/* fatal error if not a full overlap */
	switch (overlap)
	{
		case OVERLAP_NONE:
			fatalerror("Memory block [0x%p-0x%p] not found", memory, (UINT8 *)memory + memory_size - 1);
			break;

		case OVERLAP_PARTIAL:
			fatalerror("Memory block [0x%p-0x%p] partially overlaps with allocated block [0x%p-0x%p]", memory, (UINT8 *)memory + memory_size - 1, block_base, (UINT8 *)block_base + block_size - 1);
			break;

		case OVERLAP_FULL:
			/* expected outcome */
			break;
	}
}


/*-------------------------------------------------
    astring_destructor - destructor for astring
    objects
-------------------------------------------------*/

static void astring_destructor(void *object, size_t size)
{
	astring_free(object);
}


/*-------------------------------------------------
    bitmap_destructor - destructor for bitmap
    objects
-------------------------------------------------*/

static void bitmap_destructor(void *object, size_t size)
{
	bitmap_free(object);
}


/*-------------------------------------------------
    pool_contains_block - determines if a pool
    contains a memory block
-------------------------------------------------*/

static memory_block_overlap pool_contains_block(object_pool *pool, void *ptr, size_t size, void **found_block, size_t *found_block_size)
{
	memory_block_overlap overlap = OVERLAP_NONE;
	object_pool_iterator *iter;
	UINT8 *ptrstart = ptr;
	UINT8 *ptrend = ptrstart + size - 1;
	void *blockptr = NULL;
	size_t blocksize = 0;

	/* iterate over memory objects in the pool */
	for (iter = pool_iterate_begin(pool, OBJTYPE_MEMORY); iter != NULL && pool_iterate_next(iter, &blockptr, &blocksize,  NULL); )
	{
		int startwithin = (ptrstart >= (UINT8 *)blockptr && ptrstart < (UINT8 *)blockptr + blocksize);
		int endwithin = (ptrend >= (UINT8 *)blockptr && ptrend < (UINT8 *)blockptr + blocksize);

		/* if the start of the incoming pointer lies within the block... */
		if (startwithin || endwithin)
		{
			overlap = (startwithin && endwithin) ? OVERLAP_FULL : OVERLAP_PARTIAL;
			break;
		}
	}
	pool_iterate_end(iter);

	/* store the results */
	if (overlap != OVERLAP_NONE)
	{
		if (found_block != NULL)
			*found_block = blockptr;
		if (found_block_size != NULL)
			*found_block_size = blocksize;
	}
	return overlap;
}
