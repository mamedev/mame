// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

#include "nl_config.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// plinearlist_t: a simple list
// ----------------------------------------------------------------------------------------

template <class _ListClass, int _NumElem = 0>
class plinearlist_t
{
public:

	ATTR_COLD plinearlist_t(int numElements = _NumElem)
	{
		m_num_elements = numElements;
		if (m_num_elements == 0)
			m_list = NULL;
		else
			m_list = nl_alloc_array(_ListClass, m_num_elements);
		m_count = 0;
	}

	ATTR_COLD plinearlist_t(const plinearlist_t &rhs)
	{
		m_num_elements = rhs.capacity();
		if (m_num_elements == 0)
			m_list = NULL;
		else
			m_list = nl_alloc_array(_ListClass, m_num_elements);
		m_count = 0;
		for (int i=0; i<rhs.count(); i++)
		{
			this->add(rhs[i]);
		}
	}

	ATTR_COLD plinearlist_t &operator=(const plinearlist_t &rhs)
	{
		this->clear();
		for (int i=0; i<rhs.count(); i++)
		{
			this->add(rhs[i]);
		}
		return *this;
	}


	ATTR_COLD ~plinearlist_t()
	{
		if (m_list != NULL)
			nl_free_array(m_list);
		m_list = NULL;
	}

	ATTR_HOT inline operator _ListClass *  () { return m_list; }
	ATTR_HOT inline operator const _ListClass * () const { return m_list; }

	/* using the [] operator will not allow gcc to vectorize code because
	 * basically a pointer is returned.
	 * array works around this.
	 */

	ATTR_HOT inline _ListClass *array() { return m_list; }

	ATTR_HOT inline _ListClass& operator[](const int index) { return m_list[index]; }
	ATTR_HOT inline const _ListClass& operator[](const int index) const { return m_list[index]; }

	ATTR_HOT inline void add(const _ListClass &elem)
	{
		if (m_count >= m_num_elements){
			int new_size = m_num_elements * 2;
			if (new_size < 32)
				new_size = 32;
			resize(new_size);
		}

		m_list[m_count++] = elem;
	}

	ATTR_HOT inline void remove(const _ListClass &elem)
	{
		for (int i = 0; i < m_count; i++)
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

	ATTR_HOT inline void remove_at(const int pos)
	{
		nl_assert((pos>=0) && (pos<m_count));
		m_count--;
		for (int i = pos; i < m_count; i++)
		{
			m_list[i] = m_list[i+1];
		}
	}

	ATTR_HOT inline void swap(const int pos1, const int pos2)
	{
		nl_assert((pos1>=0) && (pos1<m_count));
		nl_assert((pos2>=0) && (pos2<m_count));
		_ListClass tmp = m_list[pos1];
		m_list[pos1] = m_list[pos2];
		m_list[pos2] =tmp;
	}

	ATTR_HOT inline bool contains(const _ListClass &elem) const
	{
		for (_ListClass *i = m_list; i < m_list + m_count; i++)
		{
			if (*i == elem)
				return true;
		}
		return false;
	}

	ATTR_HOT inline int indexof(const _ListClass &elem) const
	{
		for (int i = 0; i < m_count; i++)
		{
			if (m_list[i] == elem)
				return i;
		}
		return -1;
	}

	ATTR_HOT inline const _ListClass *first() const { return ((m_count > 0) ? &m_list[0] : NULL ); }
	ATTR_HOT inline const _ListClass *next(const _ListClass *lc) const { return ((lc < last()) ? lc + 1 : NULL ); }
	ATTR_HOT inline const _ListClass *last() const { return &m_list[m_count -1]; }
	ATTR_HOT inline int count() const { return m_count; }
	ATTR_HOT inline bool is_empty() const { return (m_count == 0); }
	ATTR_HOT inline void clear() { m_count = 0; }
	ATTR_HOT inline int capacity() const { return m_num_elements; }

	ATTR_COLD void clear_and_free()
	{
		for (_ListClass *i = m_list; i < m_list + m_count; i++)
		{
			nl_free(*i);
		}
		clear();
	}

private:
	ATTR_COLD void resize(const int new_size)
	{
		int cnt = count();
		if (new_size > 0)
		{
			_ListClass *m_new = nl_alloc_array(_ListClass, new_size);
			_ListClass *pd = m_new;

			if (cnt > new_size)
				cnt = new_size;
			for (_ListClass *ps = m_list; ps < m_list + cnt; ps++, pd++)
				*pd = *ps;
			if (m_list != NULL)
				nl_free_array(m_list);
			m_list = m_new;
			m_count = cnt;
		}
		else
		{
			if (m_list != NULL)
				nl_free_array(m_list);
			m_list = NULL;
			m_count = 0;
		}
		m_num_elements = new_size;
	}

	int m_count;
	_ListClass * m_list /* ATTR_ALIGN */;
	int m_num_elements;
};

// ----------------------------------------------------------------------------------------
// pnamedlist_t: a simple list
// ----------------------------------------------------------------------------------------

#if (defined(__sun__) && defined(__svr4__)) || defined(__ANDROID__) || defined(__OpenBSD__)
#undef _C
#endif

template <class _C>
class pnamedlist_t : public plinearlist_t<_C>
{
public:
	_C find(const pstring name) const
	{
		for (int i=0; i < this->count(); i++)
			if (get_name((*this)[i]) == name)
				return (*this)[i];
		return _C(NULL);
	}

