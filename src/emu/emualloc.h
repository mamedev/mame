// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emualloc.h

    Memory allocation helpers for the core emulator.

***************************************************************************/

#pragma once

#ifndef __EMUALLOC_H__
#define __EMUALLOC_H__

#include <new>
#include "osdcore.h"
#include "coretmpl.h"


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
		: m_next(nullptr),
			m_ordered_next(nullptr),
			m_ordered_prev(nullptr),
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

	void add(resource_pool_item &item, size_t size, const char *type);
	void remove(resource_pool_item &item) { remove(item.m_ptr); }
	void remove(void *ptr);
	void remove(const void *ptr) { remove(const_cast<void *>(ptr)); }
	resource_pool_item *find(void *ptr);
	bool contains(void *ptrstart, void *ptrend);
	void clear();

	template<class _ObjectClass> _ObjectClass *add_object(_ObjectClass* object) { add(*EMUALLOC_SELF_NEW resource_pool_object<_ObjectClass>(object), sizeof(_ObjectClass), typeid(_ObjectClass).name()); return object; }
	template<class _ObjectClass> _ObjectClass *add_array(_ObjectClass* array, int count) { add(*EMUALLOC_SELF_NEW resource_pool_array<_ObjectClass>(array, count), sizeof(_ObjectClass), typeid(_ObjectClass).name()); return array; }

private:
	int                     m_hash_size;
	osd_lock *              m_listlock;
	std::vector<resource_pool_item *> m_hash;
	resource_pool_item *    m_ordered_head;
	resource_pool_item *    m_ordered_tail;
	static UINT64           s_id;
};


#endif  /* __EMUALLOC_H__ */
