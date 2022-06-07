// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nets.h
///

#ifndef NL_CORE_NETS_H_
#define NL_CORE_NETS_H_

#include "base_objects.h"
#include "core_device.h"
#include "exec.h"
#include "state_var.h"

#include "../nltypes.h"
#include "../plib/plists.h"
#include "../plib/pstring.h"

#include <algorithm>

namespace netlist
{
	namespace detail {

		// -----------------------------------------------------------------------------
		// net_t
		// -----------------------------------------------------------------------------

		class net_t : public netlist_object_t
		{
		public:

			enum class queue_status
			{
				DELAYED_DUE_TO_INACTIVE = 0,
				QUEUED,
				DELIVERED
			};

			net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *rail_terminal = nullptr);

			net_t(const net_t &) = delete;
			net_t &operator=(const net_t &) = delete;
			net_t(net_t &&) noexcept = delete;
			net_t &operator=(net_t &&) noexcept = delete;

			virtual ~net_t() noexcept = default;

			virtual void reset() noexcept;

			// -----------------------------------------------------------------------------
			// Hot section
			//
			// Any changes below will impact performance.
			// -----------------------------------------------------------------------------

			constexpr void toggle_new_Q() noexcept
			{
				m_new_Q = (m_cur_Q ^ 1);
			}

			void toggle_and_push_to_queue(const netlist_time &delay) noexcept
			{
				toggle_new_Q();
				push_to_queue(delay);
			}

			void push_to_queue(const netlist_time &delay) noexcept;

			constexpr bool is_queued() const noexcept
			{
				return m_in_queue == queue_status::QUEUED;
			}

			// -----------------------------------------------------------------------------
			// Very hot
			// -----------------------------------------------------------------------------

			template <bool KEEP_STATS>
			void update_devs() noexcept;

			constexpr const netlist_time_ext &next_scheduled_time() const noexcept { return m_next_scheduled_time; }
			void set_next_scheduled_time(const netlist_time_ext &next_time) noexcept { m_next_scheduled_time = next_time; }

			bool is_rail_net() const noexcept { return !(m_rail_terminal == nullptr); }
			core_terminal_t & rail_terminal() const noexcept { return *m_rail_terminal; }

			void add_to_active_list(core_terminal_t &term) noexcept;
			void remove_from_active_list(core_terminal_t &term) noexcept;

			// -----------------------------------------------------------------------------
			// setup stuff - cold
			// -----------------------------------------------------------------------------

			bool is_logic() const noexcept;
			bool is_analog() const noexcept;

			void rebuild_list();     // rebuild m_list after a load

			void update_inputs() noexcept
			{
				if constexpr (config::use_copy_instead_of_reference::value)
				{
					for (auto *term : core_terms_ref())
						term->set_copied_input(m_cur_Q);
				}
			}

			// -----------------------------------------------------------------------------
			// net management
			// -----------------------------------------------------------------------------

			const std::vector<detail::core_terminal_t *> core_terms_copy()
			{
				std::vector<detail::core_terminal_t *> ret(core_terms_ref().size());
				std::copy(core_terms_ref().begin(), core_terms_ref().end(), ret.begin());
				return ret;
			}

			void remove_terminal(detail::core_terminal_t &term);
			void remove_all_terminals();
			void add_terminal(detail::core_terminal_t &terminal);

			bool core_terms_empty() noexcept { return core_terms_ref().empty(); }
		protected:

			// only used for logic nets
			constexpr const netlist_sig_t &Q() const noexcept { return m_cur_Q; }

			// only used for logic nets
			void initial(netlist_sig_t val) noexcept
			{
				m_cur_Q = m_new_Q = val;
				update_inputs();
			}

			// only used for logic nets
			void set_Q_and_push(netlist_sig_t newQ, const netlist_time &delay) noexcept;

			// only used for logic nets
			void set_Q_time(netlist_sig_t newQ, const netlist_time_ext &at) noexcept;


		private:
#if NL_USE_INPLACE_CORE_TERMS
			const plib::linked_list_t<core_terminal_t, 1> &core_terms_ref() const noexcept
			{
				return m_core_terms;
			}
#else
			std::vector<detail::core_terminal_t *> &core_terms_ref()
			{
				return state().core_terms(*this);
			}
#endif
			state_var<netlist_sig_t>     m_new_Q;
			state_var<netlist_sig_t>     m_cur_Q;
			state_var<queue_status>      m_in_queue;
			// FIXME: this needs to be saved as well
			plib::linked_list_t<core_terminal_t, 0> m_list_active;
			state_var<netlist_time_ext>  m_next_scheduled_time;

			core_terminal_t * m_rail_terminal;
#if NL_USE_INPLACE_CORE_TERMS
			plib::linked_list_t<core_terminal_t, 1> m_core_terms;
#endif
		};

		inline void net_t::push_to_queue(const netlist_time &delay) noexcept
		{
			if (is_queued())
				exec().queue_remove(this);

			m_next_scheduled_time = exec().time() + delay;
			if constexpr (config::avoid_noop_queue_pushes::value)
				m_in_queue = (m_list_active.empty() ? queue_status::DELAYED_DUE_TO_INACTIVE
					: (m_new_Q != m_cur_Q ? queue_status::QUEUED : queue_status::DELIVERED));
			else
				m_in_queue = m_list_active.empty() ? queue_status::DELAYED_DUE_TO_INACTIVE : queue_status::QUEUED;

			if (m_in_queue == queue_status::QUEUED)
				exec().queue_push(m_next_scheduled_time, this);
			else
				update_inputs();
		}

