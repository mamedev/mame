/***************************************************************************

    restrack.h

    Core MAME resource tracking.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RESTRACK_H__
#define __RESTRACK_H__

#include "mamecore.h"
#include "pool.h"
#include "astring.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define OBJTYPE_ASTRING					OBJECT_TYPE('a','s','t','r')
#define OBJTYPE_BITMAP					OBJECT_TYPE('b','i','t','m')
#define OBJTYPE_TIMER					OBJECT_TYPE('t','i','m','r')
#define OBJTYPE_STATEREG				OBJECT_TYPE('s','t','a','t')



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialize the resource tracking system */
void init_resource_tracking(void);

/* tear down the resource tracking system */
void exit_resource_tracking(void);

/* begin tracking resources */
void begin_resource_tracking(void);

/* stop tracking resources and free everything since the last begin */
void end_resource_tracking(void);

/* register an object with the current pool */
void *restrack_register_object(object_type type, void *ptr, size_t size, const char *file, int line);

/* validate that a block of memory has been allocated by auto_malloc() */
void validate_auto_malloc_memory(void *memory, size_t memory_size);

/* return the current resource tag */
INLINE int get_resource_tag(void)
{
	extern int resource_tracking_tag;
	return resource_tracking_tag;
}


/* allocate memory and fatalerror if there's a problem */
#define malloc_or_die(s)			malloc_or_die_file_line(s, __FILE__, __LINE__)
void *malloc_or_die_file_line(size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory that will be freed at the next end_resource_tracking */
#define auto_malloc(s)				auto_malloc_file_line(s, __FILE__, __LINE__)
void *auto_malloc_file_line(size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory that will be freed at the next end_resource_tracking */
#define auto_realloc(p, s)			auto_realloc_file_line(p, s, __FILE__, __LINE__)
void *auto_realloc_file_line(void *ptr, size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory and duplicate a string that will be freed at the next end_resource_tracking */
#define auto_strdup(s)				auto_strdup_file_line(s, __FILE__, __LINE__)
char *auto_strdup_file_line(const char *str, const char *file, int line) ATTR_MALLOC;

/* auto_strdup() variant that tolerates NULL */
#define auto_strdup_allow_null(s)	auto_strdup_allow_null_file_line(s, __FILE__, __LINE__)
char *auto_strdup_allow_null_file_line(const char *str, const char *file, int line) ATTR_MALLOC;

/* allocate a bitmap that will be freed at the next end_resource_tracking */
#define auto_astring_alloc()		auto_astring_alloc_file_line(__FILE__, __LINE__)
astring *auto_astring_alloc_file_line(const char *file, int line);

/* allocate a bitmap that will be freed at the next end_resource_tracking */
#define auto_bitmap_alloc(w,h,f)	auto_bitmap_alloc_file_line(w, h, f, __FILE__, __LINE__)
bitmap_t *auto_bitmap_alloc_file_line(int width, int height, bitmap_format format, const char *file, int line);


#endif /* __RESTRACK_H__ */
