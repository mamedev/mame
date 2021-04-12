// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PLISTS_H_
#define PLISTS_H_

///
/// \file plists.h
///

#include "ptypes.h"

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

namespace plib {

	/// \brief Array holding uninitialized elements
	///
	/// Use with care. This template is provided to improve locality of storage
	/// in high frequency applications. It should not be used for anything else.
	///
	template <class C, std::size_t N>
	class uninitialised_array
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
		uninitialised_array() noexcept = default;

		uninitialised_array(const uninitialised_array &) = default;
		uninitialised_array &operator=(const uninitialised_array &) = default;
		uninitialised_array(uninitialised_array &&) noexcept = default;
		uninitialised_array &operator=(uninitialised_array &&) noexcept = default;

		~uninitialised_array() noexcept = default;

		constexpr size_t size() const noexcept { return N; }

		constexpr bool empty() const noexcept { return size() == 0; }

		reference operator[](size_type index) noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<reference>(m_buf[index]);
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<const_reference>(m_buf[index]);
		}

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator begin() const noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator end() const noexcept { return reinterpret_cast<iterator>(&m_buf[0] + N); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator begin() noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator end() noexcept { return reinterpret_cast<iterator>(&m_buf[0] + N); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		const_iterator cbegin() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		const_iterator cend() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0] + N); }

	protected:

	private:
		std::array<typename std::aligned_storage<sizeof(C), alignof(C)>::type, N> m_buf;
	};

	/// \brief fixed allocation vector
	///
	/// Currently only emplace_back and clear are supported.
	///
	/// Use with care. This template is provided to improve locality of storage
	/// in high frequency applications. It should not be used for anything else.
	///
	template <class C, std::size_t N>
	class static_vector
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

		static_vector() noexcept
		: m_pos(0)
		{
		}

		static_vector(const static_vector &) = default;
		static_vector &operator=(const static_vector &) = default;
		static_vector(static_vector &&) noexcept = default;
		static_vector &operator=(static_vector &&) noexcept = default;

		~static_vector() noexcept
		{
			clear();
		}

		constexpr size_t size() const noexcept { return m_pos; }

		constexpr bool empty() const noexcept { return size() == 0; }

		void clear()
		{
			for (size_type i=0; i<m_pos; ++i)
				(*this)[i].~C();
			m_pos = 0;
		}

		template<typename... Args>
		void emplace_back(Args&&... args)
		{
			// placement new on buffer
			new (&m_buf[m_pos]) C(std::forward<Args>(args)...);
			m_pos++;
		}

		reference operator[](size_type index) noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<reference>(m_buf[index]);
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<const_reference>(m_buf[index]);
		}

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator begin() const noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator end() const noexcept { return reinterpret_cast<iterator>(&m_buf[0] + m_pos); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator begin() noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		iterator end() noexcept { return reinterpret_cast<iterator>(&m_buf[0] + m_pos); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		const_iterator cbegin() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		const_iterator cend() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0] + m_pos); }

	protected:

	private:
		std::array<typename std::aligned_storage<sizeof(C), alignof(C)>::type, N> m_buf;
		size_type m_pos;
	};

	/// \brief a simple linked list.
	///
	/// The list allows insertions and deletions whilst being processed.
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

			element_t(const element_t &) = default; \
			element_t &operator=(const element_t &) = default;
			element_t(element_t &&) noexcept = default;
			element_t &operator=(element_t &&) noexcept = default;

			constexpr LC * &next() noexcept { return m_next; }
			constexpr LC * &prev() noexcept { return m_prev; }
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
			constexpr iter_t(const iter_t &rhs) noexcept = default;
			iter_t(iter_t &&rhs) noexcept = default;

			iter_t& operator=(const iter_t &rhs) noexcept = default;
			iter_t& operator=(iter_t &&rhs) noexcept  = default;
			~iter_t() noexcept = default;

			iter_t& operator++() noexcept { p = p->next(); return *this; }
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
