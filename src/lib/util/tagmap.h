// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tagmap.h

    Simple tag->object mapping functions.

***************************************************************************/

#pragma once

#ifndef __TAGMAP_H__
#define __TAGMAP_H__

#include "osdcore.h"
#include "coretmpl.h"
#include <string>
#include <utility>
#ifdef MAME_DEBUG
#include "eminline.h"
#endif



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum tagmap_error
{
	TMERR_NONE,
	TMERR_DUPLICATE
};


#ifdef MAME_DEBUG
extern INT32 g_tagmap_finds;
extern bool g_tagmap_counter_enabled;
#endif

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// generally used for small tables, though the hash size can be increased
// as necessary; good primes are: 53, 97, 193, 389, 769, 1543, 3079, 6151, etc
template<class _ElementType, int _HashSize = 53>
class tagmap_t
{
private:
	// disable copying/assignment
	tagmap_t(const tagmap_t &);
	tagmap_t &operator=(const tagmap_t &);

public:
	// an entry in the table
	class entry_t
	{
		friend class tagmap_t<_ElementType, _HashSize>;

	public:
		// construction/destruction
		entry_t(const char *tag, UINT32 fullhash, _ElementType object)
			: m_next(nullptr),
				m_fullhash(fullhash),
				m_tag(tag),
				m_object(std::move(object)) { }

		// accessors
		const std::string &tag() const { return m_tag; }
		_ElementType object() const { return m_object; }

		// setters
		void set_object(_ElementType object) { m_object = object; }

	private:
		// internal helpers
		entry_t *next() const { return m_next; }
		UINT32 fullhash() const { return m_fullhash; }

		// internal state
		entry_t *       m_next;
		UINT32          m_fullhash;
		std::string     m_tag;
		_ElementType    m_object;
	};

	// construction/destruction
	tagmap_t() { memset(m_table, 0, sizeof(m_table)); }
	~tagmap_t() { reset(); }

	// core hashing function
	UINT32 hash(const char *string) const
	{
		UINT32 result = *string++;
		for (UINT8 c = *string++; c != 0; c = *string++)
			result = (result*33) ^ c;
		return result;
	}

	// empty the list
	void reset()
	{
		for (auto & elem : m_table)
			while (elem != nullptr)
				remove_common(&elem);
	}

	// add/remove
	tagmap_error add(const char *tag, _ElementType object, bool replace_if_duplicate = false) { return add_common(tag, object, replace_if_duplicate, false); }
	tagmap_error add_unique_hash(const char *tag, _ElementType object, bool replace_if_duplicate = false) { return add_common(tag, object, replace_if_duplicate, true); }

	// remove by tag
	void remove(const char *tag)
	{
		UINT32 fullhash = hash(tag);
		for (entry_t **entryptr = &m_table[fullhash % ARRAY_LENGTH(m_table)]; *entryptr != nullptr; entryptr = &(*entryptr)->m_next)
			if ((*entryptr)->fullhash() == fullhash && (*entryptr)->tag() == tag)
				return remove_common(entryptr);
	}

	// remove by object
	void remove(_ElementType object)
	{
		for (auto & elem : m_table)
			for (entry_t **entryptr = &elem; *entryptr != nullptr; entryptr = &(*entryptr)->m_next)
				if ((*entryptr)->object() == object)
					return remove_common(entryptr);
	}

	// find by tag
	_ElementType find(const char *tag) const { return find(tag, hash(tag)); }

	// find by tag with precomputed hash
	_ElementType find(const char *tag, UINT32 fullhash) const
	{
#ifdef MAME_DEBUG
		if (g_tagmap_counter_enabled)
			atomic_increment32(&g_tagmap_finds);
#endif
		for (entry_t *entry = m_table[fullhash % ARRAY_LENGTH(m_table)]; entry != nullptr; entry = entry->next())
			if (entry->fullhash() == fullhash && entry->tag() == tag)
				return entry->object();
		return _ElementType(NULL);
	}

	// find by tag without checking anything but the hash
	_ElementType find_hash_only(const char *tag) const
	{
#ifdef MAME_DEBUG
		if (g_tagmap_counter_enabled)
			atomic_increment32(&g_tagmap_finds);
#endif
		UINT32 fullhash = hash(tag);
		for (entry_t *entry = m_table[fullhash % ARRAY_LENGTH(m_table)]; entry != nullptr; entry = entry->next())
			if (entry->fullhash() == fullhash)
				return entry->object();
		return nullptr;
	}

