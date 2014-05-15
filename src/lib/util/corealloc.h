// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corealloc.h

    Memory allocation helpers for the helper library.

***************************************************************************/

#pragma once

#ifndef __COREALLOC_H__
#define __COREALLOC_H__

#include <stdlib.h>
#include <new>
#include "osdcore.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

// define this to initialize allocated memory to a fixed non-0 value
//#define INITIALIZE_ALLOCATED_MEMORY

// define this to zap memory to a fixed non-0 value before freeing
//#define OVERWRITE_FREED_MEMORY



//**************************************************************************
//  MACROS
//**************************************************************************

// global allocation helpers -- use these instead of new and delete
#define global_alloc(_type)                         new(__FILE__, __LINE__) _type
#define global_alloc_clear(_type)                   new(__FILE__, __LINE__, zeromem) _type
#define global_alloc_array(_type, _num)             new(__FILE__, __LINE__) _type[_num]
#define global_alloc_array_clear(_type, _num)       new(__FILE__, __LINE__, zeromem) _type[_num]
#define global_free(_ptr)                           do { delete _ptr; } while (0)
#define global_free_array(_ptr)                     do { delete[] _ptr; } while (0)



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// allocate memory with file and line number information
void *malloc_file_line(size_t size, const char *file, int line, bool array, bool throw_on_fail, bool clear);

// free memory with file and line number information
void free_file_line(void *memory, const char *file, int line, bool array);
inline void free_file_line(const void *memory, const char *file, int line, bool array) { free_file_line(const_cast<void *>(memory), file, line, array); }

// called from the exit path of any code that wants to check for unfreed memory
void track_memory(bool track);
UINT64 next_memory_id();
void dump_unfreed_mem(UINT64 start = 0);



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// zeromem_t is a dummy class used to tell new to zero memory after allocation
class zeromem_t { };

#ifndef NO_MEM_TRACKING

// standard new/delete operators (try to avoid using)
ATTR_FORCE_INLINE inline void *operator new(std::size_t size) throw (std::bad_alloc) { return malloc_file_line(size, NULL, 0, false, true, false); }
ATTR_FORCE_INLINE inline void *operator new[](std::size_t size) throw (std::bad_alloc) { return malloc_file_line(size, NULL, 0, true, true, false); }
ATTR_FORCE_INLINE inline void operator delete(void *ptr) throw() { if (ptr != NULL) free_file_line(ptr, NULL, 0, false); }
ATTR_FORCE_INLINE inline void operator delete[](void *ptr) throw() { if (ptr != NULL) free_file_line(ptr, NULL, 0, true); }

#endif

// file/line new/delete operators
ATTR_FORCE_INLINE inline void *operator new(std::size_t size, const char *file, int line) throw (std::bad_alloc) { return malloc_file_line(size, file, line, false, true, false); }
ATTR_FORCE_INLINE inline void *operator new[](std::size_t size, const char *file, int line) throw (std::bad_alloc) { return malloc_file_line(size, file, line, true, true, false); }
ATTR_FORCE_INLINE inline void operator delete(void *ptr, const char *file, int line) { if (ptr != NULL) free_file_line(ptr, file, line, false); }
ATTR_FORCE_INLINE inline void operator delete[](void *ptr, const char *file, int line) { if (ptr != NULL) free_file_line(ptr, file, line, true); }

// file/line new/delete operators with zeroing
ATTR_FORCE_INLINE inline void *operator new(std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc) { return malloc_file_line(size, file, line, false, true, true); }
ATTR_FORCE_INLINE inline void *operator new[](std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc) { return malloc_file_line(size, file, line, true, true, true); }
ATTR_FORCE_INLINE inline void operator delete(void *ptr, const char *file, int line, const zeromem_t &) { if (ptr != NULL) free_file_line(ptr, file, line, false); }
ATTR_FORCE_INLINE inline void operator delete[](void *ptr, const char *file, int line, const zeromem_t &) { if (ptr != NULL) free_file_line(ptr, file, line, true); }



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// dummy objects to pass to the specialized new variants
extern const zeromem_t zeromem;



//**************************************************************************
//  ADDDITIONAL MACROS
//**************************************************************************

#ifndef NO_MEM_TRACKING
// re-route classic malloc-style allocations
#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(x)       malloc_file_line(x, __FILE__, __LINE__, true, false, false)
#define calloc(x,y)     __error_use_auto_alloc_clear_or_global_alloc_clear_instead__
#define realloc(x,y)    __error_realloc_is_dangerous__
#define free(x)         free_file_line(x, __FILE__, __LINE__, true)
#endif

#endif  /* __COREALLOC_H__ */
