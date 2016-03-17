// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

#include <cstring>
#include <algorithm>
#include <cmath>
#include <stack>
#include <vector>

#include "palloc.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// parray_t: dynamic array
// ----------------------------------------------------------------------------------------

template <class _ListClass>
class parray_t
{
public:

	ATTR_COLD parray_t(std::size_t numElements)
	: m_list(0), m_capacity(0)
	{
		set_capacity(numElements);
	}

	ATTR_COLD parray_t(const parray_t &rhs)
	: m_list(0), m_capacity(0)
	{
		set_capacity(rhs.size());
		for (std::size_t i=0; i<m_capacity; i++)
			m_list[i] = rhs[i];
	}

	ATTR_COLD parray_t &operator=(const parray_t &rhs)
	{
		set_capacity(rhs.size());
		for (std::size_t i=0; i<m_capacity; i++)
			m_list[i] = rhs[i];
		return *this;
	}

	~parray_t()
	{
		if (m_list != NULL)
			pfree_array(m_list);
		m_list = NULL;
	}

	ATTR_HOT  _ListClass& operator[](const std::size_t index) { return m_list[index]; }
	ATTR_HOT  const _ListClass& operator[](const std::size_t index) const { return m_list[index]; }

	ATTR_HOT  std::size_t size() const { return m_capacity; }

	void resize(const std::size_t new_size)
	{
		set_capacity(new_size);
	}

protected:
	ATTR_COLD void set_capacity(const std::size_t new_capacity)
	{
		if (m_list != NULL)
			pfree_array(m_list);
		if (new_capacity > 0)
			m_list = palloc_array(_ListClass, new_capacity);
		else
			m_list = NULL;
		m_capacity = new_capacity;
	}

private:

	_ListClass * m_list;
	int m_capacity;
};

// ----------------------------------------------------------------------------------------
// plist_t: a simple list
// ----------------------------------------------------------------------------------------

template <typename _ListClass>
class pvector_t : public std::vector<_ListClass>
{
public:
	pvector_t() : std::vector<_ListClass>() {}

	void clear_and_free()
	{
		for (_ListClass i : *this)
		{
			pfree(i);
		}
		this->clear();
	}

	bool contains(const _ListClass &elem) const
	{
		return (std::find(this->begin(), this->end(), elem) != this->end());
	}

	void remove(const _ListClass &elem)
	{
		this->erase(std::remove(this->begin(), this->end(), elem), this->end());
	}

	ATTR_HOT void insert_at(const std::size_t index, const _ListClass &elem)
	{
		this->insert(this->begin() + index, elem);
	}

	ATTR_HOT  void remove_at(const std::size_t pos)
	{
		this->erase(this->begin() + pos);
	}

	int indexof(const _ListClass &elem) const
	{
		for (int i = 0; i < this->size(); i++)
		{
			if (this->at(i) == elem)
				return i;
		}
		return -1;
	}

};

// ----------------------------------------------------------------------------------------
// plinkedlist_t: a simple linked list
//                the list allows insertions / deletions if used properly
// ----------------------------------------------------------------------------------------

template <class _ListClass>
class plinkedlist_t;

#if 1

template <class _ListClass>
struct plinkedlist_element_t
{
public:

	friend class plinkedlist_t<_ListClass>;

	plinkedlist_element_t() : m_next(NULL) {}

	_ListClass *next() const { return m_next; }
private:
	_ListClass * m_next;
};

template <class _ListClass>
class plinkedlist_t
{
public:

	plinkedlist_t() : m_head(NULL) {}

	ATTR_HOT  void insert(const _ListClass &before, _ListClass &elem)
	{
		if (m_head == &before)
		{
			elem.m_next = m_head;
			m_head = elem;
		}
		else
		{
			_ListClass *p = m_head;
			while (p != NULL)
			{
				if (p->m_next == &before)
				{
					elem->m_next = &before;
					p->m_next = &elem;
					return;
				}
				p = p->m_next;
			}
			//throw pexception("element not found");
		}
	}

	ATTR_HOT  void insert(_ListClass &elem)
	{
		elem.m_next = m_head;
		m_head = &elem;
	}

	ATTR_HOT  void add(_ListClass &elem)
	{
		_ListClass **p = &m_head;
		while (*p != NULL)
		{
			p = &((*p)->m_next);
		}
		*p = &elem;
		elem.m_next = NULL;
	}

	ATTR_HOT  void remove(const _ListClass &elem)
	{
		_ListClass **p;
		for (p = &m_head; *p != &elem; p = &((*p)->m_next))
		{
			//nl_assert(*p != NULL);
		}
		(*p) = elem.m_next;
	}

	ATTR_HOT  _ListClass *first() const { return m_head; }
	ATTR_HOT  void clear() { m_head = NULL; }
	ATTR_HOT  bool is_empty() const { return (m_head == NULL); }

private:
	_ListClass *m_head;
};
#else

template <class _ListClass>
struct plinkedlist_element_t
{
public:

	friend class plinkedlist_t<_ListClass>;

	plinkedlist_element_t() : m_next(NULL), m_prev(NULL) {}

	_ListClass *next() const { return m_next; }
private:
	_ListClass * m_next;
	_ListClass * m_prev;
};

template <class _ListClass>
class plinkedlist_t
{
public:

	plinkedlist_t() : m_head(NULL), m_tail(NULL) {}

	ATTR_HOT  void insert(_ListClass &elem)
	{
		if (m_head != NULL)
			m_head->m_prev = &elem;
		elem.m_next = m_head;
		elem.m_prev = NULL;
		m_head = &elem;
		if (m_tail == NULL)
			m_tail = &elem;
	}

	ATTR_HOT  void add(_ListClass &elem)
	{
		if (m_tail != NULL)
			m_tail->m_next = &elem;
		elem.m_prev = m_tail;
		m_tail = &elem;
		elem.m_next = NULL;
		if (m_head == NULL)
			m_head = &elem;
	}

	ATTR_HOT  void remove(const _ListClass &elem)
	{
		if (prev(elem) == NULL)
		{
			m_head = next(elem);
			if (m_tail == &elem)
				m_tail = NULL;
		}
		else
			prev(elem)->m_next = next(elem);

		if (next(elem) == NULL)
		{
			m_tail = prev(elem);
			if (m_head == &elem)
				m_head = NULL;
		}
		else
			next(elem)->m_prev = prev(elem);
	}


	ATTR_HOT static  _ListClass *next(const _ListClass &elem) { return static_cast<_ListClass *>(elem.m_next); }
	ATTR_HOT static  _ListClass *next(const _ListClass *elem) { return static_cast<_ListClass *>(elem->m_next); }
	ATTR_HOT static  _ListClass *prev(const _ListClass &elem) { return static_cast<_ListClass *>(elem.m_prev); }
	ATTR_HOT static  _ListClass *prev(const _ListClass *elem) { return static_cast<_ListClass *>(elem->m_prev); }
	ATTR_HOT  _ListClass *first() const { return m_head; }
	ATTR_HOT  void clear() { m_head = m_tail = NULL; }
	ATTR_HOT  bool is_empty() const { return (m_head == NULL); }

private:
	_ListClass *m_head;
	_ListClass *m_tail;
};
#endif

// ----------------------------------------------------------------------------------------
// string list
// ----------------------------------------------------------------------------------------

class pstring_vector_t : public pvector_t<pstring>
{
public:
	pstring_vector_t() : pvector_t<pstring>() { }

	pstring_vector_t(const pstring &str, const pstring &onstr, bool ignore_empty = false)
	: pvector_t<pstring>()
	{
		int p = 0;
		int pn;

		pn = str.find(onstr, p);
		while (pn>=0)
		{
			pstring t = str.substr(p, pn - p);
			if (!ignore_empty || t.len() != 0)
				this->push_back(t);
			p = pn + onstr.len();
			pn = str.find(onstr, p);
		}
		if (p < (int) str.len())
		{
			pstring t = str.substr(p);
			if (!ignore_empty || t.len() != 0)
				this->push_back(t);
		}
	}

	pstring_vector_t(const pstring &str, const pstring_vector_t &onstrl)
	: pvector_t<pstring>()
	{
		pstring col = "";

		unsigned i = 0;
		while (i<str.blen())
		{
			int p = -1;
			for (std::size_t j=0; j < onstrl.size(); j++)
			{
				if (std::memcmp(onstrl[j].cstr(), &(str.cstr()[i]), onstrl[j].blen())==0)
				{
					p = j;
					break;
				}
			}
			if (p>=0)
			{
				if (col != "")
					this->push_back(col);

				col = "";
				this->push_back(onstrl[p]);
				i += onstrl[p].blen();
			}
			else
			{
				pstring::traits::code_t c = pstring::traits::code(str.cstr() + i);
				col += c;
				i+=pstring::traits::codelen(c);
			}
		}
		if (col != "")
			this->push_back(col);
	}
};

// ----------------------------------------------------------------------------------------
// hashmap list
// ----------------------------------------------------------------------------------------


template <class C>
struct phash_functor
{
	phash_functor()
	{}
	phash_functor(const C &v)
	{
		m_hash = v;
	}
	friend unsigned operator%(const phash_functor &lhs, const unsigned &rhs) { return lhs.m_hash % rhs; }
	bool operator==(const phash_functor &lhs) const { return (m_hash == lhs.m_hash); }
private:
	unsigned m_hash;
};

template <>
struct phash_functor<pstring>
{
	phash_functor()
	{}
	phash_functor(const pstring &v)
	{
		/* modified djb2 */
		const pstring::mem_t *string = v.cstr();
		unsigned result = 5381;
		for (pstring::mem_t c = *string; c != 0; c = *string++)
			result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ c;
			//result = (result*33) ^ c;
		m_hash = result;
	}
	friend unsigned operator%(const phash_functor<pstring> &lhs, const unsigned &rhs) { return lhs.m_hash % rhs; }
	bool operator==(const phash_functor<pstring> &lhs) const { return (m_hash == lhs.m_hash); }
private:
	unsigned m_hash;
};

