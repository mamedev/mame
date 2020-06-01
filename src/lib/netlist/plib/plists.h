// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PLISTS_H_
#define PLISTS_H_

///
/// \file plists.h
///

#include "palloc.h"
#include "pchrono.h"
#include "pstring.h"

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

namespace plib {

	/// \brief fixed size array allowing to override constructor and initialize members by placement new.
	///
	/// Use with care. This template is provided to improve locality of storage
	/// in high frequency applications. It should not be used for anything else.
	///
	///
	template <class C, std::size_t N>
	class uninitialised_array_t
	{
	public:

		using value_type = C;
		using pointer = value_type *;
		using const_pointer = const value_type *;
		using reference = value_type &;
		using const_reference = const value_type &;
		using iterator = value_type *;
		using const_iterator = const value_type *;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		//uninitialised_array_t() noexcept = default;
		uninitialised_array_t() noexcept
		: m_initialized(0)
		{
		}

		PCOPYASSIGNMOVE(uninitialised_array_t, delete)
		~uninitialised_array_t() noexcept
		{
			if (m_initialized>=N)
				for (size_type i=0; i<N; ++i)
					(*this)[i].~C();
		}

		constexpr size_t size() const noexcept { return N; }

		constexpr bool empty() const noexcept { return size() == 0; }

		reference operator[](size_type index) noexcept
		{
			return reinterpret_cast<reference>(m_buf[index]);
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			return reinterpret_cast<const_reference>(m_buf[index]);
		}

		template<typename... Args>
		void emplace(size_type index, Args&&... args)
		{
			m_initialized++;
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

		// ensure proper alignment
		PALIGNAS_VECTOROPT()
		std::array<typename std::aligned_storage<sizeof(C), alignof(C)>::type, N> m_buf;
		unsigned m_initialized;
	};

	/// \brief a simple linked list.
	///
	/// The list allows insertions deletions whilst being processed.
	///
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

			PCOPYASSIGNMOVE(element_t, delete)

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
			constexpr iter_t(iter_t &rhs) noexcept : p(rhs.p) { }
			iter_t(iter_t &&rhs) noexcept { std::swap(*this, rhs);  }

			iter_t& operator=(const iter_t &rhs) noexcept // NOLINT(bugprone-unhandled-self-assignment, cert-oop54-cpp)
			{
				if (this == &rhs)
					return *this;

				p = rhs.p;
				return *this;
			}

			iter_t& operator=(iter_t &&rhs) noexcept { std::swap(*this, rhs); return *this; }
			~iter_t() = default;

			iter_t& operator++() noexcept { p = p->next();return *this; }
			// NOLINTNEXTLINE(cert-dcl21-cpp)
			iter_t operator++(int) & noexcept { const iter_t tmp(*this); operator++(); return tmp; }

			constexpr bool operator==(const iter_t& rhs) const noexcept { return p == rhs.p; }
			constexpr bool operator!=(const iter_t& rhs) const noexcept { return p != rhs.p; }
			constexpr LC& operator*() noexcept { return *p; }
			constexpr LC* operator->() noexcept { return p; }

			constexpr LC& operator*() const noexcept { return *p; }
			constexpr LC* operator->() const noexcept { return p; }
		};

		constexpr linkedlist_t() noexcept : m_head(nullptr) {}

		constexpr iter_t begin() const noexcept { return iter_t(m_head); }
		constexpr iter_t end() const noexcept { return iter_t(nullptr); }

		void push_front(LC *elem) noexcept
		{
			elem->m_next = m_head;
			elem->m_prev = nullptr;
			if (m_head)
				m_head->m_prev = elem;
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
				// update tail
			}
		}

		constexpr LC *front() const noexcept { return m_head; }
		constexpr bool empty() const noexcept { return (m_head == nullptr); }
		void clear() noexcept
		{
			LC *p(m_head);
			while (p != nullptr)
			{
				LC *n(p->m_next);
				p->m_next = nullptr;
				p->m_prev = nullptr;
				p = n;
			}
			m_head = nullptr;
		}

	private:
		LC *m_head;
	};

} // namespace plib

#endif // PLISTS_H_
