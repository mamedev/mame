// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

#include <algorithm>
#include <vector>
#include <type_traits>
#include <cmath>

#include "pstring.h"

namespace plib {
/* ----------------------------------------------------------------------------------------
 * uninitialised_array_t:
 *      fixed size array allowing to override constructor and initialize
 *      members by placement new.
 *
 *      Use with care. This template is provided to improve locality of storage
 *      in high frequency applications. It should not be used for anything else.
 * ---------------------------------------------------------------------------------------- */

template <class C, std::size_t N>
class uninitialised_array_t
{
public:
	uninitialised_array_t()
	{
	}

	~uninitialised_array_t()
	{
		for (std::size_t i=0; i<N; i++)
			(*this)[i].~C();
	}

	size_t size() { return N; }

	C& operator[](const std::size_t &index)
	{
		return *reinterpret_cast<C *>(&m_buf[index]);
	}

	const C& operator[](const std::size_t &index) const
	{
		return *reinterpret_cast<C *>(&m_buf[index]);
	}

	template<typename... Args>
	void emplace(const std::size_t index, Args&&... args)
	{
		new (&m_buf[index]) C(std::forward<Args>(args)...);
	}

protected:

private:

	/* ensure proper alignment */
	typename std::aligned_storage<sizeof(C), alignof(C)>::type m_buf[N];
};

// ----------------------------------------------------------------------------------------
// plinkedlist_t: a simple linked list
//                the list allows insertions / deletions if used properly
// ----------------------------------------------------------------------------------------

template <class LC>
class linkedlist_t
{
public:

	struct element_t
	{
	public:

		friend class linkedlist_t<LC>;

		element_t() : m_next(nullptr) {}

		LC *next() const { return m_next; }
	private:
		LC * m_next;
	};

	struct iter_t final : public std::iterator<std::forward_iterator_tag, LC>
	{
		LC* p;
	public:
		explicit constexpr iter_t(LC* x) noexcept : p(x) {}
		explicit iter_t(const iter_t &rhs) noexcept = default;
		iter_t(iter_t &&rhs) noexcept = default;
		iter_t& operator++() noexcept {p = p->next();return *this;}
		iter_t operator++(int) noexcept {iter_t tmp(*this); operator++(); return tmp;}
		bool operator==(const iter_t& rhs) noexcept {return p==rhs.p;}
		bool operator!=(const iter_t& rhs) noexcept {return p!=rhs.p;}
		LC& operator*() noexcept {return *p;}
		LC* operator->() noexcept {return p;}
	};

	linkedlist_t() : m_head(nullptr) {}

	iter_t begin() const { return iter_t(m_head); }
	constexpr iter_t end() const { return iter_t(nullptr); }

	void push_front(LC *elem)
	{
		elem->m_next = m_head;
		m_head = elem;
	}

	void push_back(LC *elem)
	{
		LC **p = &m_head;
		while (*p != nullptr)
		{
			p = &((*p)->m_next);
		}
		*p = elem;
		elem->m_next = nullptr;
	}

	void remove(const LC *elem)
	{
		auto p = &m_head;
		for ( ; *p != elem; p = &((*p)->m_next))
		{
			//nl_assert(*p != nullptr);
		}
		(*p) = elem->m_next;
	}

	LC *front() const { return m_head; }
	void clear() { m_head = nullptr; }
	bool empty() const { return (m_head == nullptr); }

private:
	LC *m_head;
};

}

#endif /* PLISTS_H_ */
