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
#include <unordered_map>

//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum tagmap_error
{
	TMERR_NONE,
	TMERR_DUPLICATE
};

// ======================> tagged_list

class add_exception
{
public:
	add_exception(const char *tag) : m_tag(tag) { }
	const char *tag() const { return m_tag.c_str(); }
private:
	std::string m_tag;
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
	void reset() { m_list.reset(); m_map.clear(); }

	// add the given object to the head of the list
	_ElementType &prepend(const char *tag, _ElementType &object)
	{
		if (!m_map.insert(std::make_pair(tag, &object)).second)
			throw add_exception(tag);
		return m_list.prepend(object);
	}

	// add the given object to the tail of the list
	_ElementType &append(const char *tag, _ElementType &object)
	{
		if (!m_map.insert(std::make_pair(tag, &object)).second)
			throw add_exception(tag);
		return m_list.append(object);
	}

	// operations by tag
	void remove(const char *tag) { auto search = m_map.find(tag);  if (search != m_map.end()) { m_list.remove(*search->second); m_map.erase(search); } }
	_ElementType *find(const char *tag) const { auto search = m_map.find(tag); return (search == m_map.end()) ? nullptr : search->second; }

private:
	// internal state
	simple_list<_ElementType>   m_list;
	std::unordered_map<std::string,_ElementType *>  m_map;
};


#endif /* __TAGMAP_H__ */