	void remove_by_name(const pstring name)
	{
		plinearlist_t<_C>::remove(find(name));
	}

	bool add(_C dev, bool allow_duplicate)
	{
		if (allow_duplicate)
			plinearlist_t<_C>::add(dev);
		else
		{
			if (!(this->find(get_name(dev)) == _C(NULL)))
				return false;
			plinearlist_t<_C>::add(dev);
		}
		return true;
	}

private:
	template <typename T> static const pstring get_name(T &elem) { return elem.name(); }
	template <typename T> static const pstring get_name(T *elem) { return elem->name(); }

};


// ----------------------------------------------------------------------------------------
// pstack_t: a simple stack
// ----------------------------------------------------------------------------------------

template <class _StackClass, int _NumElem = 128>
class pstack_t
{
public:

	ATTR_COLD pstack_t(int numElements = _NumElem)
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


	ATTR_COLD ~pstack_t()
	{
	}

	ATTR_HOT inline void push(const _StackClass &elem)
	{
		m_list.add(elem);
	}

	ATTR_HOT inline _StackClass peek() const
	{
		return m_list[m_list.count() - 1];
	}

	ATTR_HOT inline _StackClass pop()
	{
		_StackClass ret = peek();
		m_list.remove_at(m_list.count() - 1);
		return ret;
	}

	ATTR_HOT inline int count() const { return m_list.count(); }
	ATTR_HOT inline bool empty() const { return (m_list.count() == 0); }
	ATTR_HOT inline void reset() { m_list.reset(); }
	ATTR_HOT inline int capacity() const { return m_list.capacity(); }

private:
	plinearlist_t<_StackClass, _NumElem> m_list;
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

	ATTR_HOT inline void insert(const _ListClass &before, _ListClass &elem)
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
			nl_assert_always(false, "element not found");
		}
	}

	ATTR_HOT inline void insert(_ListClass &elem)
	{
		elem.m_next = m_head;
		m_head = &elem;
	}

	ATTR_HOT inline void add(_ListClass &elem)
	{
		_ListClass **p = &m_head;
		while (*p != NULL)
		{
			p = &((*p)->m_next);
		}
		*p = &elem;
		elem.m_next = NULL;
	}

	ATTR_HOT inline void remove(const _ListClass &elem)
	{
		_ListClass **p = &m_head;
		while (*p != &elem)
		{
			nl_assert(*p != NULL);
			p = &((*p)->m_next);
		}
		(*p) = elem.m_next;
	}


	ATTR_HOT static inline _ListClass *next(const _ListClass &elem) { return elem.m_next; }
	ATTR_HOT static inline _ListClass *next(const _ListClass *elem) { return elem->m_next; }
	ATTR_HOT inline _ListClass *first() const { return m_head; }
	ATTR_HOT inline void clear() { m_head = NULL; }
	ATTR_HOT inline bool is_empty() const { return (m_head == NULL); }

private:
	_ListClass *m_head;
};

#endif /* PLISTS_H_ */
