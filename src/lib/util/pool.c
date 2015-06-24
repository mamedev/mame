// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    pool.c

    Resource pool code

***************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include "pool.h"
#include "coreutil.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define OBJECT_ENTRY_BLOCK  256
#define POOL_HASH_SIZE      3797



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* an object type entry */
struct objtype_entry
{
	objtype_entry *     next;
	UINT32              type;
	const char *        friendly;
	void                (*destructor)(void *, size_t);
};


/* an entry in a pool */
struct object_entry
{
	object_entry *      next;
	object_entry *      globalnext;
	object_entry *      globalprev;
	objtype_entry *     type;
	void *              object;
	size_t              size;
	const char *        file;
	int                 line;
};


/* a block of entry items */
struct object_entry_block
{
	object_entry_block *next;
	object_entry        entry[OBJECT_ENTRY_BLOCK];
};


/* the object pool itself */
struct object_pool
{
	object_entry *      hashtable[POOL_HASH_SIZE];
	object_entry *      globallist;
	object_entry *      freelist;
	object_entry_block *blocklist;
	objtype_entry *     typelist;
	void                (*fail)(const char *message);
};


/* an iterator over objects in a pool */
struct object_pool_iterator
{
	object_pool *       pool;
	object_type         type;
	object_entry *      last;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void memory_destruct(void *object, size_t size);
static void report_failure(object_pool *pool, const char *format, ...) ATTR_PRINTF(2,3);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    hash_object - compute the hash for a given
    object
-------------------------------------------------*/

INLINE int hash_object(void *object)
{
	return ((size_t)object >> 4) % POOL_HASH_SIZE;
}


/*-------------------------------------------------
    get_object_type - return the type entry for
    a given object type
-------------------------------------------------*/

INLINE objtype_entry *get_object_type(object_pool *pool, object_type type)
{
	objtype_entry *entry;

	for (entry = pool->typelist; entry != NULL; entry = entry->next)
		if (entry->type == type)
			return entry;

	return NULL;
}



/***************************************************************************
    RESOURCE POOL MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    pool_alloc - allocates a new memory pool
-------------------------------------------------*/

object_pool *pool_alloc_lib(void (*fail)(const char *message))
{
	object_pool *pool;

	/* allocate memory for the pool itself */
	pool = (object_pool *)malloc(sizeof(*pool));
	if (pool == NULL)
		return NULL;
	memset(pool, 0, sizeof(*pool));

	/* set the failure handler */
	pool->fail = fail;

	/* register the built-in types */
	pool_type_register(pool, OBJTYPE_MEMORY, "Memory", memory_destruct);

	return pool;
}


/*-------------------------------------------------
    pool_type_register - register a new pool
    type
-------------------------------------------------*/

void pool_type_register(object_pool *pool, object_type type, const char *friendly, void (*destructor)(void *, size_t))
{
	objtype_entry *newtype = get_object_type(pool, type);

	/* if the type doesn't already exist... */
	if (newtype == NULL)
	{
		/* allocate a new entry */
		newtype = (objtype_entry *)malloc(sizeof(*newtype));
		if (newtype == NULL)
		{
			report_failure(pool, "Error adding new type %s\n", friendly);
			return;
		}
		memset(newtype, 0, sizeof(*newtype));

		/* link us in */
		newtype->next = pool->typelist;
		pool->typelist = newtype;
	}

	/* fill in the type information */
	newtype->type = type;
	newtype->friendly = friendly;
	newtype->destructor = destructor;
}


/*-------------------------------------------------
    pool_clear - frees all contained memory blocks
    in a pool
-------------------------------------------------*/

void pool_clear(object_pool *pool)
{
	object_entry *entry, *next;

	/* iterate over all entries in the global list and free them */
	for (entry = pool->globallist; entry != NULL; entry = next)
	{
		/* remember the next entry */
		next = entry->globalnext;

		/* call the destructor */
		(*entry->type->destructor)(entry->object, entry->size);

		/* add ourself to the free list */
		entry->next = pool->freelist;
		entry->globalnext = entry->globalprev = NULL;
		pool->freelist = entry;
	}

	/* zap the hashtable */
	memset(pool->hashtable, 0, sizeof(pool->hashtable));
}


/*-------------------------------------------------
    pool_free_lib - frees a memory pool and all
    contained memory blocks
-------------------------------------------------*/

void pool_free_lib(object_pool *pool)
{
	object_entry_block *block, *nextblock;
	objtype_entry *type, *nexttype;

	/* clear the pool */
	pool_clear(pool);

	/* free all entry blocks */
	for (block = pool->blocklist; block != NULL; block = nextblock)
	{
		nextblock = block->next;
		free(block);
	}

	/* free all types */
	for (type = pool->typelist; type != NULL; type = nexttype)
	{
		nexttype = type->next;
		free(type);
	}

	/* free the pool itself */
	free(pool);
}



/***************************************************************************
    OBJECT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    pool_object_add_file_line - add an object to
    the resource pool
-------------------------------------------------*/

void *pool_object_add_file_line(object_pool *pool, object_type _type, void *object, size_t size, const char *file, int line)
{
	objtype_entry *type = get_object_type(pool, _type);
	int hashnum = hash_object(object);
	object_entry *entry;

	/* if we have an invalid type, fail */
	if (type == NULL)
	{
		report_failure(pool, "pool_object_add (via %s:%d): Attempted to add object of unknown type with size %d", file, line, (int)size);
		return object;
	}

	/* if we get a NULL object, fail */
	if (object == NULL)
	{
		report_failure(pool, "pool_object_add (via %s:%d): Attempted to add a NULL object of size %d", file, line, (int)size);
		return object;
	}

	/* allocate a new entry */
	if (pool->freelist == NULL)
	{
		object_entry_block *block;
		int entrynum;

		/* if we need a new block, allocate that now */
		block = (object_entry_block *)malloc(sizeof(*block));
		if (block == NULL)
			return NULL;
		memset(block, 0, sizeof(*block));

		/* hook us into the blocklist */
		block->next = pool->blocklist;
		pool->blocklist = block;

		/* add all entries to the free list */
		for (entrynum = 0; entrynum < ARRAY_LENGTH(block->entry); entrynum++)
		{
			block->entry[entrynum].next = pool->freelist;
			pool->freelist = &block->entry[entrynum];
		}
	}

	/* remove the head of the freelist */
	entry = pool->freelist;
	pool->freelist = entry->next;

	/* fill in the entry */
	entry->type = type;
	entry->object = object;
	entry->size = size;
	entry->file = file;
	entry->line = line;

	/* hook us into the global list */
	if (pool->globallist != NULL)
		pool->globallist->globalprev = entry;
	entry->globalprev = NULL;
	entry->globalnext = pool->globallist;
	pool->globallist = entry;

	/* hook up to the appropriate hash table */
	entry->next = pool->hashtable[hashnum];
	pool->hashtable[hashnum] = entry;
	return object;
}


/*-------------------------------------------------
    pool_object_remove - remove an entry from a
    pool, optionally calling its destructor
-------------------------------------------------*/

void *pool_object_remove(object_pool *pool, void *object, int destruct)
{
	int hashnum = hash_object(object);
	object_entry **entryptr;

	/* find the object in question and remove it */
	for (entryptr = &pool->hashtable[hashnum]; *entryptr != NULL; entryptr = &(*entryptr)->next)
		if ((*entryptr)->object == object)
		{
			object_entry *entry = *entryptr;

			/* call the destructor */
			if (destruct)
				(*entry->type->destructor)(entry->object, entry->size);

			/* remove us from the global list */
			if (entry->globalprev != NULL)
				entry->globalprev->globalnext = entry->globalnext;
			if (entry->globalnext != NULL)
				entry->globalnext->globalprev = entry->globalprev;
			if (pool->globallist == entry)
				pool->globallist = entry->globalnext;

			/* remove us from the list */
			*entryptr = entry->next;

			/* add us to the free list */
			entry->next = pool->freelist;
			pool->freelist = entry;
			break;
		}

	return NULL;
}


/*-------------------------------------------------
    pool_object_exists - return TRUE if an
    object exists in the pool
-------------------------------------------------*/

int pool_object_exists(object_pool *pool, object_type type, void *object)
{
	int hashnum = hash_object(object);
	object_entry *entry;

	/* find the object in question */
	for (entry = pool->hashtable[hashnum]; entry != NULL; entry = entry->next)
		if (entry->object == object && (type == OBJTYPE_WILDCARD || entry->type->type == type))
			return TRUE;

	return FALSE;
}



/***************************************************************************
    OBJECT ITERATORS
***************************************************************************/

/*-------------------------------------------------
    pool_iterate_begin - begin iterating over
    objects in an object pool
-------------------------------------------------*/

object_pool_iterator *pool_iterate_begin(object_pool *pool, object_type type)
{
	object_pool_iterator *iter;

	/* allocate the iterator */
	iter = (object_pool_iterator *)malloc(sizeof(*iter));
	if (iter == NULL)
		return NULL;
	memset(iter, 0, sizeof(*iter));

	/* fill it in */
	iter->pool = pool;
	iter->type = type;
	iter->last = NULL;
	return iter;
}


/*-------------------------------------------------
    pool_iterate_next - get the next object in the
    object pool
-------------------------------------------------*/

int pool_iterate_next(object_pool_iterator *iter, void **objectptr, size_t *sizeptr, object_type *typeptr)
{
	/* if no previous entry, find the first */
	if (iter->last == NULL)
		iter->last = iter->pool->globallist;
	else
		iter->last = iter->last->globalnext;

	/* stop when we get one */
	if (iter->last != NULL)
	{
		if (objectptr != NULL)
			*objectptr = iter->last;
		if (sizeptr != NULL)
			*sizeptr = iter->last->size;
		if (typeptr != NULL)
			*typeptr = iter->last->type->type;
		return TRUE;
	}

	/* nothing left */
	return FALSE;
}


/*-------------------------------------------------
    pool_iterate_end - finish iterating over
    objects in an object pool
-------------------------------------------------*/

void pool_iterate_end(object_pool_iterator *iter)
{
	free(iter);
}



/***************************************************************************
    MEMORY HELPERS
***************************************************************************/

/*-------------------------------------------------
    pool_malloc_file_line - allocates memory in a
    memory pool
-------------------------------------------------*/

void *pool_malloc_file_line(object_pool *pool, size_t size, const char *file, int line)
{
	void *ptr = malloc(size);
	return pool_object_add_file_line(pool, OBJTYPE_MEMORY, ptr, size, file, line);
}


/*-------------------------------------------------
    pool_realloc_file_line - reallocs memory in a
    memory pool
-------------------------------------------------*/

void *pool_realloc_file_line(object_pool *pool, void *ptr, size_t size, const char *file, int line)
{
	if (ptr != NULL)
		pool_object_remove(pool, ptr, FALSE);
	ptr = realloc(ptr, size);
	if (size != 0)
		pool_object_add_file_line(pool, OBJTYPE_MEMORY, ptr, size, file, line);
	return ptr;
}


/*-------------------------------------------------
    pool_strdup_file_line - strdup in a memory
    pool
-------------------------------------------------*/

char *pool_strdup_file_line(object_pool *pool, const char *str, const char *file, int line)
{
	char *ptr = (char *)pool_malloc_file_line(pool, strlen(str) + 1, file, line);
	if (ptr != NULL)
		strcpy(ptr, str);
	return ptr;
}



/***************************************************************************
    GENERIC DESTRUCTORS
***************************************************************************/

/*-------------------------------------------------
    memory_destruct - generic destructor
    that just does a free() on the object
-------------------------------------------------*/

static void memory_destruct(void *object, size_t size)
{
	free(object);
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    report_failure - report a failure to the
    failure callback
-------------------------------------------------*/

static void report_failure(object_pool *pool, const char *format, ...)
{
	/* only do the work if we have a callback */
	if (pool->fail != NULL)
	{
		char message[1024];
		va_list argptr;

		/* generate the final message text */
		va_start(argptr, format);
		vsprintf(message, format, argptr);
		va_end(argptr);

		/* call the callback with the message */
		(*pool->fail)(message);
	}
}



/***************************************************************************
    TESTING FUNCTIONS
***************************************************************************/

static int has_memory_error;


/*-------------------------------------------------
    memory_error - report a memory error
-------------------------------------------------*/

static void memory_error(const char *message)
{
	printf("memory test failure: %s\n", message);
	has_memory_error = TRUE;
}


/*-------------------------------------------------
    test_memory_pools - unit tests for memory
    pool behavior
-------------------------------------------------*/

/**
 * @fn  int test_memory_pools(void)
 *
 * @brief   Tests memory pools.
 *
 * @return  An int.
 */

int test_memory_pools(void)
{
	object_pool *pool;
	void *ptrs[16];
	int i;

	has_memory_error = FALSE;
	pool = pool_alloc_lib(memory_error);
	memset(ptrs, 0, sizeof(ptrs));

	ptrs[0] = pool_malloc_lib(pool, 50);
	ptrs[1] = pool_malloc_lib(pool, 100);

	ptrs[0] = pool_realloc_lib(pool, ptrs[0], 150);
	ptrs[1] = pool_realloc_lib(pool, ptrs[1], 200);

	ptrs[2] = pool_malloc_lib(pool, 250);
	ptrs[3] = pool_malloc_lib(pool, 300);

	ptrs[0] = pool_realloc_lib(pool, ptrs[0], 350);
	ptrs[1] = pool_realloc_lib(pool, ptrs[1], 400);

	ptrs[2] = pool_realloc_lib(pool, ptrs[2], 450);
	ptrs[3] = pool_realloc_lib(pool, ptrs[3], 500);

	ptrs[0] = pool_realloc_lib(pool, ptrs[0], 0);
	ptrs[1] = pool_realloc_lib(pool, ptrs[1], 0);

	ptrs[2] = pool_realloc_lib(pool, ptrs[2], 550);
	ptrs[3] = pool_realloc_lib(pool, ptrs[3], 600);

	/* some heavier stress tests */
	for (i = 0; i < 512; i++)
	{
		ptrs[i % ARRAY_LENGTH(ptrs)] = pool_realloc_lib(pool,
			ptrs[i % ARRAY_LENGTH(ptrs)], rand() % 1000);
	}

	pool_free_lib(pool);
	return has_memory_error;
}
