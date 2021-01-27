// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    pool.h

    Abstract object pool management

***************************************************************************/

#ifndef MAME_UTIL_POOL_H
#define MAME_UTIL_POOL_H

#pragma once

#include <cstddef>
#include <cstdint>


/***************************************************************************
    MACROS
***************************************************************************/

/* helper macros for memory pools that pass file/line number */
#define pool_malloc_lib(pool, size)         pool_malloc_file_line((pool), (size), __FILE__, __LINE__)
#define pool_realloc_lib(pool, ptr, size)   pool_realloc_file_line((pool), (ptr), (size), __FILE__, __LINE__)
#define pool_strdup_lib(pool, size)         pool_strdup_file_line((pool), (size), __FILE__, __LINE__)

/* macro to define a 4-character type for a pool */
#define OBJECT_TYPE(a,b,c,d)            (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/* built-in pool types */
#define OBJTYPE_WILDCARD                (0)
#define OBJTYPE_MEMORY                  OBJECT_TYPE('m','e','m','o')



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* pool types are UINT32s */
typedef uint32_t object_type;

/* opaque type representing a pool of objects */
struct object_pool;

/* opaque type representing an iterator over pool objects */
struct object_pool_iterator;



/***************************************************************************
    PROTOTYPES
***************************************************************************/


/* ----- object pool management ----- */

/* allocate a new object pool */
object_pool *pool_alloc_lib(void (*fail)(const char *message));

/* register a new object type; returns true if the type already existed and was overridden */
void pool_type_register(object_pool *pool, object_type type, const char *friendly, void (*destructor)(void *, size_t));

/* free all allocated objects in a pool */
void pool_clear(object_pool *pool);

/* free an object pool, including all allocated objects */
void pool_free_lib(object_pool *pool);



/* ----- object management ----- */

/* add an object to the pool, along with its filename/line number */
void *pool_object_add_file_line(object_pool *pool, object_type type, void *object, size_t size, const char *file, int line);

/* remove an object from the pool (optionally calling destructor) */
void *pool_object_remove(object_pool *pool, void *object, int destruct);

/* does an object exist in the pool? */
bool pool_object_exists(object_pool *pool, object_type type, void *object);



/* ----- object iterators ----- */

/* begin iterating over objects in an object pool */
object_pool_iterator *pool_iterate_begin(object_pool *pool, object_type type);

/* get the next object in the object pool */
bool pool_iterate_next(object_pool_iterator *iter, void **objectptr, size_t *sizeptr, object_type *typeptr);

/* finish iterating over objects in an object pool */
void pool_iterate_end(object_pool_iterator *iter);



/* ----- memory helpers ----- */

/* malloc memory and register it with the given pool */
void *pool_malloc_file_line(object_pool *pool, size_t size, const char *file, int line);

/* realloc memory and register it with the given pool */
void *pool_realloc_file_line(object_pool *pool, void *ptr, size_t size, const char *file, int line);

/* strdup memory and register it with the given pool */
char *pool_strdup_file_line(object_pool *pool, const char *str, const char *file, int line);



/* ----- miscellaneous ----- */

/* internal unit tests */
bool test_memory_pools(void);


#endif // MAME_UTIL_POOL_H
