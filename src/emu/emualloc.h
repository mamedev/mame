/***************************************************************************

    emualloc.h

    Memory allocation helpers for the core emulator.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMUALLOC_H__
#define __EMUALLOC_H__

#include <new>
#include "osdcore.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

// set to 1 to track memory allocated by emualloc.h itself as well
#define TRACK_SELF_MEMORY       (0)



//**************************************************************************
//  MACROS
//**************************************************************************

// self-allocation helpers
#if TRACK_SELF_MEMORY
#define EMUALLOC_SELF_NEW new(__FILE__, __LINE__)
#else
#define EMUALLOC_SELF_NEW new
#endif

// pool allocation helpers
#define pool_alloc(_pool, _type)                    (_pool).add_object(new(__FILE__, __LINE__) _type)
#define pool_alloc_clear(_pool, _type)              (_pool).add_object(new(__FILE__, __LINE__, zeromem) _type)
#define pool_alloc_array(_pool, _type, _num)        (_pool).add_array(new(__FILE__, __LINE__) _type[_num], (_num))
#define pool_alloc_array_clear(_pool, _type, _num)  (_pool).add_array(new(__FILE__, __LINE__, zeromem) _type[_num], (_num))
#define pool_free(_pool, v)                         (_pool).remove(v)

// global allocation helpers
#define global_alloc(_type)                         pool_alloc(global_resource_pool(), _type)
#define global_alloc_clear(_type)                   pool_alloc_clear(global_resource_pool(), _type)
#define global_alloc_array(_type, _num)             pool_alloc_array(global_resource_pool(), _type, _num)
#define global_alloc_array_clear(_type, _num)       pool_alloc_array_clear(global_resource_pool(), _type, _num)
#define global_free(v)                              pool_free(global_resource_pool(), v)



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// allocate memory with file and line number information
void *malloc_file_line(size_t size, const char *file, int line);
void *malloc_array_file_line(size_t size, const char *file, int line);

// free memory with file and line number information
void free_file_line(void *memory, const char *file, int line);

// called from the exit path of any code that wants to check for unfreed memory
void track_memory(bool track);
void dump_unfreed_mem();



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// zeromem_t is a dummy class used to tell new to zero memory after allocation
class zeromem_t { };

#ifndef NO_MEM_TRACKING

// standard new/delete operators (try to avoid using)
ATTR_FORCE_INLINE inline void *operator new(std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

ATTR_FORCE_INLINE inline void *operator new[](std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_array_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

ATTR_FORCE_INLINE inline void operator delete(void *ptr) throw()
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}

ATTR_FORCE_INLINE inline void operator delete[](void *ptr) throw()
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}

#endif

// file/line new/delete operators
ATTR_FORCE_INLINE inline void *operator new(std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

ATTR_FORCE_INLINE inline void *operator new[](std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_array_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

ATTR_FORCE_INLINE inline void operator delete(void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}

ATTR_FORCE_INLINE inline void operator delete[](void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}


// file/line new/delete operators with zeroing
ATTR_FORCE_INLINE inline void *operator new(std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}

ATTR_FORCE_INLINE inline void *operator new[](std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_array_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}

ATTR_FORCE_INLINE inline void operator delete(void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}

ATTR_FORCE_INLINE inline void operator delete[](void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// resource_pool_item is a base class for items that are tracked by a resource pool
class resource_pool_item
{
private:
	resource_pool_item(const resource_pool_item &);
	resource_pool_item &operator=(const resource_pool_item &);

public:
	resource_pool_item(void *ptr, size_t size)
		: m_next(NULL),
			m_ordered_next(NULL),
			m_ordered_prev(NULL),
			m_ptr(ptr),
			m_size(size),
			m_id(~(UINT64)0) { }
	virtual ~resource_pool_item() { }

	resource_pool_item *    m_next;
	resource_pool_item *    m_ordered_next;
	resource_pool_item *    m_ordered_prev;
	void *                  m_ptr;
	size_t                  m_size;
	UINT64                  m_id;
};


// a resource_pool_object is a simple object wrapper for the templatized type
template<class _ObjectClass>
class resource_pool_object : public resource_pool_item
{
private:
	resource_pool_object<_ObjectClass>(const resource_pool_object<_ObjectClass> &);
	resource_pool_object<_ObjectClass> &operator=(const resource_pool_object<_ObjectClass> &);

public:
	resource_pool_object(_ObjectClass *object)
		: resource_pool_item(reinterpret_cast<void *>(object), sizeof(_ObjectClass)),
			m_object(object) { }
	virtual ~resource_pool_object() { delete m_object; }

private:
	_ObjectClass *          m_object;
};


// a resource_pool_array is a simple object wrapper for an allocated array of
// the templatized type
template<class _ObjectClass> class resource_pool_array : public resource_pool_item
{
private:
	resource_pool_array<_ObjectClass>(const resource_pool_array<_ObjectClass> &);
	resource_pool_array<_ObjectClass> &operator=(const resource_pool_array<_ObjectClass> &);

public:
	resource_pool_array(_ObjectClass *array, int count)
		: resource_pool_item(reinterpret_cast<void *>(array), sizeof(_ObjectClass) * count),
			m_array(array),
			m_count(count) { }
	virtual ~resource_pool_array() { delete[] m_array; }

private:
	_ObjectClass *          m_array;
	int                     m_count;
};


// a resource pool tracks items and frees them upon reset or destruction
class resource_pool
{
private:
	resource_pool(const resource_pool &);
	resource_pool &operator=(const resource_pool &);

public:
	resource_pool(int hash_size = 193);
	virtual ~resource_pool();

	void add(resource_pool_item &item);
	void remove(resource_pool_item &item) { remove(item.m_ptr); }
	void remove(void *ptr);
	void remove(const void *ptr) { remove(const_cast<void *>(ptr)); }
	resource_pool_item *find(void *ptr);
	bool contains(void *ptrstart, void *ptrend);
	void clear();

	template<class _ObjectClass> _ObjectClass *add_object(_ObjectClass* object) { add(*EMUALLOC_SELF_NEW resource_pool_object<_ObjectClass>(object)); return object; }
	template<class _ObjectClass> _ObjectClass *add_array(_ObjectClass* array, int count) { add(*EMUALLOC_SELF_NEW resource_pool_array<_ObjectClass>(array, count)); return array; }

private:
	int                     m_hash_size;
	osd_lock *              m_listlock;
	resource_pool_item **   m_hash;
	resource_pool_item *    m_ordered_head;
	resource_pool_item *    m_ordered_tail;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// dummy objects to pass to the specialized new variants
extern const zeromem_t zeromem;


resource_pool &global_resource_pool();



//**************************************************************************
//  ADDDITIONAL MACROS
//**************************************************************************

#ifndef NO_MEM_TRACKING
// re-route classic malloc-style allocations
#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(x)       malloc_array_file_line(x, __FILE__, __LINE__)
#define calloc(x,y)     __error_use_auto_alloc_clear_or_global_alloc_clear_instead__
#define realloc(x,y)    __error_realloc_is_dangerous__
#define free(x)         free_file_line(x, __FILE__, __LINE__)
#endif

#endif  /* __EMUALLOC_H__ */
