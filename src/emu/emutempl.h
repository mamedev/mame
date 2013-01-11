/***************************************************************************

    emutempl.h

    Core templates for basic non-string types.

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

#ifndef __EMUTEMPL_H__
#define __EMUTEMPL_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> simple_list

// a simple_list is a singly-linked list whose 'next' pointer is owned
// by the object
template<class _ElementType>
class simple_list
{
	// we don't support deep copying
	DISABLE_COPYING(simple_list);

public:
	// construction/destruction
	simple_list(resource_pool &pool = global_resource_pool())
		: m_head(NULL),
			m_tail(NULL),
			m_pool(pool),
			m_count(0) { }

	virtual ~simple_list() { reset(); }

	// simple getters
	resource_pool &pool() const { return m_pool; }
	_ElementType *first() const { return m_head; }
	_ElementType *last() const { return m_tail; }
	int count() const { return m_count; }

	// remove (free) all objects in the list, leaving an empty list
	void reset()
	{
		while (m_head != NULL)
			remove(*m_head);
	}

	// add the given object to the head of the list
	_ElementType &prepend(_ElementType &object)
	{
		object.m_next = m_head;
		m_head = &object;
		if (m_tail == NULL)
			m_tail = m_head;
		m_count++;
		return object;
	}

	// add the given list to the head of the list
	void prepend_list(simple_list<_ElementType> &list)
	{
		int count = list.count();
		if (count == 0)
			return;
		_ElementType *tail = list.last();
		_ElementType *head = list.detach_all();
		tail->m_next = m_head;
		m_head = head;
		if (m_tail == NULL)
			m_tail = tail;
		m_count += count;
	}

	// add the given object to the tail of the list
	_ElementType &append(_ElementType &object)
	{
		object.m_next = NULL;
		if (m_tail != NULL)
			m_tail = m_tail->m_next = &object;
		else
			m_tail = m_head = &object;
		m_count++;
		return object;
	}

	// add the given list to the tail of the list
	void append_list(simple_list<_ElementType> &list)
	{
		int count = list.count();
		if (count == 0)
			return;
		_ElementType *tail = list.last();
		_ElementType *head = list.detach_all();
		if (m_tail != NULL)
			m_tail->m_next = head;
		else
			m_head = head;
		m_tail = tail;
		m_count += count;
	}

	// insert the given object after a particular object (NULL means prepend)
	_ElementType &insert_after(_ElementType &object, _ElementType *insert_after)
	{
		if (insert_after == NULL)
			return prepend(object);
		object.m_next = insert_after->m_next;
		insert_after->m_next = &object;
		if (m_tail == insert_after)
			m_tail = &object;
		m_count++;
		return object;
	}

	// insert the given object before a particular object (NULL means append)
	_ElementType &insert_before(_ElementType &object, _ElementType *insert_before)
	{
		if (insert_before == NULL)
			return append(object);
		for (_ElementType **curptr = &m_head; *curptr != NULL; curptr = &(*curptr)->m_next)
			if (*curptr == insert_before)
			{
				object.m_next = insert_before;
				*curptr = &object;
				if (m_head == insert_before)
					m_head = &object;
				m_count++;
				return object;
			}
		return object;
	}

	// replace an item in the list at the same location, and remove it
	_ElementType &replace_and_remove(_ElementType &object, _ElementType &toreplace)
	{
		_ElementType *prev = NULL;
		for (_ElementType *cur = m_head; cur != NULL; prev = cur, cur = cur->m_next)
			if (cur == &toreplace)
			{
				if (prev != NULL)
					prev->m_next = &object;
				else
					m_head = &object;
				if (m_tail == &toreplace)
					m_tail = &object;
				object.m_next = toreplace.m_next;
				pool_free(m_pool, &toreplace);
				return object;
			}
		return append(object);
	}

	// detach the head item from the list, but don't free its memory
	_ElementType *detach_head()
	{
		_ElementType *result = m_head;
		if (result != NULL)
		{
			m_head = result->m_next;
			m_count--;
			if (m_head == NULL)
				m_tail = NULL;
		}
		return result;
	}

	// detach the given item from the list, but don't free its memory
	_ElementType &detach(_ElementType &object)
	{
		_ElementType *prev = NULL;
		for (_ElementType *cur = m_head; cur != NULL; prev = cur, cur = cur->m_next)
			if (cur == &object)
			{
				if (prev != NULL)
					prev->m_next = object.m_next;
				else
					m_head = object.m_next;
				if (m_tail == &object)
					m_tail = prev;
				m_count--;
				return object;
			}
		return object;
	}

	// deatch the entire list, returning the head, but don't free memory
	_ElementType *detach_all()
	{
		_ElementType *result = m_head;
		m_head = m_tail = NULL;
		m_count = 0;
		return result;
	}

	// remove the given object and free its memory
	void remove(_ElementType &object)
	{
		detach(object);
		pool_free(m_pool, &object);
	}

	// find an object by index in the list
	_ElementType *find(int index) const
	{
		for (_ElementType *cur = m_head; cur != NULL; cur = cur->m_next)
			if (index-- == 0)
				return cur;
		return NULL;
	}

	// return the index of the given object in the list
	int indexof(const _ElementType &object) const
	{
		int index = 0;
		for (_ElementType *cur = m_head; cur != NULL; cur = cur->m_next)
		{
			if (cur == &object)
				return index;
			index++;
		}
		return -1;
	}

private:
	// internal state
	_ElementType *  m_head;         // head of the singly-linked list
	_ElementType *  m_tail;         // tail of the singly-linked list
	resource_pool & m_pool;         // resource pool where objects are freed
	int             m_count;        // number of objects in the list
};


// ======================> simple_list_wrapper

// a simple_list_wrapper wraps an existing object with a next pointer so it
// can live in a simple_list without requiring the object to have a next
// pointer
template<class _ObjectType>
class simple_list_wrapper
{
public:
	template<class U> friend class simple_list;

	// construction/destruction
	simple_list_wrapper(_ObjectType *object)
		: m_next(NULL),
			m_object(object) { }

	// operators
	operator _ObjectType *() { return m_object; }
	operator _ObjectType *() const { return m_object; }
	_ObjectType *operator *() { return m_object; }
	_ObjectType *operator *() const { return m_object; }

	// getters
	simple_list_wrapper *next() const { return m_next; }
	_ObjectType *object() const { return m_object; }

private:
	// internal state
	simple_list_wrapper *   m_next;
	_ObjectType *           m_object;
};


// ======================> fixed_allocator

// a fixed_allocator is a simple class that maintains a free pool of objects
template<class _ItemType>
class fixed_allocator
{
	// we don't support deep copying
	DISABLE_COPYING(fixed_allocator);

public:
	// construction/destruction
	fixed_allocator(resource_pool &pool = global_resource_pool())
		: m_freelist(pool) { }

	// allocate a new item, either by recycling an old one, or by allocating a new one
	_ItemType *alloc()
	{
		_ItemType *result = m_freelist.detach_head();
		if (result == NULL)
			result = m_freelist.pool().add_object(new _ItemType);
		return result;
	}

	// reclaim an item by adding it to the free list
	void reclaim(_ItemType *item) { if (item != NULL) m_freelist.append(*item); }
	void reclaim(_ItemType &item) { m_freelist.append(item); }

	// reclaim all items from a list
	void reclaim_all(simple_list<_ItemType> &list) { m_freelist.append_list(list); }

private:
	// internal state
	simple_list<_ItemType>  m_freelist;     // list of free objects
};


// ======================> tagged_list

// a tagged_list is a class that maintains a list of objects that can be quickly looked up by tag
template<class _ElementType>
class tagged_list
{
	// we don't support deep copying
	DISABLE_COPYING(tagged_list);

public:
	// construction/destruction
	tagged_list(resource_pool &pool = global_resource_pool())
		: m_list(pool) { }

	// simple getters
	resource_pool &pool() const { return m_list.pool(); }
	_ElementType *first() const { return m_list.first(); }
	_ElementType *last() const { return m_list.last(); }
	int count() const { return m_list.count(); }

	// remove (free) all objects in the list, leaving an empty list
	void reset() { m_list.reset(); m_map.reset(); }

	// add the given object to the head of the list
	_ElementType &prepend(const char *tag, _ElementType &object)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw emu_fatalerror("Error adding object named '%s'", tag);
		return m_list.prepend(object);
	}

	// add the given object to the tail of the list
	_ElementType &append(const char *tag, _ElementType &object)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw emu_fatalerror("Error adding object named '%s'", tag);
		return m_list.append(object);
	}

	// insert the given object after a particular object (NULL means prepend)
	_ElementType &insert_after(const char *tag, _ElementType &object, _ElementType *insert_after)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw emu_fatalerror("Error adding object named '%s'", tag);
		return m_list.insert_after(object, insert_after);
	}

	// replace an item in the list at the same location, and remove it
	_ElementType &replace_and_remove(const char *tag, _ElementType &object, _ElementType &toreplace)
	{
		m_map.remove(&toreplace);
		m_list.replace_and_remove(object, toreplace);
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw emu_fatalerror("Error replacing object named '%s'", tag);
		return object;
	}

	// detach the given item from the list, but don't free its memory
	_ElementType &detach(_ElementType &object)
	{
		m_map.remove(&object);
		return m_list.detach(object);
	}

	// remove the given object and free its memory
	void remove(_ElementType &object)
	{
		m_map.remove(&object);
		return m_list.remove(object);
	}

	// find an object by index in the list
	_ElementType *find(int index) const
	{
		return m_list.find(index);
	}

	// return the index of the given object in the list
	int indexof(const _ElementType &object) const
	{
		return m_list.indexof(object);
	}

	// operations by tag
	_ElementType &replace_and_remove(const char *tag, _ElementType &object) { _ElementType *existing = find(tag); return (existing == NULL) ? append(tag, object) : replace_and_remove(tag, object, *existing); }
	void remove(const char *tag) { _ElementType *object = find(tag); if (object != NULL) remove(*object); }
	_ElementType *find(const char *tag) const { return m_map.find_hash_only(tag); }
	int indexof(const char *tag) const { _ElementType *object = find(tag); return (object != NULL) ? m_list.indexof(*object) : NULL; }

private:
	// internal state
	simple_list<_ElementType>   m_list;
	tagmap_t<_ElementType *>    m_map;
};


#endif  /* __EMUTEMPL_H__ */
