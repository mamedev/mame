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
//  MACROS
//**************************************************************************

// pool allocation helpers
#define pool_alloc(_pool, _type)					(_pool).add_object(new(__FILE__, __LINE__) _type)
#define pool_alloc_clear(_pool, _type)				(_pool).add_object(new(__FILE__, __LINE__, zeromem) _type)
#define pool_alloc_array(_pool, _type, _num)		(_pool).add_array(new(__FILE__, __LINE__) _type[_num], (_num))
#define pool_alloc_array_clear(_pool, _type, _num)	(_pool).add_array(new(__FILE__, __LINE__, zeromem) _type[_num], (_num))
#define pool_free(_pool, v)							(_pool).remove(v)

// global allocation helpers
#define global_alloc(_type)							pool_alloc(global_resource_pool, _type)
#define global_alloc_clear(_type)					pool_alloc_clear(global_resource_pool, _type)
#define global_alloc_array(_type, _num)				pool_alloc_array(global_resource_pool, _type, _num)
#define global_alloc_array_clear(_type, _num)		pool_alloc_array_clear(global_resource_pool, _type, _num)
#define global_free(v)								pool_free(global_resource_pool, v)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// zeromem_t is a dummy class used to tell new to zero memory after allocation
class zeromem_t { };


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
		  m_size(size) { }
	virtual ~resource_pool_item() { }

	resource_pool_item *	m_next;
	resource_pool_item *	m_ordered_next;
	resource_pool_item *	m_ordered_prev;
	void *					m_ptr;
	size_t					m_size;
};


// a resource_pool_object is a simple object wrapper for the templatized type
template<class T> class resource_pool_object : public resource_pool_item
{
private:
	resource_pool_object<T>(const resource_pool_object<T> &);
	resource_pool_object<T> &operator=(const resource_pool_object<T> &);

public:
	resource_pool_object(T *object)
		: resource_pool_item(reinterpret_cast<void *>(object), sizeof(T)),
		  m_object(object) { }
	virtual ~resource_pool_object() { delete m_object; }

private:
	T *						m_object;
};


// a resource_pool_array is a simple object wrapper for an allocated array of
// the templatized type
template<class T> class resource_pool_array : public resource_pool_item
{
private:
	resource_pool_array<T>(const resource_pool_array<T> &);
	resource_pool_array<T> &operator=(const resource_pool_array<T> &);

public:
	resource_pool_array(T *array, int count)
		: resource_pool_item(reinterpret_cast<void *>(array), sizeof(T) * count),
		  m_array(array),
		  m_count(count) { }
	virtual ~resource_pool_array() { delete[] m_array; }

private:
	T *						m_array;
	int 					m_count;
};


// a resource pool tracks items and frees them upon reset or destruction
class resource_pool
{
private:
	resource_pool(const resource_pool &);
	resource_pool &operator=(const resource_pool &);

public:
	resource_pool();
	~resource_pool();

	void add(resource_pool_item &item);
	void remove(resource_pool_item &item) { remove(item.m_ptr); }
	void remove(void *ptr);
	void remove(const void *ptr) { remove(const_cast<void *>(ptr)); }
	resource_pool_item *find(void *ptr);
	bool contains(void *ptrstart, void *ptrend);
	void clear();

	template<class T> T *add_object(T* object) { add(*new(__FILE__, __LINE__) resource_pool_object<T>(object)); return object; }
	template<class T> T *add_array(T* array, int count) { add(*new(__FILE__, __LINE__) resource_pool_array<T>(array, count)); return array; }

private:
	static const int		k_hash_prime = 193;

	osd_lock *				m_listlock;
	resource_pool_item *	m_hash[k_hash_prime];
	resource_pool_item *	m_ordered_head;
	resource_pool_item *	m_ordered_tail;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// global resource pool
extern resource_pool global_resource_pool;

// dummy objects to pass to the specialized new variants
extern const zeromem_t zeromem;



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// allocate memory with file and line number information
void *malloc_file_line(size_t size, const char *file, int line);

// free memory with file and line number information
void free_file_line(void *memory, const char *file, int line);

// called from the exit path of any code that wants to check for unfreed memory
void dump_unfreed_mem();



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

// standard new/delete operators (try to avoid using)
inline void *operator new(std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

inline void *operator new[](std::size_t size) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, NULL, 0);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

inline void operator delete(void *ptr)
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}

inline void operator delete[](void *ptr)
{
	if (ptr != NULL)
		free_file_line(ptr, NULL, 0);
}


// file/line new/delete operators
inline void *operator new(std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

inline void *operator new[](std::size_t size, const char *file, int line) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

inline void operator delete(void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}

inline void operator delete[](void *ptr, const char *file, int line)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}


// file/line new/delete operators with zeroing
inline void *operator new(std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}

inline void *operator new[](std::size_t size, const char *file, int line, const zeromem_t &) throw (std::bad_alloc)
{
	void *result = malloc_file_line(size, file, line);
	if (result == NULL)
		throw std::bad_alloc();
	memset(result, 0, size);
	return result;
}

inline void operator delete(void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}

inline void operator delete[](void *ptr, const char *file, int line, const zeromem_t &)
{
	if (ptr != NULL)
		free_file_line(ptr, file, line);
}



//**************************************************************************
//  ADDDITIONAL MACROS
//**************************************************************************

// re-route classic malloc-style allocations
#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(x)		malloc_file_line(x, __FILE__, __LINE__)
#define calloc(x,y)		__error_use_auto_alloc_clear_or_global_alloc_clear_instead__
#define realloc(x,y)	__error_realloc_is_dangerous__
#define free(x)			free_file_line(x, __FILE__, __LINE__)

// disable direct deletion
#define delete			__error_use_pool_free_mechanisms__


#endif	/* __EMUALLOC_H__ */