		template <bool KEEP_STATS>
		void net_t::update_devs() noexcept
		{
			gsl_Expects(this->is_rail_net());

			m_in_queue = queue_status::DELIVERED; // mark as taken ...

			const netlist_sig_t new_Q(m_new_Q);
			const netlist_sig_t cur_Q(m_cur_Q);
			if (config::avoid_noop_queue_pushes::value || ((new_Q ^ cur_Q) != 0))
			{
				m_cur_Q = new_Q;
				const auto mask = (new_Q << core_terminal_t::INP_LH_SHIFT)
					| (cur_Q << core_terminal_t::INP_HL_SHIFT);

				if (!KEEP_STATS)
				{
					for (core_terminal_t * p: m_list_active)
					{
						p->set_copied_input(new_Q);
						if ((p->terminal_state() & mask) != 0)
							p->run_delegate();
					}
				}
				else
				{
					for (core_terminal_t * p : m_list_active)
					{
						p->set_copied_input(new_Q);
						auto *stats(p->device().stats());
						stats->m_stat_call_count.inc();
						if ((p->terminal_state() & mask))
						{
							auto g(stats->m_stat_total_time.guard());
							p->run_delegate();
						}
					}
				}
			}
		}

		inline void net_t::add_to_active_list(core_terminal_t &term) noexcept
		{
			if (!m_list_active.empty())
			{
				term.set_copied_input(m_cur_Q);
				m_list_active.push_front(&term);
			}
			else
			{
				m_list_active.push_front(&term);
				rail_terminal().device().do_inc_active();
				if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
				{
					// if we avoid queue pushes we must test if m_cur_Q and m_new_Q are equal
					if ((!config::avoid_noop_queue_pushes::value || (m_cur_Q != m_new_Q))
						&& (m_next_scheduled_time > exec().time()))
					{
						m_in_queue = queue_status::QUEUED;     // pending
						exec().queue_push(m_next_scheduled_time, this);
					}
					else
					{
						m_in_queue = queue_status::DELIVERED;
						m_cur_Q = m_new_Q;
					}
					update_inputs();
				}
				else
					term.set_copied_input(m_cur_Q);
			}
		}

		inline void net_t::remove_from_active_list(core_terminal_t &term) noexcept
		{
			gsl_Expects(!m_list_active.empty());
			m_list_active.remove(&term);
			if (m_list_active.empty())
			{
				if constexpr (true || config::avoid_noop_queue_pushes::value)
				{
					// All our connected outputs have signalled they no longer
					// will act on input. We thus remove any potentially queued
					// events and mark them.
					// FIXME: May cause regression test to fail - revisit in this case
					//
					// This code is definitively needed for the
					// AVOID_NOOP_QUEUE_PUSHES code path - therefore I left
					// the if statement in and enabled it for all code paths
					if (is_queued())
					{
						exec().queue_remove(this);
						m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
					}
				}
				rail_terminal().device().do_dec_active();
			}
		}

		// only used for logic nets
		inline void net_t::set_Q_and_push(netlist_sig_t newQ, const netlist_time &delay) noexcept
		{
			gsl_Expects(delay >= netlist_time::zero());

			if (newQ != m_new_Q)
			{
				m_new_Q = newQ;
				push_to_queue(delay);
			}
		}

		// only used for logic nets
		inline void net_t::set_Q_time(netlist_sig_t newQ, const netlist_time_ext &at) noexcept
		{
			gsl_Expects(at >= netlist_time_ext::zero());

			if (newQ != m_new_Q)
			{
				m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
				m_next_scheduled_time = at;
				m_cur_Q = m_new_Q = newQ;
				update_inputs();
			}
			else
			{
				m_cur_Q = newQ;
				update_inputs();
			}
		}

	} // namespace detail


	class analog_net_t : public detail::net_t
	{
	public:

		analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *rail_terminal = nullptr);

		void reset() noexcept override;

		const nl_fptype &Q_Analog() const noexcept { return m_cur_Analog; }
		void set_Q_Analog(nl_fptype v) noexcept { m_cur_Analog = v; }
		// used by solver code ...
		nl_fptype *Q_Analog_state_ptr() noexcept { return *m_cur_Analog; }

		//FIXME: needed by current solver code
		solver::matrix_solver_t *solver() const noexcept { return m_solver; }
		void set_solver(solver::matrix_solver_t *solver) noexcept { m_solver = solver; }

		friend constexpr bool operator==(const analog_net_t &lhs, const analog_net_t &rhs) noexcept
		{
			return &lhs == &rhs;
		}

	private:
		state_var<nl_fptype>     m_cur_Analog;
		solver::matrix_solver_t *m_solver;
	};

	class logic_net_t : public detail::net_t
	{
	public:

		logic_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *rail_terminal = nullptr);

		using detail::net_t::Q;
		using detail::net_t::initial;
		using detail::net_t::set_Q_and_push;
		using detail::net_t::set_Q_time;
	};


} // namespace netlist


#endif // NL_CORE_NETS_H_
