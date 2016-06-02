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
#include "plib/plists.h"

#include <atomic>


// ----------------------------------------------------------------------------------------
// timed queue
// ----------------------------------------------------------------------------------------

namespace netlist
{
	template <class Element, class Time>
	class timed_queue
	{
		P_PREVENT_COPYING(timed_queue)
	public:

		struct entry_t
		{
			Time m_exec_time;
			Element m_object;
		};

		timed_queue(unsigned list_size)
		: m_list(list_size)
		{
	#if HAS_OPENMP && USE_OPENMP
			m_lock = 0;
	#endif
			clear();
		}

		ATTR_HOT  std::size_t capacity() const { return m_list.size(); }
		ATTR_HOT  bool empty() const { return (m_end == &m_list[1]); }

		ATTR_HOT void push(const entry_t e) NOEXCEPT
		{
	#if HAS_OPENMP && USE_OPENMP
			/* Lock */
			while (m_lock.exchange(1)) { }
	#endif
			const Time t = e.m_exec_time;
			entry_t * i = m_end++;
			for (; t > (i - 1)->m_exec_time; --i)
			{
				*(i) = *(i-1);
				//i--;
				inc_stat(m_prof_sortmove);
			}
			*i = e;
			inc_stat(m_prof_call);
	#if HAS_OPENMP && USE_OPENMP
			m_lock = 0;
	#endif
			//nl_assert(m_end - m_list < Size);
		}

		ATTR_HOT  const entry_t & pop() NOEXCEPT       { return *(--m_end); }
		ATTR_HOT  const entry_t & top() const NOEXCEPT { return *(m_end-1); }

		ATTR_HOT  void remove(const Element &elem) NOEXCEPT
		{
			/* Lock */
	#if HAS_OPENMP && USE_OPENMP
			while (m_lock.exchange(1)) { }
	#endif
			for (entry_t * i = m_end - 1; i > &m_list[0]; i--)
			{
				if (i->m_object == elem)
				{
					m_end--;
					while (i < m_end)
					{
						*i = *(i+1);
						i++;
					}
	#if HAS_OPENMP && USE_OPENMP
					m_lock = 0;
	#endif
					return;
				}
			}
	#if HAS_OPENMP && USE_OPENMP
			m_lock = 0;
	#endif
		}

		ATTR_COLD void clear()
		{
			m_end = &m_list[0];
			/* put an empty element with maximum time into the queue.
			 * the insert algo above will run into this element and doesn't
			 * need a comparison with queue start.
			 */
			m_list[0] = { Time::from_raw(~0), Element(0) };
			m_end++;
		}

		// save state support & mame disasm

		ATTR_COLD  const entry_t *listptr() const { return &m_list[1]; }
		ATTR_HOT  std::size_t size() const { return m_end - &m_list[1]; }
		ATTR_HOT  const entry_t & operator[](const std::size_t index) const { return m_list[1+index]; }

	#if (NL_KEEP_STATISTICS)
		// profiling
		INT32   m_prof_sortmove;
		INT32   m_prof_call;
	#endif

	private:

	#if HAS_OPENMP && USE_OPENMP
		volatile std::atomic<int> m_lock;
	#endif
		entry_t * m_end;
		plib::array_t<entry_t> m_list;
	};

}

#endif /* NLLISTS_H_ */
