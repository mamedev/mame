// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

#include "pstring.h"

#include <array>
#include <type_traits>
#include <vector>

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

	using iterator = C *;
	using const_iterator = const C *;

	uninitialised_array_t() = default;

	COPYASSIGNMOVE(uninitialised_array_t, delete)
	~uninitialised_array_t()
	{
		for (std::size_t i=0; i<N; i++)
			(*this)[i].~C();
	}

	size_t size() const { return N; }

	C& operator[](const std::size_t &index) noexcept
	{
		return *reinterpret_cast<C *>(&m_buf[index]);
	}

	const C& operator[](const std::size_t &index) const noexcept
	{
		return *reinterpret_cast<const C *>(&m_buf[index]);
	}

	template<typename... Args>
	void emplace(const std::size_t index, Args&&... args)
	{
		// allocate on buffer
		new (&m_buf[index]) C(std::forward<Args>(args)...);
	}

	iterator begin() const noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
	iterator end() const noexcept { return reinterpret_cast<iterator>(&m_buf[N]); }

	iterator begin() noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
	iterator end() noexcept { return reinterpret_cast<iterator>(&m_buf[N]); }

	const_iterator cbegin() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0]); }
	const_iterator cend() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[N]); }

protected:

private:

	/* ensure proper alignment */
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	std::array<typename std::aligned_storage<sizeof(C), alignof(C)>::type, N> m_buf;
};

// ----------------------------------------------------------------------------------------
// plinkedlist_t: a simple linked list
//                the list allows insertions / deletions if used properly
// ----------------------------------------------------------------------------------------

#if 0
template <class LC>
class linkedlist_t
{
public:

	struct element_t
	{
	public:

		friend class linkedlist_t<LC>;

		constexpr element_t() : m_next(nullptr) {}
		constexpr element_t(const element_t &rhs) = delete;
		constexpr element_t(element_t &&rhs) = delete;

		constexpr LC *next() const noexcept { return m_next; }

	protected:
		~element_t() = default;
	private:
		LC * m_next;
	};

	struct iter_t final : public std::iterator<std::forward_iterator_tag, LC>
	{
	private:
		LC* p;
	public:
		explicit constexpr iter_t(LC* x) noexcept : p(x) { }
		explicit constexpr iter_t(const iter_t &rhs) noexcept : p(rhs.p) { }
		iter_t(iter_t &&rhs) noexcept { std::swap(*this, rhs);  }
		iter_t& operator=(const iter_t &rhs) { iter_t t(rhs); std::swap(*this, t); return *this; }
		iter_t& operator=(iter_t &&rhs) { std::swap(*this, rhs); return *this; }
		iter_t& operator++() noexcept {p = p->next();return *this;}
		iter_t operator++(int) noexcept {iter_t tmp(*this); operator++(); return tmp;}
		constexpr bool operator==(const iter_t& rhs) const noexcept {return p == rhs.p;}
		constexpr bool operator!=(const iter_t& rhs) const noexcept {return p != rhs.p;}
		/* constexpr */ LC& operator*() noexcept {return *p;}
		/* constexpr */ LC* operator->() noexcept {return p;}

		constexpr LC& operator*() const noexcept {return *p;}
		constexpr LC* operator->() const noexcept {return p;}
	};

	constexpr linkedlist_t() : m_head(nullptr) {}

	constexpr iter_t begin() const noexcept { return iter_t(m_head); }
	constexpr iter_t end() const noexcept { return iter_t(nullptr); }

	void push_front(LC *elem) noexcept
	{
		elem->m_next = m_head;
		m_head = elem;
	}

	void push_back(LC *elem) noexcept
	{
		LC **p = &m_head;
		while (*p != nullptr)
		{
			p = &((*p)->m_next);
		}
		*p = elem;
		elem->m_next = nullptr;
	}

	void remove(const LC *elem) noexcept
	{
		auto p = &m_head;
		while(*p != elem)
		{
			//nl_assert(*p != nullptr);
			p = &((*p)->m_next);
		}
		(*p) = elem->m_next;
	}

	LC *front() const noexcept { return m_head; }
	void clear() noexcept { m_head = nullptr; }
	constexpr bool empty() const noexcept { return (m_head == nullptr); }

private:
	LC *m_head;
};
#else
template <class LC>
class linkedlist_t
{
public:

	struct element_t
	{
	public:

		friend class linkedlist_t<LC>;

		constexpr element_t() : m_next(nullptr), m_prev(nullptr) {}
		~element_t() noexcept = default;

		COPYASSIGNMOVE(element_t, delete)

		constexpr LC *next() const noexcept { return m_next; }
		constexpr LC *prev() const noexcept { return m_prev; }
	private:
		LC * m_next;
		LC * m_prev;
	};

	struct iter_t final : public std::iterator<std::forward_iterator_tag, LC>
	{
	private:
		LC* p;
	public:
		explicit constexpr iter_t(LC* x) noexcept : p(x) { }
		explicit constexpr iter_t(iter_t &rhs) noexcept : p(rhs.p) { }
		iter_t(iter_t &&rhs) noexcept { std::swap(*this, rhs);  }
		iter_t& operator=(const iter_t &rhs) { iter_t t(rhs); std::swap(*this, t); return *this; }
		iter_t& operator=(iter_t &&rhs) noexcept { std::swap(*this, rhs); return *this; }
		iter_t& operator++() noexcept {p = p->next();return *this;}
		// NOLINTNEXTLINE(cert-dcl21-cpp)
		iter_t operator++(int) & noexcept {const iter_t tmp(*this); operator++(); return tmp;}

		~iter_t() = default;

		constexpr bool operator==(const iter_t& rhs) const noexcept {return p == rhs.p;}
		constexpr bool operator!=(const iter_t& rhs) const noexcept {return p != rhs.p;}
		/* constexpr */ LC& operator*() noexcept {return *p;}
		/* constexpr */ LC* operator->() noexcept {return p;}

		constexpr LC& operator*() const noexcept {return *p;}
		constexpr LC* operator->() const noexcept {return p;}
	};

	constexpr linkedlist_t() : m_head(nullptr) {}

	constexpr iter_t begin() const noexcept { return iter_t(m_head); }
	constexpr iter_t end() const noexcept { return iter_t(nullptr); }

	void push_front(LC *elem) noexcept
	{
		elem->m_next = m_head;
		elem->m_prev = nullptr;
		if (elem->m_next)
			elem->m_next->m_prev = elem;
		m_head = elem;
	}

	void push_back(LC *elem) noexcept
	{
		LC ** p(&m_head);
		LC *  prev(nullptr);
		while (*p != nullptr)
		{
			prev = *p;
			p = &((*p)->m_next);
		}
		*p = elem;
		elem->m_prev = prev;
		elem->m_next = nullptr;
	}

	void remove(const LC *elem) noexcept
	{
		if (elem->m_prev)
			elem->m_prev->m_next = elem->m_next;
		else
			m_head = elem->m_next;
		if (elem->m_next)
			elem->m_next->m_prev = elem->m_prev;
		else
		{
			/* update tail */
		}
	}

	LC *front() const noexcept { return m_head; }
	void clear() noexcept { m_head = nullptr; }
	constexpr bool empty() const noexcept { return (m_head == nullptr); }

private:
	LC *m_head;
};
#endif
} // namespace plib

#endif /* PLISTS_H_ */
