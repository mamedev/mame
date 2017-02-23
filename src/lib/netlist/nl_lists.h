// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nllists.h
 *
 */

#pragma once

#ifndef NLLISTS_H_
#define NLLISTS_H_

#include "nl_config.h"
#include "netlist_types.h"
#include "plib/plists.h"
#include "plib/pchrono.h"
#include "plib/ptypes.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>

// ----------------------------------------------------------------------------------------
// timed queue
// ----------------------------------------------------------------------------------------

#define USE_HEAP	(0)

namespace netlist
{
	//FIXME: move to an appropriate place
	template<bool enabled_ = true>
	class pspin_mutex
	{
	public:
		pspin_mutex() noexcept { }
		void lock() noexcept{ while (m_lock.test_and_set(std::memory_order_acquire)) { } }
		void unlock() noexcept { m_lock.clear(std::memory_order_release); }
	private:
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
	};

	template<>
	class pspin_mutex<false>
	{
	public:
		void lock() const noexcept { }
		void unlock() const noexcept { }
	};

#if !USE_HEAP
	template <class Element, class Time>
	class timed_queue : plib::nocopyassignmove
	{
	public:

		struct entry_t final
		{
			constexpr entry_t() noexcept : m_exec_time(), m_object(nullptr) { }
			constexpr entry_t(const Time &t, const Element &o) noexcept : m_exec_time(t), m_object(o) { }
			constexpr entry_t(const entry_t &e) noexcept : m_exec_time(e.m_exec_time), m_object(e.m_object) { }
			entry_t(entry_t &&e) noexcept { swap(e); }
			~entry_t() = default;

			entry_t& operator=(entry_t && other) noexcept
			{
				swap(other);
				return *this;
			}

			entry_t& operator=(const entry_t &other) noexcept
			{
				entry_t t(other);
				swap(t);
				return *this;
			}

			void swap(entry_t &other) noexcept
			{
				std::swap(m_exec_time, other.m_exec_time);
				std::swap(m_object, other.m_object);
			}

			Time m_exec_time;
			Element m_object;
		};

		timed_queue(const std::size_t list_size)
		: m_list(list_size)
		{
			clear();
		}

		constexpr std::size_t capacity() const noexcept { return m_list.capacity() - 1; }
		constexpr bool empty() const noexcept { return (m_end == &m_list[1]); }

		void push(entry_t &&e) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			entry_t * i = m_end;
			for (; e.m_exec_time > (i - 1)->m_exec_time; --i)
			{
				*(i) = *(i-1);
				m_prof_sortmove.inc();
			}
			*i = std::move(e);
			++m_end;
			m_prof_call.inc();
		}

		entry_t pop() noexcept              { return *(--m_end); }
		const entry_t &top() const noexcept { return *(m_end-1); }

		void remove(const Element &elem) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			for (entry_t * i = m_end - 1; i > &m_list[0]; i--)
			{
				if (i->m_object == elem)
				{
					m_end--;
					for (;i < m_end; i++)
						*i = std::move(*(i+1));
					return;
				}
			}
		}

		void retime(const Element &elem, const Time t) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			for (entry_t * i = m_end - 1; i > &m_list[0]; i--)
			{
				if (i->m_object == elem)
				{
					i->m_exec_time = t;
					while ((i-1)->m_exec_time < i->m_exec_time)
					{
						std::swap(*(i-1), *i);
						i--;
					}
					while (i < m_end && (i+1)->m_exec_time > i->m_exec_time)
					{
						std::swap(*(i+1), *i);
						i++;
					}
					return;
				}
			}
		}

		void clear()
		{
			tqlock lck(m_lock);
			m_end = &m_list[0];
			/* put an empty element with maximum time into the queue.
			 * the insert algo above will run into this element and doesn't
			 * need a comparison with queue start.
			 */
			m_list[0] = { Time::never(), Element(0) };
			m_end++;
		}

		// save state support & mame disasm

		constexpr const entry_t *listptr() const { return &m_list[1]; }
		constexpr std::size_t size() const noexcept { return static_cast<std::size_t>(m_end - &m_list[1]); }
		constexpr const entry_t & operator[](const std::size_t index) const { return m_list[ 1 + index]; }
	private:
	#if HAS_OPENMP && USE_OPENMP
		using tqmutex = pspin_mutex<true>;
	#else
		using tqmutex = pspin_mutex<false>;
	#endif
		using tqlock = std::lock_guard<tqmutex>;

		tqmutex m_lock;
		entry_t * m_end;
		std::vector<entry_t> m_list;

	public:
		// profiling
		nperfcount_t m_prof_sortmove;
		nperfcount_t m_prof_call;
	};
