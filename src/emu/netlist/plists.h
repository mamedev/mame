// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

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

	ATTR_HOT /* inline */ _ListClass *data() { return m_list; }

	ATTR_HOT /* inline */ _ListClass& operator[](std::size_t index) { return m_list[index]; }
	ATTR_HOT /* inline */ const _ListClass& operator[](std::size_t index) const { return m_list[index]; }

	ATTR_HOT /* inline */ std::size_t size() const { return m_capacity; }

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

	/* inline */ void add(const _ListClass &elem) { this->push_back(elem); }
	void clear_and_free()
	{
		for (_ListClass *i = this->data(); i < this->data() + this->size(); i++)
		{
			pfree(*i);
		}
		this->clear();
	}
	/* inline */ bool contains(const _ListClass &elem) const
	{
		for (const _ListClass *i = this->data(); i < this->data() + this->size(); i++)
		{
			if (*i == elem)
				return true;
		}
		return false;
	}

	/* inline */ void remove(const _ListClass &elem)
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
	/* inline */ void remove_at(const int pos)
	{
		this->erase(this->begin() + pos);
	}

	/* inline */ int indexof(const _ListClass &elem) const
	{
		for (int i = 0; i < this->size(); i++)
		{
			if (this->at(i) == elem)
				return i;
		}
		return -1;
	}

	ATTR_HOT /* inline */ void swap(const int pos1, const int pos2)
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

	ATTR_HOT /* inline */ _ListClass *data() { return m_list; }

	ATTR_HOT /* inline */ _ListClass& operator[](std::size_t index) { return *(m_list + index); }
	ATTR_HOT /* inline */ const _ListClass& operator[](std::size_t index) const { return *(m_list + index); }

	ATTR_HOT /* inline */ void add(const _ListClass &elem)
	{
		if (m_count >= m_capacity){
			std::size_t new_size = m_capacity * 2;
			if (new_size < 32)
				new_size = 32;
			set_capacity(new_size);
		}

		m_list[m_count++] = elem;
	}

	ATTR_HOT /* inline */ void remove(const _ListClass &elem)
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

	ATTR_HOT /* inline */ void remove_at(const std::size_t pos)
	{
		//nl_assert((pos>=0) && (pos<m_count));
		m_count--;
		for (int i = pos; i < m_count; i++)
		{
			m_list[i] = m_list[i+1];
		}
	}

	ATTR_HOT /* inline */ void swap(const std::size_t pos1, const std::size_t pos2)
	{
		//nl_assert((pos1>=0) && (pos1<m_count));
		//nl_assert((pos2>=0) && (pos2<m_count));
		_ListClass tmp = m_list[pos1];
		m_list[pos1] = m_list[pos2];
		m_list[pos2] =tmp;
	}

	ATTR_HOT /* inline */ bool contains(const _ListClass &elem) const
	{
		for (_ListClass *i = m_list; i < m_list + m_count; i++)
		{
			if (*i == elem)
				return true;
		}
		return false;
	}

	ATTR_HOT /* inline */ int indexof(const _ListClass &elem) const
	{
		for (std::size_t i = 0; i < m_count; i++)
		{
			if (m_list[i] == elem)
				return i;
		}
		return -1;
	}

	ATTR_HOT /* inline */ std::size_t size() const { return m_count; }
	ATTR_HOT /* inline */ bool is_empty() const { return (m_count == 0); }
	ATTR_HOT /* inline */ void clear() { m_count = 0; }
	ATTR_HOT /* inline */ std::size_t capacity() const { return m_capacity; }

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
			for (_ListClass *ps = m_list; ps < m_list + cnt; ps++, pd++)
				*pd = *ps;
			if (m_list != NULL)
				this->dealloc(m_list);
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
	_ListClass find(const pstring &name) const
	{
		for (int i=0; i < this->size(); i++)
			if (get_name((*this)[i]) == name)
				return (*this)[i];
		return _ListClass(NULL);
	}

	void remove_by_name(const pstring &name)
	{
		plist_t<_ListClass>::remove(find(name));
	}

	bool add(_ListClass dev, bool allow_duplicate)
	{
		if (allow_duplicate)
			plist_t<_ListClass>::add(dev);
		else
		{
			if (!(this->find(get_name(dev)) == _ListClass(NULL)))
				return false;
			plist_t<_ListClass>::add(dev);
		}
		return true;
	}

private:
	template <typename T> static const pstring get_name(const T *elem) { return elem->name(); }
	template <typename T> static const pstring get_name(T *elem) { return elem->name(); }
	template <typename T> static const pstring get_name(const T &elem) { return elem.name(); }

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

	ATTR_HOT /* inline */ void push(const _StackClass &elem)
	{
		m_list.add(elem);
	}

	ATTR_HOT /* inline */ _StackClass peek() const
	{
		return m_list[m_list.size() - 1];
	}

	ATTR_HOT /* inline */ _StackClass pop()
	{
		_StackClass ret = peek();
		m_list.remove_at(m_list.size() - 1);
		return ret;
	}

	ATTR_HOT /* inline */ int count() const { return m_list.size(); }
	ATTR_HOT /* inline */ bool empty() const { return (m_list.size() == 0); }
	ATTR_HOT /* inline */ void reset() { m_list.reset(); }
	ATTR_HOT /* inline */ int capacity() const { return m_list.capacity(); }

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

	ATTR_HOT /* inline */ void insert(const _ListClass &before, _ListClass &elem)
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

	ATTR_HOT /* inline */ void insert(_ListClass &elem)
	{
		elem.m_next = m_head;
		m_head = &elem;
	}

	ATTR_HOT /* inline */ void add(_ListClass &elem)
	{
		_ListClass **p = &m_head;
		while (*p != NULL)
		{
			p = &((*p)->m_next);
		}
		*p = &elem;
		elem.m_next = NULL;
	}

	ATTR_HOT /* inline */ void remove(const _ListClass &elem)
	{
		_ListClass **p = &m_head;
		while (*p != &elem)
		{
			//nl_assert(*p != NULL);
			p = &((*p)->m_next);
		}
		(*p) = elem.m_next;
	}


	ATTR_HOT static /* inline */ _ListClass *next(const _ListClass &elem) { return elem.m_next; }
	ATTR_HOT static /* inline */ _ListClass *next(const _ListClass *elem) { return elem->m_next; }
	ATTR_HOT /* inline */ _ListClass *first() const { return m_head; }
	ATTR_HOT /* inline */ void clear() { m_head = NULL; }
	ATTR_HOT /* inline */ bool is_empty() const { return (m_head == NULL); }

private:
	_ListClass *m_head;
};

#endif /* PLISTS_H_ */
