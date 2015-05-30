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


// ----------------------------------------------------------------------------------------
// timed queue
// ----------------------------------------------------------------------------------------

template <class _Element, class _Time, int _Size>
class netlist_timed_queue
{
	NETLIST_PREVENT_COPYING(netlist_timed_queue)
public:

	class entry_t
	{
	public:
		ATTR_HOT /* inline */ entry_t()
		:  m_exec_time(), m_object() {}
		ATTR_HOT /* inline */ entry_t(const _Time &atime, const _Element &elem) : m_exec_time(atime), m_object(elem)  {}
		ATTR_HOT /* inline */ const _Time &exec_time() const { return m_exec_time; }
		ATTR_HOT /* inline */ const _Element &object() const { return m_object; }

		ATTR_HOT /* inline */ entry_t &operator=(const entry_t &right) {
			m_exec_time = right.m_exec_time;
			m_object = right.m_object;
			return *this;
		}

	private:
		_Time m_exec_time;
		_Element m_object;
	};

	netlist_timed_queue()
	{
		clear();
	}

	ATTR_HOT /* inline */ int capacity() const { return _Size; }
	ATTR_HOT /* inline */ bool is_empty() const { return (m_end == &m_list[1]); }
	ATTR_HOT /* inline */ bool is_not_empty() const { return (m_end > &m_list[1]); }

	ATTR_HOT void push(const entry_t &e)
	{
		const _Time t = e.exec_time();
		entry_t * i = m_end++;
		while (t > (i - 1)->exec_time())
		{
			*(i) = *(i-1);
			i--;
			inc_stat(m_prof_sortmove);
		}
		*i = e;
		inc_stat(m_prof_call);
		//nl_assert(m_end - m_list < _Size);
	}

	ATTR_HOT /* inline */ const entry_t *pop()
	{
		return --m_end;
	}

	ATTR_HOT /* inline */ const entry_t *peek() const
	{
		return (m_end-1);
	}

	ATTR_HOT /* inline */ void remove(const _Element &elem)
	{
		entry_t * i = m_end - 1;
		while (i > &m_list[0])
		{
			if (i->object() == elem)
			{
				m_end--;
				while (i < m_end)
				{
					*i = *(i+1);
					i++;
				}
				return;
			}
			i--;
		}
	}

	ATTR_COLD void clear()
	{
		m_end = &m_list[0];
		/* put an empty element with maximum time into the queue.
		 * the insert algo above will run into this element and doesn't
		 * need a comparison with queue start.
		 */
		m_list[0] = entry_t(_Time::from_raw(~0), _Element(0));
		m_end++;
	}

	// save state support & mame disasm

	ATTR_COLD /* inline */ const entry_t *listptr() const { return &m_list[0]; }
	ATTR_HOT /* inline */ int count() const { return m_end - m_list; }
	ATTR_HOT /* inline */ const entry_t & operator[](const int & index) const { return m_list[index]; }

#if (NL_KEEP_STATISTICS)
	// profiling
	INT32   m_prof_sortmove;
	INT32   m_prof_call;
#endif

private:

	entry_t * m_end;
	entry_t m_list[_Size];

};

#endif /* NLLISTS_H_ */
