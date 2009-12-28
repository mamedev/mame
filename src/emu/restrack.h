/***************************************************************************

    restrack.h

    Core MAME resource tracking.

    Copyright Nicola Salmoria and the MAME Team.
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
#define alloc_or_die(t)					((t *)malloc_or_die_file_line(sizeof(t), __FILE__, __LINE__))
#define alloc_clear_or_die(t)			((t *)memset(malloc_or_die_file_line(sizeof(t), __FILE__, __LINE__), 0, sizeof(t)))
#define alloc_array_or_die(t, c)		((t *)malloc_or_die_file_line((c) * sizeof(t), __FILE__, __LINE__))
#define alloc_array_clear_or_die(t, c)	((t *)memset(malloc_or_die_file_line((c) * sizeof(t), __FILE__, __LINE__), 0, (c) * sizeof(t)))
void *malloc_or_die_file_line(size_t size, const char *file, int line) ATTR_MALLOC;


/* allocate memory that will be freed at the next end_resource_tracking */
#define auto_alloc(m, t)				((t *)auto_malloc_file_line(m, sizeof(t), __FILE__, __LINE__))
#define auto_alloc_clear(m, t)			((t *)memset(auto_malloc_file_line(m, sizeof(t), __FILE__, __LINE__), 0, sizeof(t)))
#define auto_alloc_array(m, t, c)		((t *)auto_malloc_file_line(m, (c) * sizeof(t), __FILE__, __LINE__))
#define auto_alloc_array_clear(m, t, c) ((t *)memset(auto_malloc_file_line(m, (c) * sizeof(t), __FILE__, __LINE__), 0, (c) * sizeof(t)))
void *auto_malloc_file_line(running_machine *machine, size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory that will be freed at the next end_resource_tracking */
#define auto_extend_array(m, p, t, c)	((t *)auto_realloc_file_line(m, p, (c) * sizeof(t), __FILE__, __LINE__))
void *auto_realloc_file_line(running_machine *machine, void *ptr, size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory and duplicate a string that will be freed at the next end_resource_tracking */
#define auto_strdup(m, s)				auto_strdup_file_line(m, s, __FILE__, __LINE__)
char *auto_strdup_file_line(running_machine *machine, const char *str, const char *file, int line) ATTR_MALLOC;

/* auto_strdup() variant that tolerates NULL */
#define auto_strdup_allow_null(m, s)	auto_strdup_allow_null_file_line(m, s, __FILE__, __LINE__)
char *auto_strdup_allow_null_file_line(running_machine *machine, const char *str, const char *file, int line) ATTR_MALLOC;

/* allocate a bitmap that will be freed at the next end_resource_tracking */
#define auto_astring_alloc(m)			auto_astring_alloc_file_line(m, __FILE__, __LINE__)
astring *auto_astring_alloc_file_line(running_machine *machine, const char *file, int line);

/* allocate a bitmap that will be freed at the next end_resource_tracking */
#define auto_bitmap_alloc(m, w, h, f)	auto_bitmap_alloc_file_line(m, w, h, f, __FILE__, __LINE__)
bitmap_t *auto_bitmap_alloc_file_line(running_machine *machine, int width, int height, bitmap_format format, const char *file, int line);


#endif /* __RESTRACK_H__ */
