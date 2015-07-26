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

	/* using the [] operator will not allow gcc to vectorize code because
	 * basically a pointer is returned.
	 * array works around this.
	 */

	ATTR_HOT  _ListClass *data() { return m_list; }

	ATTR_HOT  _ListClass& operator[](std::size_t index) { return m_list[index]; }
	ATTR_HOT  const _ListClass& operator[](std::size_t index) const { return m_list[index]; }

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
#if 0
#include <vector>
//#define plist_t std::vector
template <typename _ListClass>
class plist_t : public std::vector<_ListClass>
{
public:
	plist_t() : std::vector<_ListClass>() {}
	plist_t(const int numElements) : std::vector<_ListClass>(numElements) {}

		void add(const _ListClass &elem) { this->push_back(elem); }
	void clear_and_free()
	{
		for (_ListClass *i = this->data(); i < this->data() + this->size(); i++)
		{
			pfree(*i);
		}
		this->clear();
	}
		bool contains(const _ListClass &elem) const
	{
		for (const _ListClass *i = this->data(); i < this->data() + this->size(); i++)
		{
			if (*i == elem)
				return true;
		}
		return false;
	}

		void remove(const _ListClass &elem)
	{
		for (int i = 0; i < this->size(); i++)
		{
			if (this->at(i) == elem)
			{
				this->erase(this->begin() + i);
				return;
			}
		}
	}
		void remove_at(const int pos)
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

	ATTR_HOT  void swap(const int pos1, const int pos2)
	{
		//nl_assert((pos1>=0) && (pos1<m_count));
		//nl_assert((pos2>=0) && (pos2<m_count));
		_ListClass tmp = (*this)[pos1];
		(*this)[pos1] = (*this)[pos2];
		(*this)[pos2] =tmp;
	}

};

#else
template <typename _ListClass>
class plist_t
{
public:

	ATTR_COLD plist_t(const std::size_t numElements = 0)
	{
		m_capacity = numElements;
		if (m_capacity == 0)
			m_list = NULL;
		else
			m_list = this->alloc(m_capacity);
		m_count = 0;
	}

	ATTR_COLD plist_t(const plist_t &rhs)
	{
		m_capacity = rhs.capacity();
		if (m_capacity == 0)
			m_list = NULL;
		else
			m_list = this->alloc(m_capacity);
		m_count = 0;
		for (std::size_t i=0; i<rhs.size(); i++)
		{
			this->add(rhs[i]);
		}
	}

	ATTR_COLD plist_t &operator=(const plist_t &rhs)
	{
		this->clear();
		for (std::size_t i=0; i<rhs.size(); i++)
		{
			this->add(rhs[i]);
		}
		return *this;
	}


	~plist_t()
	{
		if (m_list != NULL)
			this->dealloc(m_list);
		m_list = NULL;
	}

	ATTR_HOT  _ListClass *data() { return m_list; }

	ATTR_HOT  _ListClass& operator[](std::size_t index) { return *(m_list + index); }
	ATTR_HOT  const _ListClass& operator[](std::size_t index) const { return *(m_list + index); }

	ATTR_HOT  void add(const _ListClass &elem)
	{
		if (m_count >= m_capacity){
			std::size_t new_size = m_capacity * 2;
			if (new_size < 32)
				new_size = 32;
			set_capacity(new_size);
		}

		m_list[m_count++] = elem;
	}

	ATTR_HOT  void insert_at(const _ListClass &elem, const std::size_t index)
	{
		if (m_count >= m_capacity){
			std::size_t new_size = m_capacity * 2;
			if (new_size < 32)
				new_size = 32;
			set_capacity(new_size);
		}
		for (std::size_t i = m_count; i>index; i--)
			m_list[i] = m_list[i-1];
		m_list[index] = elem;
		m_count++;
	}

	ATTR_HOT  void remove(const _ListClass &elem)
	{
		for (std::size_t i = 0; i < m_count; i++)
		{
			if (m_list[i] == elem)
			{
				m_count --;
				while (i < m_count)
				{
					m_list[i] = m_list[i+1];
					i++;
				}
				return;
			}
		}
	}