#else
	template <class Element, class Time>
	class timed_queue : plib::nocopyassignmove
	{
	public:

		struct entry_t final
		{
			constexpr entry_t() noexcept : m_exec_time(), m_object(nullptr) { }
			constexpr entry_t(const Time &t, const Element &o) noexcept : m_exec_time(t), m_object(o) { }
			constexpr entry_t(const entry_t &e) noexcept : m_exec_time(e.m_exec_time), m_object(e.m_object) { }
			entry_t(entry_t &&e) noexcept { swap(e); }
			~entry_t() = default;

			entry_t& operator=(entry_t && other) noexcept
			{
				swap(other);
				return *this;
			}

			entry_t& operator=(const entry_t &other) noexcept
			{
				entry_t t(other);
				swap(t);
				return *this;
			}

			void swap(entry_t &other) noexcept
			{
				std::swap(m_exec_time, other.m_exec_time);
				std::swap(m_object, other.m_object);
			}

			Time m_exec_time;
			Element m_object;
		};

		struct compare
		{
			bool operator()(const entry_t &a, const entry_t &b) { return a.m_exec_time > b.m_exec_time; }
		};

		timed_queue(const std::size_t list_size)
		: m_list(list_size)
		{
			clear();
		}

		constexpr std::size_t capacity() const noexcept { return m_list.capacity(); }
		constexpr bool empty() const noexcept { return &m_list[0] == m_end; }

		void push(entry_t &&e) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			*m_end++ = e;
			std::push_heap(&m_list[0], m_end, compare());
			m_prof_call.inc();
		}

		entry_t pop() noexcept
		{
			entry_t t(m_list[0]);
			std::pop_heap(&m_list[0], m_end, compare());
			m_end--;
			return t;
		}

		const entry_t &top() const noexcept { return m_list[0]; }

		void remove(const Element &elem) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			for (entry_t * i = m_end - 1; i >= &m_list[0]; i--)
			{
				if (i->m_object == elem)
				{
					m_end--;
					for (;i < m_end; i++)
						*i = std::move(*(i+1));
					std::make_heap(&m_list[0], m_end, compare());
					return;
				}
			}
		}

		void retime(const Element &elem, const Time t) noexcept
		{
			/* Lock */
			tqlock lck(m_lock);
			for (entry_t * i = m_end - 1; i >= &m_list[0]; i--)
			{
				if (i->m_object == elem)
				{
					i->m_exec_time = t;
					std::make_heap(&m_list[0], m_end, compare());
					return;
				}
			}
		}

		void clear()
		{
			tqlock lck(m_lock);
			m_list.clear();
			m_end = &m_list[0];
		}

		// save state support & mame disasm

		constexpr const entry_t *listptr() const { return &m_list[0]; }
		constexpr std::size_t size() const noexcept { return m_list.size(); }
		constexpr const entry_t & operator[](const std::size_t index) const { return m_list[ 0 + index]; }
	private:
	#if HAS_OPENMP && USE_OPENMP
		using tqmutex = pspin_mutex<true>;
	#else
		using tqmutex = pspin_mutex<false>;
	#endif
		using tqlock = std::lock_guard<tqmutex>;

		tqmutex m_lock;
		std::vector<entry_t> m_list;
		entry_t *m_end;

	public:
		// profiling
		nperfcount_t m_prof_sortmove;
		nperfcount_t m_prof_call;
	};
#endif
}

#endif /* NLLISTS_H_ */
