/***************************************************************************

    drccache.h

    Universal dynamic recompiler cache management.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRCCACHE_H__
#define __DRCCACHE_H__



/***************************************************************************
    MACROS
***************************************************************************/

/* ensure that a given pointer is within the cache boundaries */
#define assert_in_cache(c,p)		assert(drccache_contains_pointer(c, p))
#define assert_in_near_cache(c,p)	assert(drccache_contains_near_pointer(c, p))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* generic code pointer */
typedef UINT8 *drccodeptr;

/* opaque cache state */
typedef struct _drccache drccache;

/* out of band codegen callback */
typedef void (*drccache_oob_func)(drccodeptr *codeptr, void *param1, void *param2, void *param3);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization/teardown ----- */

/* allocate the cache itself */
drccache *drccache_alloc(size_t bytes);

/* free the cache */
void drccache_free(drccache *cache);



/* ----- cache information ----- */

/* return true if a pointer is within the cache */
int drccache_contains_pointer(drccache *cache, const void *ptr);

/* return true if a pointer is within the near area of the cache */
int drccache_contains_near_pointer(drccache *cache, const void *ptr);

/* return a pointer to the near part of the cache */
drccodeptr drccache_near(drccache *cache);

/* return a pointer to the base of the cache, which is where code generation starts */
drccodeptr drccache_base(drccache *cache);

/* return the current top of the cache, which is where the next code will be generated */
drccodeptr drccache_top(drccache *cache);



/* ----- memory management ----- */

/* flush the cache contents */
void drccache_flush(drccache *cache);

/* allocate permanent memory from the cache */
void *drccache_memory_alloc(drccache *cache, size_t bytes);

/* allocate permanent memory from the near portion of the cache */
void *drccache_memory_alloc_near(drccache *cache, size_t bytes);

/* release permanent memory allocated from the cache */
void drccache_memory_free(drccache *cache, void *memory, size_t bytes);

/* allocate temporary memory from the cache (released on a reset) */
void *drccache_memory_alloc_temporary(drccache *cache, size_t bytes);



/* ----- code generation ----- */

/* begin code generation */
drccodeptr *drccache_begin_codegen(drccache *cache, UINT32 reserve_bytes);

/* complete code generation */
drccodeptr drccache_end_codegen(drccache *cache);

/* request callback for out-of-band codegen */
void drccache_request_oob_codegen(drccache *cache, drccache_oob_func callback, void *param1, void *param2, void *param3);


#endif /* __DRCCACHE_H__ */
