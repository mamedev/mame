/***************************************************************************

    tagmap.h

    Simple tag->object mapping functions.

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

#ifndef __TAGMAP_H__
#define __TAGMAP_H__

#include "osdcore.h"
#include "astring.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum tagmap_error
{
	TMERR_NONE,
	TMERR_DUPLICATE
};



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
			: m_next(NULL),
			  m_fullhash(fullhash),
			  m_tag(tag),
			  m_object(object) { }

		// accessors
		const astring &tag() const { return m_tag; }
		_ElementType object() const { return m_object; }

		// setters
		void set_object(_ElementType object) { m_object = object; }

	private:
		// internal helpers
		entry_t *next() const { return m_next; }
		UINT32 fullhash() const { return m_fullhash; }

		// internal state
		entry_t *		m_next;
		UINT32			m_fullhash;
		astring			m_tag;
		_ElementType	m_object;
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
		for (UINT32 hashindex = 0; hashindex < ARRAY_LENGTH(m_table); hashindex++)
			while (m_table[hashindex] != NULL)
				remove_common(&m_table[hashindex]);
	}

	// add/remove
	tagmap_error add(const char *tag, _ElementType object, bool replace_if_duplicate = false) { return add_common(tag, object, replace_if_duplicate, false); }
	tagmap_error add_unique_hash(const char *tag, _ElementType object, bool replace_if_duplicate = false) { return add_common(tag, object, replace_if_duplicate, true); }

	// remove by tag
	void remove(const char *tag)
	{
		UINT32 fullhash = hash(tag);
		for (entry_t **entryptr = &m_table[fullhash % ARRAY_LENGTH(m_table)]; *entryptr != NULL; entryptr = &(*entryptr)->m_next)
			if ((*entryptr)->fullhash() == fullhash && (*entryptr)->tag() == tag)
				return remove_common(entryptr);
	}

	// remove by object
	void remove(_ElementType object)
	{
		for (UINT32 hashindex = 0; hashindex < ARRAY_LENGTH(m_table); hashindex++)
			for (entry_t **entryptr = &m_table[hashindex]; *entryptr != NULL; entryptr = &(*entryptr)->m_next)
				if ((*entryptr)->object() == object)
					return remove_common(entryptr);
	}

	// find by tag
	_ElementType find(const char *tag) const { return find(tag, hash(tag)); }

	// find by tag with precomputed hash
	_ElementType find(const char *tag, UINT32 fullhash) const
	{
		for (entry_t *entry = m_table[fullhash % ARRAY_LENGTH(m_table)]; entry != NULL; entry = entry->next())
			if (entry->fullhash() == fullhash && entry->tag() == tag)
				return entry->object();
		return _ElementType(NULL);
	}

	// find by tag without checking anything but the hash
	_ElementType find_hash_only(const char *tag) const
	{
		UINT32 fullhash = hash(tag);
		for (entry_t *entry = m_table[fullhash % ARRAY_LENGTH(m_table)]; entry != NULL; entry = entry->next())
			if (entry->fullhash() == fullhash)
				return entry->object();
		return NULL;
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
		delete entry;
	}

	// internal state
	entry_t *		m_table[_HashSize];
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
	for (entry_t *entry = m_table[hashindex]; entry != NULL; entry = entry->next())
		if (entry->fullhash() == fullhash)
			if (unique_hash || entry->tag() == tag)
			{
				if (replace_if_duplicate)
					entry->set_object(object);
				return TMERR_DUPLICATE;
			}

	// now allocate a new entry and add to the head of the list
	entry_t *entry = new entry_t(tag, fullhash, object);
	entry->m_next = m_table[hashindex];
	m_table[hashindex] = entry;
	return TMERR_NONE;
}


#endif /* __TAGMAP_H__ */
