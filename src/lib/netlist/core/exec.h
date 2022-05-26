// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file exec.h
///

#ifndef NL_CORE_EXEC_H_
#define NL_CORE_EXEC_H_

#include "base_objects.h"
#include "state_var.h"

#include "../nltypes.h"
#include "../plib/plists.h"
#include "../plib/pstring.h"

namespace netlist
{
	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------

	class netlist_t // NOLINT(clang-analyzer-optin.performance.Padding)
	{
	public:

		explicit netlist_t(netlist_state_t &state, const pstring &aname);

		PCOPYASSIGNMOVE(netlist_t, delete)

		virtual ~netlist_t() noexcept = default;

		// run functions

		const netlist_time_ext &time() const noexcept { return m_time; }

		void process_queue(netlist_time_ext delta) noexcept;
		void abort_current_queue_slice() noexcept
		{
			qremove(nullptr);
			qpush(m_time, nullptr);
		}

		const detail::queue_t &queue() const noexcept { return m_queue; }

		template<typename... Args>
		void qpush(Args&&...args) noexcept
		{
			if (config::use_queue_stats::value && m_use_stats)
				m_queue.emplace<false>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
			else
				m_queue.emplace<true>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
		}

		template <class R>
		void qremove(const R &elem) noexcept
		{
			if (config::use_queue_stats::value && m_use_stats)
				m_queue.remove<true>(elem);
			else
				m_queue.remove<false>(elem);
		}

		// Control functions

		void stop();
		void reset();

		// only used by nltool to create static c-code
		devices::nld_solver *solver() const noexcept { return m_solver; }

		// FIXME: force late type resolution
		template <typename X = devices::nld_solver>
		nl_fptype gmin([[maybe_unused]] X *solver = nullptr) const noexcept
		{
			return static_cast<X *>(m_solver)->gmin();
		}

		netlist_state_t &nl_state() noexcept { return m_state; }
		const netlist_state_t &nl_state() const noexcept { return m_state; }

		log_type & log() noexcept { return m_state.log(); }
		const log_type &log() const noexcept { return m_state.log(); }

		void print_stats() const;

		constexpr bool stats_enabled() const noexcept { return m_use_stats; }
		void enable_stats(bool val) noexcept { m_use_stats = val; }

	private:

		template <bool KEEP_STATS>
		void process_queue_stats(netlist_time_ext delta) noexcept;

		netlist_state_t &                   m_state;
		devices::nld_solver *               m_solver;

		// mostly rw
		//PALIGNAS(16)
		netlist_time_ext                    m_time;
		devices::nld_mainclock *            m_main_clock;

		//PALIGNAS_CACHELINE()
		//PALIGNAS(16)
		bool                                m_use_stats;
		detail::queue_t                     m_queue;
		// performance
		plib::pperftime_t<true>             m_stat_mainloop;
		plib::pperfcount_t<true>            m_perf_out_processed;
	};

} // namespace netlist


#endif // NL_CORE_EXEC_H_
