// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PTIMED_QUEUE_H_
#define PTIMED_QUEUE_H_

///
/// \file ptimed_queue.h
///

#include "palloc.h" // FIXME: for aligned_vector
#include "pchrono.h"
#include "pmulti_threading.h"
#include "ptypes.h"

#include <algorithm>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace plib {

	// ----------------------------------------------------------------------------------------
	// timed queue
	// ----------------------------------------------------------------------------------------

	template <typename Time, typename Element>
	struct queue_entry_t final
	{
		constexpr queue_entry_t() noexcept : m_exec_time(), m_object(nullptr) { }
		constexpr queue_entry_t(const Time &t, const Element &o) noexcept : m_exec_time(t), m_object(o) { }

		queue_entry_t(const queue_entry_t &) = default;
		queue_entry_t &operator=(const queue_entry_t &) = default;
		queue_entry_t(queue_entry_t &&) noexcept = default;
		queue_entry_t &operator=(queue_entry_t &&) noexcept = default;

		~queue_entry_t() = default;

		constexpr bool operator ==(const queue_entry_t &rhs) const noexcept
		{
			return m_object == rhs.m_object;
		}

		constexpr bool operator ==(const Element &rhs) const noexcept
		{
			return m_object == rhs;
		}

		constexpr bool operator <=(const queue_entry_t &rhs) const noexcept
		{
			return (m_exec_time <= rhs.m_exec_time);
		}

		constexpr bool operator <(const queue_entry_t &rhs) const noexcept
		{
			return (m_exec_time < rhs.m_exec_time);
		}

		static constexpr queue_entry_t never() noexcept { return queue_entry_t(Time::never(), nullptr); }

		constexpr const Time &exec_time() const noexcept { return m_exec_time; }
		constexpr const Element &object() const noexcept { return m_object; }
	private:
		Time m_exec_time;
		Element m_object;
	};

	// Use TS = true for a threadsafe queue
	template <class A, class T, bool TS>
	class timed_queue_linear
	{
	public:

		explicit timed_queue_linear(A &arena, const std::size_t list_size)
		: m_list(arena, list_size)
		{
			clear();
		}
		~timed_queue_linear() = default;

		PCOPYASSIGNMOVE(timed_queue_linear, delete)

		std::size_t capacity() const noexcept { return m_list.capacity() - 1; }
		bool empty() const noexcept { return (m_end == &m_list[1]); }

		template<bool KEEPSTAT, typename... Args>
		void emplace(Args&&... args) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			T * i(m_end++);
			*i = T(std::forward<Args>(args)...);

			if (!KEEPSTAT)
			{
				for (; *(i-1) < *i; --i)
				{
					std::swap(*(i-1), *(i));
				}
			}
			else
			{
				for (; *(i-1) < *i; --i)
				{
					std::swap(*(i-1), *(i));
					m_prof_sortmove.inc();
				}
				m_prof_call.inc();
			}
		}

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

		void pop() noexcept       { --m_end; }

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
			//printf("Element not found in delete %s\n", elem->name().c_str());
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

		// save state support & mame disassembler

		const T *list_pointer() const noexcept { return &m_list[1]; }
		std::size_t size() const noexcept { return narrow_cast<std::size_t>(m_end - &m_list[1]); }
		const T & operator[](std::size_t index) const noexcept { return m_list[ 1 + index]; }
	private:
		using mutex_type       = pspin_mutex<TS>;
		using lock_guard_type  = std::lock_guard<mutex_type>;

		mutex_type               m_lock;
		T *                      m_end;
		plib::arena_vector<A, T> m_list;

	public:
		// profiling
		// FIXME: Make those private
		pperfcount_t<true> m_prof_sortmove; // NOLINT
		pperfcount_t<true> m_prof_call; // NOLINT
		pperfcount_t<true> m_prof_remove; // NOLINT
	};

	template <class A, class T, bool TS>
	class timed_queue_heap
	{
	public:

		struct compare
		{
			constexpr bool operator()(const T &a, const T &b) const noexcept { return b <= a; }
		};

		explicit timed_queue_heap(A &arena, const std::size_t list_size)
		: m_list(arena, list_size)
		{
			clear();
		}
		~timed_queue_heap() = default;

		PCOPYASSIGNMOVE(timed_queue_heap, delete)

		std::size_t capacity() const noexcept { return m_list.capacity(); }
		bool empty() const noexcept { return &m_list[0] == m_end; }

		template<bool KEEPSTAT, typename... Args>
		void emplace(Args&&... args) noexcept
		{
			// Lock
			lock_guard_type lck(m_lock);
			*m_end++ = T(std::forward<Args>(args)...);
			std::push_heap(&m_list[0], m_end, compare());
			if (KEEPSTAT)
				m_prof_call.inc();
		}

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

		void pop() noexcept
		{
			std::pop_heap(&m_list[0], m_end, compare());
			m_end--;
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
					*i = *m_end;
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

		// save state support & mame disassembler

		constexpr const T *list_pointer() const { return &m_list[0]; }
		constexpr std::size_t size() const noexcept { return m_list.size(); }
		constexpr const T & operator[](const std::size_t index) const { return m_list[ 0 + index]; }
	private:
		using mutex_type = pspin_mutex<TS>;
		using lock_guard_type = std::lock_guard<mutex_type>;

		mutex_type               m_lock;
		T *                      m_end;
		plib::arena_vector<A, T> m_list;

	public:
		// profiling
		pperfcount_t<true> m_prof_sortmove; // NOLINT
		pperfcount_t<true> m_prof_call; // NOLINT
		pperfcount_t<true> m_prof_remove; // NOLINT
	};

} // namespace plib

#endif // PTIMED_QUEUE_H_
