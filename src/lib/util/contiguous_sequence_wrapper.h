// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/******************************************************************************

	contiguous_sequence_wrapper.h

	STL container that points to an existing contiguous sequence

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_CONTIGUOUS_SEQUENCE_WRAPPER_H
#define MAME_LIB_UTIL_CONTIGUOUS_SEQUENCE_WRAPPER_H

#include <cstddef>

namespace util {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<typename T >
class contiguous_sequence_wrapper
{
public:
	typedef std::ptrdiff_t difference_type;
	typedef std::size_t size_type;
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;
	typedef T *pointer;
	typedef T *iterator;
	typedef const T *const_iterator;

	contiguous_sequence_wrapper(T *ptr, std::size_t size)
		: m_begin(ptr)
		, m_end(ptr + size)
	{
	}

	contiguous_sequence_wrapper(const contiguous_sequence_wrapper &that) = default;

	// iteration
	iterator begin() { return m_begin; }
	const_iterator begin() const { return m_begin; }
	const_iterator cbegin() const { return m_begin; }
	iterator end() { return m_end; }
	const_iterator end() const { return m_end; }
	const_iterator cend() const { return m_end; }

	// reverse iteration
	std::reverse_iterator<iterator> rbegin() { return std::reverse_iterator<iterator>(end() - 1); }
	std::reverse_iterator<const_iterator> rbegin() const { return std::reverse_iterator<const_iterator>(end() - 1); }
	std::reverse_iterator<const_iterator> crbegin() const { return std::reverse_iterator<const_iterator>(cend() - 1); }
	std::reverse_iterator<iterator> rend() { return std::reverse_iterator<iterator>(begin() - 1); }
	std::reverse_iterator<const_iterator> rend() const { return std::reverse_iterator<iterator>(begin() - 1); }
	std::reverse_iterator<const_iterator> crend() const { return std::reverse_iterator<iterator>(begin() - 1); }

	// capacity
	size_type size() const { return m_end - m_begin; }
	size_type max_size() const { return size(); }
	bool empty() const { return size() > 0; }

	// element access
	reference front() { return operator[](0); }
	const_reference front() const { return operator[](0); }
	reference back() { return operator[](size() - 1); }
	const_reference back() const { return operator[](size() - 1); }
	reference operator[] (size_type n) { return m_begin[n]; }
	const_reference operator[] (size_type n) const { return m_begin[n]; }
	reference at(size_type n) { check_in_bounds(n); return operator[](n); }
	const_reference at(size_type n) const { check_in_bounds(n); return operator[](n); }

private:
	iterator m_begin;
	iterator m_end;

	void check_in_bounds(size_type n)
	{
		if (n < 0 || n >= size())
			throw new std::out_of_range("invalid contiguous_sequence_wrapper<T> subscript");
	}
};

}; // namespace util

#endif // MAME_LIB_UTIL_CONTIGUOUS_SEQUENCE_WRAPPER_H