#if 0
#if 0
	unsigned hash(const pstring &v) const
	{
		/* Fowler???Noll???Vo hash - FNV-1 */
		const char *string = v.cstr();
		unsigned result = 2166136261;
		for (UINT8 c = *string++; c != 0; c = *string++)
			result = (result * 16777619) ^ c;
			// result = (result ^ c) * 16777619; FNV 1a
		return result;
	}
#else
	unsigned hash(const pstring &v) const
	{
		/* jenkins one at a time algo */
		unsigned result = 0;
		const char *string = v.cstr();
		while (*string)
		{
			result += *string;
			string++;
			result += (result << 10);
			result ^= (result >> 6);
		}
		result += (result << 3);
		result ^= (result >> 11);
		result += (result << 15);
		return result;
	}
#endif
#endif

template <class K, class V, class H = phash_functor<K> >
class phashmap_t
{
public:
	phashmap_t() : m_hash(37)
	{
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
	}

	~phashmap_t()
	{
	}

	struct element_t
	{
		element_t(const K &key, const H &hash, const V &value)
		: m_key(key), m_hash(hash), m_value(value), m_next(-1)
		{}
		K m_key;
		H m_hash;
		V m_value;
		int m_next;
	};

	void clear()
	{
#if 0
		if (0)
		{
			unsigned cnt = 0;
			for (unsigned i=0; i<m_hash.size(); i++)
				if (m_hash[i] >= 0)
					cnt++;
			const unsigned s = m_values.size();
			if (s>0)
				printf("phashmap: %d elements %d hashsize, percent in overflow: %d\n", s, (unsigned) m_hash.size(),  (s - cnt) * 100 / s);
			else
				printf("phashmap: No elements .. \n");
		}
#endif
		m_values.clear();
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
	}

	bool contains(const K &key) const
	{
		return (get_idx(key) >= 0);
	}

	int index_of(const K &key) const
	{
		return get_idx(key);
	}

	unsigned size() const { return m_values.size(); }

	bool add(const K &key, const V &value)
	{
		/*
		 * we are using the Euler prime function here
		 *
		 * n * n + n + 41 | 40 >= n >=0
		 *
		 * and accept that outside we will not have a prime
		 *
		 */
		if (m_values.size() * 3 / 2 > m_hash.size())
		{
			unsigned n = std::sqrt( 2 * m_hash.size());
			n = n * n + n + 41;
			m_hash.resize(n);
			rebuild();
		}
		const H hash(key);
		const unsigned pos = hash % m_hash.size();
		if (m_hash[pos] == -1)
		{
			unsigned vpos = m_values.size();
			m_values.push_back(element_t(key, hash, value));
			m_hash[pos] = vpos;
		}
		else
		{
			int ep = m_hash[pos];

			for (; ep != -1; ep = m_values[ep].m_next)
			{
				if (m_values[ep].m_hash == hash && m_values[ep].m_key == key )
					return false; /* duplicate */
			}
			unsigned vpos = m_values.size();
			m_values.push_back(element_t(key, hash, value));
			m_values[vpos].m_next = m_hash[pos];
			m_hash[pos] = vpos;
		}
		return true;
	}

	V& operator[](const K &key)
	{
		int p = get_idx(key);
		if (p == -1)
		{
			p = m_values.size();
			add(key, V());
		}
		return m_values[p].m_value;
	}

	V& value_at(const unsigned pos) { return m_values[pos].m_value; }
	const V& value_at(const unsigned pos) const { return m_values[pos].m_value; }

	V& key_at(const unsigned pos) { return m_values[pos].m_key; }
private:

	int get_idx(const K &key) const
	{
		H hash(key);
		const unsigned pos = hash % m_hash.size();

		for (int ep = m_hash[pos]; ep != -1; ep = m_values[ep].m_next)
			if (m_values[ep].m_hash == hash && m_values[ep].m_key == key )
				return ep;
		return -1;
	}

	void rebuild()
	{
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
		for (unsigned i=0; i<m_values.size(); i++)
		{
			unsigned pos = m_values[i].m_hash % m_hash.size();
			m_values[i].m_next = m_hash[pos];
			m_hash[pos] = i;
		}

	}
	pvector_t<element_t> m_values;
	parray_t<int> m_hash;
};

// ----------------------------------------------------------------------------------------
// sort a list ... slow, I am lazy
// elements must support ">" operator.
// ----------------------------------------------------------------------------------------

template<typename Class>
static inline void psort_list(Class &sl)
{
	for(unsigned i = 0; i < sl.size(); i++)
	{
		for(unsigned j = i + 1; j < sl.size(); j++)
			if(sl[i] > sl[j])
				std::swap(sl[i], sl[j]);

	}
}

#endif /* PLISTS_H_ */
