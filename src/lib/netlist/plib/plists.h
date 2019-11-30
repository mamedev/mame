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
#include <atomic>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

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

		COPYASSIGNMOVE(uninitialised_array_t, delete)
		~uninitialised_array_t() noexcept
		{
			if (m_initialized>=N)
				for (size_type i=0; i<N; ++i)
					(*this)[i].~C();
		}

		size_t size() const noexcept { return N; }

		reference operator[](size_type index) noexcept
		{
			return *reinterpret_cast<pointer>(&m_buf[index]);
		}

		const_reference operator[](size_type index) const noexcept
		{
			return *reinterpret_cast<const_pointer>(&m_buf[index]);
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
			constexpr iter_t(iter_t &rhs) noexcept : p(rhs.p) { }
			iter_t(iter_t &&rhs) noexcept { std::swap(*this, rhs);  }

			iter_t& operator=(const iter_t &rhs) noexcept
			{
				if (this != &rhs)
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
			C14CONSTEXPR LC& operator*() noexcept { return *p; }
			C14CONSTEXPR LC* operator->() noexcept { return p; }

			C14CONSTEXPR LC& operator*() const noexcept { return *p; }
			C14CONSTEXPR LC* operator->() const noexcept { return p; }
		};

		constexpr linkedlist_t() : m_head(nullptr) {}

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

		LC *front() const noexcept { return m_head; }
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

	// ----------------------------------------------------------------------------------------
	// FIXME: Move elsewhere
	// ----------------------------------------------------------------------------------------

	template<bool enabled_ = true>
	class pspin_mutex
	{
	public:
		pspin_mutex() noexcept = default;
		void lock() noexcept{ while (m_lock.test_and_set(std::memory_order_acquire)) { } }
		void unlock() noexcept { m_lock.clear(std::memory_order_release); }
	private:
		PALIGNAS_CACHELINE()
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
	};

	template<>
	class pspin_mutex<false>
	{
	public:
		void lock() const noexcept { }
		void unlock() const noexcept { }
	};

	// ----------------------------------------------------------------------------------------
	// timed queue
	// ----------------------------------------------------------------------------------------

	template <class Element, class Time>
	struct pqentry_t final
	{
		constexpr pqentry_t() noexcept : m_exec_time(), m_object(nullptr) { }
		constexpr pqentry_t(const Time t, const Element o) noexcept : m_exec_time(t), m_object(o) { }

		inline bool operator ==(const pqentry_t &rhs) const noexcept
		{
			return m_object == rhs.m_object;
		}

		inline bool operator ==(const Element &rhs) const noexcept
		{
			return m_object == rhs;
		}

		inline bool operator <=(const pqentry_t &rhs) const noexcept
		{
			return (m_exec_time <= rhs.m_exec_time);
		}

		inline bool operator <(const pqentry_t &rhs) const noexcept
		{
			return (m_exec_time < rhs.m_exec_time);
		}

		inline static constexpr pqentry_t never() noexcept { return pqentry_t(Time::never(), nullptr); }

		Time exec_time() const noexcept { return m_exec_time; }
		Element object() const noexcept { return m_object; }
	private:
		Time m_exec_time;
		Element m_object;
	};

	// Use TS = true for a threadsafe queue
	template <class T, bool TS>
	class timed_queue_linear : nocopyassignmove
	{
	public:

		explicit timed_queue_linear(const std::size_t list_size)
		: m_list(list_size)
		{
			clear();
		}

		std::size_t capacity() const noexcept { return m_list.capacity() - 1; }
		bool empty() const noexcept { return (m_end == &m_list[1]); }

		template<bool KEEPSTAT>
		void push(T && e) noexcept
		{
#if 0
			// Lock
			lock_guard_type lck(m_lock);
			T * i(m_end-1);
			for (; *i < e; --i)
			{
				*(i+1) = *(i);
				if (KEEPSTAT)
					m_prof_sortmove.inc();
			}
			*(i+1) = std::move(e);
			++m_end;
#else
			// Lock
			lock_guard_type lck(m_lock);
			T * i(m_end++);
			*i = std::move(e);
			for (; *(i-1) < *i; --i)
			{
				std::swap(*(i-1), *(i));
				if (KEEPSTAT)
					m_prof_sortmove.inc();
			}
#endif
			if (KEEPSTAT)
				m_prof_call.inc();
		}

		T pop() noexcept       { return *(--m_end); }
		const T &top() const noexcept { return *(m_end-1); }

		template <bool KEEPSTAT, class R>
		void remove(const R &elem) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			if (KEEPSTAT)
				m_prof_remove.inc();
			for (T * i = m_end - 1; i > &m_list[0]; --i)
			{
				// == operator ignores time!
				if (*i == elem)
				{
					std::copy(i+1, m_end--, i);
					return;
				}
			}
		}

		template <bool KEEPSTAT, class R>
		void retime(R && elem) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			if (KEEPSTAT)
				m_prof_retime.inc();

			for (R * i = m_end - 1; i > &m_list[0]; --i)
			{
				if (*i == elem) // partial equal!
				{
					*i = std::forward<R>(elem);
					while (*(i-1) < *i)
					{
						std::swap(*(i-1), *i);
						--i;
					}
					while (i < m_end && *i < *(i+1))
					{
						std::swap(*(i+1), *i);
						++i;
					}
					return;
				}
			}
		}

		void clear() noexcept
		{
			lock_guard_type lck(m_lock);
			m_end = &m_list[0];
			// put an empty element with maximum time into the queue.
			// the insert algo above will run into this element and doesn't
			// need a comparison with queue start.
			//
			m_list[0] = T::never();
			m_end++;
		}

		// save state support & mame disasm

		const T *listptr() const noexcept { return &m_list[1]; }
		std::size_t size() const noexcept { return static_cast<std::size_t>(m_end - &m_list[1]); }
		const T & operator[](std::size_t index) const noexcept { return m_list[ 1 + index]; }
	private:
		using mutex_type = pspin_mutex<TS>;
		using lock_guard_type = std::lock_guard<mutex_type>;

		mutex_type              m_lock;
		PALIGNAS_CACHELINE()
		T *                     m_end;
		aligned_vector<T> m_list;

	public:
		// profiling
		pperfcount_t<true> m_prof_sortmove;
		pperfcount_t<true> m_prof_call;
		pperfcount_t<true> m_prof_remove;
		pperfcount_t<true> m_prof_retime;
	};

	template <class T, bool TS>
	class timed_queue_heap : nocopyassignmove
	{
	public:

		struct compare
		{
			constexpr bool operator()(const T &a, const T &b) const { return b <= a; }
		};

		explicit timed_queue_heap(const std::size_t list_size)
		: m_list(list_size)
		{
			clear();
		}

		std::size_t capacity() const noexcept { return m_list.capacity(); }
		bool empty() const noexcept { return &m_list[0] == m_end; }

		template <bool KEEPSTAT>
		void push(T &&e) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			*m_end++ = e;
			std::push_heap(&m_list[0], m_end, compare());
			if (KEEPSTAT)
				m_prof_call.inc();
		}

		T pop() noexcept
		{
			T ret(m_list[0]);
			std::pop_heap(&m_list[0], m_end, compare());
			m_end--;
			return ret;
		}

		const T &top() const noexcept { return m_list[0]; }

		template <bool KEEPSTAT, class R>
		void remove(const R &elem) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			if (KEEPSTAT)
				m_prof_remove.inc();
			for (T * i = m_end - 1; i >= &m_list[0]; i--)
			{
				if (*i == elem)
				{
					m_end--;
					for (;i < m_end; i++)
						*i = std::move(*(i+1));
					std::make_heap(&m_list[0], m_end, compare());
					return;
				}
			}
		}

		template <bool KEEPSTAT>
		void retime(const T &elem) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			if (KEEPSTAT)
				m_prof_retime.inc();
			for (T * i = m_end - 1; i >= &m_list[0]; i--)
			{
				if (*i == elem) // partial equal!
				{
					*i = elem;
					std::make_heap(&m_list[0], m_end, compare());
					return;
				}
			}
		}

		void clear()
		{
			lock_guard_type lck(m_lock);
			m_list.clear();
			m_end = &m_list[0];
		}

		// save state support & mame disasm

		constexpr const T *listptr() const { return &m_list[0]; }
		constexpr std::size_t size() const noexcept { return m_list.size(); }
		constexpr const T & operator[](const std::size_t index) const { return m_list[ 0 + index]; }
	private:
		using mutex_type = pspin_mutex<TS>;
		using lock_guard_type = std::lock_guard<mutex_type>;

		mutex_type m_lock;
		std::vector<T> m_list;
		T *m_end;

	public:
		// profiling
		pperfcount_t<true> m_prof_sortmove;
		pperfcount_t<true> m_prof_call;
		pperfcount_t<true> m_prof_remove;
		pperfcount_t<true> m_prof_retime;
	};

} // namespace plib

#endif // PLISTS_H_