	// return first object in the table
	entry_t *first() const { return next(NULL); }

	// return next object in the table
	entry_t *next(entry_t *after) const
	{
		// if there's another item in this hash bucket, just return it
		if (after != NULL && after->next() != NULL)
			return after->next();

		// otherwise scan forward for the next bucket with an entry
		UINT32 firstindex = (after != NULL) ? (after->fullhash() % ARRAY_LENGTH(m_table) + 1) : 0;
		for (UINT32 hashindex = firstindex; hashindex < ARRAY_LENGTH(m_table); hashindex++)
			if (m_table[hashindex] != NULL)
				return m_table[hashindex];

		// all out
		return NULL;
	}

private:
	// internal helpers
	tagmap_error add_common(const char *tag, _ElementType object, bool replace_if_duplicate, bool unique_hash);

	// remove an entry given a pointer to its pointer
	void remove_common(entry_t **entryptr)
	{
		entry_t *entry = *entryptr;
		*entryptr = entry->next();
		global_free(entry);
	}

	// internal state
	entry_t *       m_table[_HashSize];
};


// ======================> tagged_list

class add_exception
{
public:
	add_exception(const char *tag) : m_tag(tag) { }
	const char *tag() const { return m_tag; }
private:
	const char *m_tag;
};

// a tagged_list is a class that maintains a list of objects that can be quickly looked up by tag
template<class _ElementType>
class tagged_list
{
	// we don't support deep copying
	tagged_list(const tagged_list &);
	tagged_list &operator=(const tagged_list &);

public:
	// construction
	tagged_list() { }

	// simple getters
	_ElementType *first() const { return m_list.first(); }
	_ElementType *last() const { return m_list.last(); }
	int count() const { return m_list.count(); }

	// remove (free) all objects in the list, leaving an empty list
	void reset() { m_list.reset(); m_map.reset(); }

	// add the given object to the head of the list
	_ElementType &prepend(const char *tag, _ElementType &object)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw add_exception(tag);
		return m_list.prepend(object);
	}

	// add the given object to the tail of the list
	_ElementType &append(const char *tag, _ElementType &object)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw add_exception(tag);
		return m_list.append(object);
	}

	// insert the given object after a particular object (NULL means prepend)
	_ElementType &insert_after(const char *tag, _ElementType &object, _ElementType *insert_after)
	{
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw add_exception(tag);
		return m_list.insert_after(object, insert_after);
	}

	// replace an item in the list at the same location, and remove it
	_ElementType &replace_and_remove(const char *tag, _ElementType &object, _ElementType &toreplace)
	{
		m_map.remove(&toreplace);
		m_list.replace_and_remove(object, toreplace);
		if (m_map.add_unique_hash(tag, &object, false) != TMERR_NONE)
			throw add_exception(tag);
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
	void remove(const char *tag) { _ElementType *object = find(tag); if (object != nullptr) remove(*object); }
	_ElementType *find(const char *tag) const { return m_map.find_hash_only(tag); }
	int indexof(const char *tag) const { _ElementType *object = find(tag); return (object != NULL) ? m_list.indexof(*object) : NULL; }

private:
	// internal state
	simple_list<_ElementType>   m_list;
	tagmap_t<_ElementType *>    m_map;
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  add_common - core implementation of a tagmap
//  addition
//-------------------------------------------------

template<class _ElementType, int _HashSize>
tagmap_error tagmap_t<_ElementType, _HashSize>::add_common(const char *tag, _ElementType object, bool replace_if_duplicate, bool unique_hash)
{
	UINT32 fullhash = hash(tag);
	UINT32 hashindex = fullhash % ARRAY_LENGTH(m_table);

	// first make sure we don't have a duplicate
	for (entry_t *entry = m_table[hashindex]; entry != nullptr; entry = entry->next())
		if (entry->fullhash() == fullhash)
			if (unique_hash || entry->tag() == tag)
			{
				if (replace_if_duplicate)
					entry->set_object(object);
				return TMERR_DUPLICATE;
			}

	// now allocate a new entry and add to the head of the list
	auto                                   entry = global_alloc(entry_t(tag, fullhash, object));
	entry->m_next = m_table[hashindex];
	m_table[hashindex] = entry;
	return TMERR_NONE;
}


#endif /* __TAGMAP_H__ */