	ATTR_HOT  void remove_at(const std::size_t pos)
	{
		//nl_assert((pos>=0) && (pos<m_count));
		m_count--;
		for (std::size_t i = pos; i < m_count; i++)
		{
			m_list[i] = m_list[i+1];
		}
	}

	ATTR_HOT  void swap(const std::size_t pos1, const std::size_t pos2)
	{
		//nl_assert((pos1>=0) && (pos1<m_count));
		//nl_assert((pos2>=0) && (pos2<m_count));
		_ListClass tmp = m_list[pos1];
		m_list[pos1] = m_list[pos2];
		m_list[pos2] =tmp;
	}

	ATTR_HOT  bool contains(const _ListClass &elem) const
	{
		for (_ListClass *i = m_list; i < m_list + m_count; i++)
		{
			if (*i == elem)
				return true;
		}
		return false;
	}

	ATTR_HOT  int indexof(const _ListClass &elem) const
	{
		for (std::size_t i = 0; i < m_count; i++)
		{
			if (m_list[i] == elem)
				return i;
		}
		return -1;
	}

	ATTR_HOT  std::size_t size() const { return m_count; }
	ATTR_HOT  bool is_empty() const { return (m_count == 0); }
	ATTR_HOT  void clear() { m_count = 0; }
	ATTR_HOT  std::size_t capacity() const { return m_capacity; }

	ATTR_COLD void clear_and_free()
	{
		for (_ListClass *i = m_list; i < m_list + m_count; i++)
		{
			pfree(*i);
		}
		clear();
	}

private:
	ATTR_COLD void set_capacity(const std::size_t new_capacity)
	{
		std::size_t cnt = size();
		if (new_capacity > 0)
		{
			_ListClass *m_new = this->alloc(new_capacity);
			_ListClass *pd = m_new;

			if (cnt > new_capacity)
				cnt = new_capacity;
			if (m_list != NULL)
			{
				for (_ListClass *ps = m_list; ps < m_list + cnt; ps++, pd++)
					*pd = *ps;
				this->dealloc(m_list);
			}
			m_list = m_new;
			m_count = cnt;
		}
		else
		{
			if (m_list != NULL)
				this->dealloc(m_list);
			m_list = NULL;
			m_count = 0;
		}
		m_capacity = new_capacity;
	}

	_ListClass *alloc(const std::size_t n)
	{
		return palloc_array(_ListClass, n);
	}

	void dealloc(_ListClass *p)
	{
		pfree_array(p);
	}

	std::size_t m_count;
	_ListClass * m_list;
	std::size_t m_capacity;
};
#endif

// ----------------------------------------------------------------------------------------
// pnamedlist_t: a simple list of elements which have a name() interface
// ----------------------------------------------------------------------------------------

template <class _ListClass>
class pnamedlist_t : public plist_t<_ListClass>
{
public:
	_ListClass find_by_name(const pstring &name) const
	{
		for (std::size_t i=0; i < this->size(); i++)
			if (get_name((*this)[i]) == name)
				return (*this)[i];
		return _ListClass(NULL);
	}

	int index_by_name(const pstring &name) const
	{
		for (std::size_t i=0; i < this->size(); i++)
			if (get_name((*this)[i]) == name)
				return (int) i;
		return -1;
	}

	void remove_by_name(const pstring &name)
	{
		plist_t<_ListClass>::remove(find_by_name(name));
	}

	bool add(_ListClass dev, bool allow_duplicate)
	{
		if (allow_duplicate)
			plist_t<_ListClass>::add(dev);
		else
		{
			if (!(this->find_by_name(get_name(dev)) == _ListClass(NULL)))
				return false;
			plist_t<_ListClass>::add(dev);
		}
		return true;
	}

private:
	template <typename T> const pstring &get_name(const T *elem) const { return elem->name(); }
	template <typename T> const pstring &get_name(T *elem) const { return elem->name(); }
	template <typename T> const pstring &get_name(const T &elem) const { return elem.name(); }

};


// ----------------------------------------------------------------------------------------
// pstack_t: a simple stack
// ----------------------------------------------------------------------------------------

template <class _StackClass>
class pstack_t
{
public:

	ATTR_COLD pstack_t(const int numElements = 128)
	: m_list(numElements)
	{
	}

	ATTR_COLD pstack_t(const pstack_t &rhs)
	: m_list(rhs.m_list)
	{
	}

	ATTR_COLD pstack_t &operator=(const pstack_t &rhs)
	{
		m_list = rhs.m_list;
		return *this;
	}


	~pstack_t()
	{
	}

	ATTR_HOT  void push(const _StackClass &elem)
	{
		m_list.add(elem);
	}

	ATTR_HOT  _StackClass peek() const
	{
		return m_list[m_list.size() - 1];
	}

	ATTR_HOT  _StackClass pop()
	{
		_StackClass ret = peek();
		m_list.remove_at(m_list.size() - 1);
		return ret;
	}

	ATTR_HOT  int count() const { return m_list.size(); }
	ATTR_HOT  bool empty() const { return (m_list.size() == 0); }
	ATTR_HOT  void reset() { m_list.reset(); }
	ATTR_HOT  int capacity() const { return m_list.capacity(); }

private:
	plist_t<_StackClass> m_list;
};

template <class _ListClass>
struct plinkedlist_element_t
{
	plinkedlist_element_t() : m_next(NULL) {}
	_ListClass * m_next;

};

// ----------------------------------------------------------------------------------------
// plinkedlist_t: a simple linked list
// ----------------------------------------------------------------------------------------

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
			//FXIME: throw a standard exception
			//nl_assert_always(false, "element not found");
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
		_ListClass **p = &m_head;
		while (*p != &elem)
		{
			//nl_assert(*p != NULL);
			p = &((*p)->m_next);
		}
		(*p) = elem.m_next;
	}


	ATTR_HOT static  _ListClass *next(const _ListClass &elem) { return elem.m_next; }
	ATTR_HOT static  _ListClass *next(const _ListClass *elem) { return elem->m_next; }
	ATTR_HOT  _ListClass *first() const { return m_head; }
	ATTR_HOT  void clear() { m_head = NULL; }
	ATTR_HOT  bool is_empty() const { return (m_head == NULL); }

private:
	_ListClass *m_head;
};

// ----------------------------------------------------------------------------------------
// string list
// ----------------------------------------------------------------------------------------

class pstring_list_t : public plist_t<pstring>
{
public:
	pstring_list_t() : plist_t<pstring>() { }

	pstring_list_t(const pstring &str, const pstring &onstr, bool ignore_empty = false)
	: plist_t<pstring>()
	{
		int p = 0;
		int pn;

		pn = str.find(onstr, p);
		while (pn>=0)
		{
			pstring t = str.substr(p, pn - p);
			if (!ignore_empty || t.len() != 0)
				this->add(t);
			p = pn + onstr.len();
			pn = str.find(onstr, p);
		}
		if (p<str.len())
		{
			pstring t = str.substr(p);
			if (!ignore_empty || t.len() != 0)
				this->add(t);
		}
	}

	static pstring_list_t splitexpr(const pstring &str, const pstring_list_t &onstrl)
	{
		pstring_list_t temp;
		pstring col = "";

		int i = 0;
		while (i<str.len())
		{
			int p = -1;
			for (std::size_t j=0; j < onstrl.size(); j++)
			{
				if (std::strncmp(onstrl[j].cstr(), &(str.cstr()[i]), onstrl[j].len())==0)
				{
					p = j;
					break;
				}
			}
			if (p>=0)
			{
				if (col != "")
					temp.add(col);
				col = "";
				temp.add(onstrl[p]);
				i += onstrl[p].len();
			}
			else
			{
				col += str.cstr()[i];
				i++;
			}
		}
		if (col != "")
			temp.add(col);
		return temp;
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
		const char *string = v.cstr();
		unsigned result = 5381;
		for (UINT8 c = *string; c != 0; c = *string++)
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
		/* Fowler–Noll–Vo hash - FNV-1 */
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
		element_t() { }
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
			m_values.add(element_t(key, hash, value));
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
			m_values.add(element_t(key, hash, value));
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
	plist_t<element_t> m_values;
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
