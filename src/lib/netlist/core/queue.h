// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file queue.h
///

#ifndef NL_CORE_QUEUE_H_
#define NL_CORE_QUEUE_H_

#include "queue.h"

#include "../nl_errstr.h"
#include "../nltypes.h"

#include "../plib/pstate.h"
#include "../plib/pstring.h"
#include "../plib/ptimed_queue.h"

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

namespace netlist::detail
{
	// Use timed_queue_heap to use stdc++ heap functions instead of linear processing.
	// This slows down processing by about 35% on a Kaby Lake.
	// template <class T, bool TS>
	// using timed_queue = plib::timed_queue_heap<T, TS>;

	template <class T, bool TS>
	using timed_queue = plib::timed_queue_linear<T, TS>;

	// -----------------------------------------------------------------------------
	// queue_t
	// -----------------------------------------------------------------------------

	// We don't need a thread-safe queue currently. Parallel processing of
	// solvers will update inputs after parallel processing.

	template <typename O, bool TS>
	class queue_base :
			public timed_queue<plib::pqentry_t<netlist_time_ext, O *>, false>,
			public plib::state_manager_t::callback_t
	{
	public:
		using entry_t = plib::pqentry_t<netlist_time_ext, O *>;
		using base_queue = timed_queue<entry_t, false>;
		using id_delegate = plib::pmfp<std::size_t (const O *)>;
		using obj_delegate = plib::pmfp<O * (std::size_t)>;

		explicit queue_base(std::size_t size, id_delegate get_id, obj_delegate get_obj)
		: timed_queue<plib::pqentry_t<netlist_time_ext, O *>, false>(size)
		, m_qsize(0)
		, m_times(size)
		, m_net_ids(size)
		, m_get_id(get_id)
		, m_obj_by_id(get_obj)
		{
		}

		~queue_base() noexcept override = default;

		queue_base(const queue_base &) = delete;
		queue_base(queue_base &&) = delete;
		queue_base &operator=(const queue_base &) = delete;
		queue_base &operator=(queue_base &&) = delete;

	protected:

		void register_state(plib::state_manager_t &manager, const pstring &module) override
		{
			manager.save_item(this, m_qsize, module + "." + "qsize");
			manager.save_item(this, &m_times[0], module + "." + "times", m_times.size());
			manager.save_item(this, &m_net_ids[0], module + "." + "names", m_net_ids.size());
		}
		void on_pre_save(plib::state_manager_t &manager) override
		{
			plib::unused_var(manager);
			m_qsize = this->size();
			for (std::size_t i = 0; i < m_qsize; i++ )
			{
				m_times[i] =  this->listptr()[i].exec_time().as_raw();
				m_net_ids[i] = m_get_id(this->listptr()[i].object());
			}
		}
		void on_post_load(plib::state_manager_t &manager) override
		{
			plib::unused_var(manager);
			this->clear();
			for (std::size_t i = 0; i < m_qsize; i++ )
			{
				O *n = m_obj_by_id(m_net_ids[i]);
				this->template push<false>(entry_t(netlist_time_ext::from_raw(m_times[i]),n));
			}
		}

	private:
		std::size_t m_qsize;
		std::vector<netlist_time_ext::internal_type> m_times;
		std::vector<std::size_t> m_net_ids;
		id_delegate m_get_id;
		obj_delegate m_obj_by_id;
	};

	using queue_t = queue_base<net_t, false>;

} // namespace netlist::detail


#endif // NL_CORE_QUEUE_H_
