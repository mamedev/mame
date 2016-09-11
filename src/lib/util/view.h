// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/******************************************************************************

	view.h

	STL container for a view

***************************************************************************/

#pragma once

#ifndef __VIEW_H__
#define __VIEW_H__

#include <cstddef>

namespace util {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<typename T >
class view
{
public:
	typedef std::ptrdiff_t difference_type;
	typedef std::size_t size_type;
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T *pointer;

	class iterator
	{
	public:
		typedef std::ptrdiff_t difference_type;
		typedef T value_type;
		typedef T &reference;
		typedef T *pointer;

		typedef std::random_access_iterator_tag iterator_category;

		iterator(T *ptr) : m_ptr(ptr) { }
		iterator(const iterator &that) : m_ptr(that) { }

		iterator& operator=(const iterator &that) { m_ptr = that.m_ptr; return *this; }
		bool operator==(const iterator &that) const { return m_ptr == that.m_ptr; }
		bool operator!=(const iterator &that) const { return m_ptr != that.m_ptr; }
		bool operator<(const iterator &that) const { return m_ptr < that.m_ptr; }
		bool operator>(const iterator &that) const { return m_ptr > that.m_ptr; }
		bool operator<=(const iterator &that) const { return m_ptr <= that.m_ptr; }
		bool operator>=(const iterator &that) const { return m_ptr >= that.m_ptr; }
		iterator operator+(size_type i) const { return iterator(m_ptr + i); }
		iterator operator-(size_type i) const { return iterator(m_ptr - i); }
		difference_type operator-(const iterator &that) const { return m_ptr - that.m_ptr; }

		iterator& operator++() { m_ptr++; return *this; }
		iterator operator++(int) { iterator result = *this; m_ptr++; return result; }

		reference operator*() const { return *m_ptr; }
		pointer operator->() const { return m_ptr; }
		operator T*() const { return m_ptr; }

	private:
		T *m_ptr;
	};

	class const_iterator
	{
	public:
		typedef std::ptrdiff_t difference_type;
		typedef T value_type;
		typedef const T &const_reference;
		typedef const T *const_pointer;

		typedef std::random_access_iterator_tag iterator_category;

		const_iterator(const T *ptr) : m_ptr(ptr) { }
		const_iterator(const iterator &that) : m_ptr(that) { }
		const_iterator(const const_iterator &that) : m_ptr(that) { }

		const_iterator& operator=(const const_iterator &that) { m_ptr = that.m_ptr; return *this; }
		bool operator==(const const_iterator &that) const { return m_ptr == that.m_ptr; }
		bool operator!=(const const_iterator &that) const { return m_ptr != that.m_ptr; }
		bool operator<(const const_iterator &that) const { return m_ptr < that.m_ptr; }
		bool operator>(const const_iterator &that) const { return m_ptr > that.m_ptr; }
		bool operator<=(const const_iterator &that) const { return m_ptr <= that.m_ptr; }
		bool operator>=(const const_iterator &that) const { return m_ptr >= that.m_ptr; }
		const_iterator operator+(size_type i) const { return const_iterator(m_ptr + i); }
		const_iterator operator-(size_type i) const { return const_iterator(m_ptr - i); }
		difference_type operator-(const const_iterator &that) const { return m_ptr - that.m_ptr; }

		const_iterator& operator++() { m_ptr++; return *this; }
		const_iterator operator++(int) { const_iterator result = *this; m_ptr++; return result; }

		const_reference operator*() const { return *m_ptr; }
		const_pointer operator->() const { return m_ptr; }
		operator T*() const { return m_ptr; }

	private:
		const T *m_ptr;
	};

	view(T *ptr, std::size_t size)
		: m_begin(ptr)
		, m_end(ptr + size)
	{
	}

	view(const view &that)
		: m_begin(that.m_begin)
		, m_end(that.m_end)
	{
	}

	// iteration
	iterator begin() { return m_begin; }
	const_iterator begin() const { return m_begin; }
	const_iterator cbegin() const { return m_begin; }
	iterator end() { return m_end; }
	const_iterator end() const { return m_end; }
	const_iterator cend() const { return m_end; }

	// capacity
	size_type size() const { return m_end - m_begin; }
	size_type max_size() const { return size(); }
	bool empty() const { return size() > 0; }

	// element access
	reference front() { return *this[0]; }
	const_reference front() const { return *this[0]; }
	reference back() { return *this[size() - 1]; }
	const_reference back() const { return *this[size() - 1]; }
	reference operator[] (size_type n) { return m_begin.m_ptr[n]; }
	const_reference operator[] (size_type n) const { return m_begin.m_ptr[n]; }
	reference at(size_type n) { check_in_bounds(n); return *this[n]; }
	const_reference at(size_type n) const { check_in_bounds(n); return *this[n]; }

private:
	iterator m_begin;
	iterator m_end;

	void check_in_bounds(size_type n)
	{
		if (n < 0 || n >= size())
			throw new std::out_of_range("invalid view<T> subscript");
	}
};

}; // namespace util

#endif // __VIEW_H__
