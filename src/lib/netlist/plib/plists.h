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

		constexpr uninitialised_array() noexcept = default;

		constexpr uninitialised_array(const uninitialised_array &) = default;
		constexpr uninitialised_array &operator=(const uninitialised_array &) = default;
		constexpr uninitialised_array(uninitialised_array &&) noexcept = default;
		constexpr uninitialised_array &operator=(uninitialised_array &&) noexcept = default;

		~uninitialised_array() noexcept = default;

		constexpr size_t size() const noexcept { return N; }

		constexpr bool empty() const noexcept { return size() == 0; }

		constexpr reference operator[](size_type index) noexcept
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
		constexpr iterator begin() const noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator end() const noexcept { return reinterpret_cast<iterator>(&m_buf[0] + N); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator begin() noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator end() noexcept { return reinterpret_cast<iterator>(&m_buf[0] + N); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr const_iterator cbegin() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr const_iterator cend() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0] + N); }

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

		constexpr static_vector() noexcept
		: m_pos(0)
		{
		}

		constexpr static_vector(const static_vector &) = default;
		constexpr static_vector &operator=(const static_vector &) = default;
		constexpr static_vector(static_vector &&) noexcept = default;
		constexpr static_vector &operator=(static_vector &&) noexcept = default;

		~static_vector() noexcept
		{
			clear();
		}

		constexpr size_t size() const noexcept { return m_pos; }

		constexpr bool empty() const noexcept { return size() == 0; }

		constexpr void clear()
		{
			for (size_type i=0; i<m_pos; ++i)
				(*this)[i].~C();
			m_pos = 0;
		}

		template<typename... Args>
		constexpr void emplace_back(Args&&... args)
		{
			// placement new on buffer
			new (&m_buf[m_pos]) C(std::forward<Args>(args)...);
			m_pos++;
		}

		constexpr reference operator[](size_type index) noexcept
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
		constexpr iterator begin() const noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator end() const noexcept { return reinterpret_cast<iterator>(&m_buf[0] + m_pos); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator begin() noexcept { return reinterpret_cast<iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr iterator end() noexcept { return reinterpret_cast<iterator>(&m_buf[0] + m_pos); }

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr const_iterator cbegin() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0]); }
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		constexpr const_iterator cend() const noexcept { return reinterpret_cast<const_iterator>(&m_buf[0] + m_pos); }

	private:
		std::array<typename std::aligned_storage<sizeof(C), alignof(C)>::type, N> m_buf;
		size_type m_pos;
	};

	/// \brief a simple linked list.
	///
	/// The list allows insertions and deletions whilst being processed.
	///
	template <class LC, int TAG>
	class linked_list_t
	{
	public:
		using ttag = std::integral_constant<int, TAG>;

		struct element_t
		{
		public:
			using tag = std::integral_constant<int, TAG>;

			friend class linked_list_t<LC, TAG>;

			constexpr element_t() noexcept : m_next(nullptr), m_prev(nullptr) {}
			~element_t() noexcept = default;

			constexpr element_t(const element_t &) noexcept = default;
			constexpr element_t &operator=(const element_t &) noexcept = default;
			constexpr element_t(element_t &&) noexcept = default;
			constexpr element_t &operator=(element_t &&) noexcept = default;

		private:
			element_t * m_next;
			element_t * m_prev;
		};

		struct iter_t final : public std::iterator<std::forward_iterator_tag, LC>
		{
		private:
			element_t * p;
		public:
			using tag = std::integral_constant<int, TAG>;

			explicit constexpr iter_t(element_t * x) noexcept : p(x) { }

			constexpr iter_t(const iter_t &rhs) noexcept = default;
			constexpr iter_t(iter_t &&rhs) noexcept = default;
			constexpr iter_t& operator=(const iter_t &rhs) noexcept = default;
			constexpr iter_t& operator=(iter_t &&rhs) noexcept  = default;

			~iter_t() noexcept = default;

			iter_t& operator++() noexcept { p = p->m_next; return *this; }
			// NOLINTNEXTLINE(cert-dcl21-cpp)
			iter_t operator++(int) & noexcept { const iter_t tmp(*this); operator++(); return tmp; }

			constexpr bool operator==(const iter_t& rhs) const noexcept { return p == rhs.p; }
			constexpr bool operator!=(const iter_t& rhs) const noexcept { return p != rhs.p; }
#if 0
			constexpr LC& operator*() noexcept { return *static_cast<LC *>(p); }
			constexpr LC* operator->() noexcept { return static_cast<LC *>(p); }

			constexpr LC& operator*() const noexcept { return *static_cast<LC *>(p); }
			constexpr LC* operator->() const noexcept { return static_cast<LC *>(p); }
#else
			constexpr LC* operator*() noexcept { return static_cast<LC *>(p); }
			constexpr LC* operator->() noexcept { return static_cast<LC *>(p); }

			constexpr LC* operator*() const noexcept { return static_cast<LC *>(p); }
			constexpr LC* operator->() const noexcept { return static_cast<LC *>(p); }
#endif
		};

		constexpr linked_list_t() noexcept : m_head(nullptr) {}

		constexpr iter_t begin() const noexcept { return iter_t(m_head); }
		constexpr iter_t end() const noexcept { return iter_t(nullptr); }

		constexpr void push_front(element_t *elem) noexcept
		{
			elem->m_next = m_head;
			elem->m_prev = nullptr;
			if (m_head)
				m_head->m_prev = elem;
			m_head = elem;
		}

		constexpr void push_back(element_t *elem) noexcept
		{
			element_t ** p(&m_head);
			element_t *  prev(nullptr);
			while (*p != nullptr)
			{
				prev = *p;
				p = &((*p)->m_next);
			}
			*p = elem;
			elem->m_prev = prev;
			elem->m_next = nullptr;
		}

		constexpr void remove(const element_t *elem) noexcept
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
		constexpr std::size_t size() const noexcept
		{
			std::size_t ret = 0;
			element_t *p = m_head;
			while (p != nullptr)
			{
				ret++;
				p = p->m_next;
			}
			return ret;
		}

		constexpr void clear() noexcept
		{
			element_t *p(m_head);
			while (p != nullptr)
			{
				element_t *n(p->m_next);
				p->m_next = nullptr;
				p->m_prev = nullptr;
				p = n;
			}
			m_head = nullptr;
		}

	private:
		element_t *m_head;
	};

} // namespace plib

#endif // PLISTS_H_
