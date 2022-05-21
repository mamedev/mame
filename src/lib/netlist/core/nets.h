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

#ifndef AVOID_NOOP_QUEUE_PUSHES
#error AVOID_NOOP_QUEUE_PUSHES not defined
#endif

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

			net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *railterminal = nullptr);

			PCOPYASSIGNMOVE(net_t, delete)

			virtual ~net_t() noexcept = default;

			virtual void reset() noexcept;

			// -----------------------------------------------------------------------------
			// Hot section
			//
			// Any changes below will impact performance.
			// -----------------------------------------------------------------------------

			void toggle_new_Q() noexcept { m_new_Q = (m_cur_Q ^ 1);   }

			void toggle_and_push_to_queue(const netlist_time &delay) noexcept
			{
				toggle_new_Q();
				push_to_queue(delay);
			}

			void push_to_queue(const netlist_time &delay) noexcept
			{
				if (!!is_queued())
					exec().qremove(this);

				m_next_scheduled_time = exec().time() + delay;
#if (AVOID_NOOP_QUEUE_PUSHES)
				m_in_queue = (m_list_active.empty() ? queue_status::DELAYED_DUE_TO_INACTIVE
					: (m_new_Q != m_cur_Q ? queue_status::QUEUED : queue_status::DELIVERED));
				if (m_in_queue == queue_status::QUEUED)
					exec().qpush(m_next_scheduled_time, this);
				else
					update_inputs();
#else
				m_in_queue = m_list_active.empty() ? queue_status::DELAYED_DUE_TO_INACTIVE : queue_status::QUEUED;
				if (m_in_queue == queue_status::QUEUED)
					exec().qpush(m_next_scheduled_time, this);
				else
					update_inputs();
#endif
			}
			constexpr bool is_queued() const noexcept { return m_in_queue == queue_status::QUEUED; }

			// -----------------------------------------------------------------------------
			// Very hot
			// -----------------------------------------------------------------------------

			template <bool KEEP_STATS>
			void update_devs() noexcept
			{
				gsl_Expects(this->is_rail_net());

				m_in_queue = queue_status::DELIVERED; // mark as taken ...

				const netlist_sig_t new_Q(m_new_Q);
				const netlist_sig_t cur_Q(m_cur_Q);
#if (!AVOID_NOOP_QUEUE_PUSHES)
				if (new_Q ^ cur_Q)
#endif
				{
					const auto mask = (new_Q << core_terminal_t::INP_LH_SHIFT)
						| (cur_Q << core_terminal_t::INP_HL_SHIFT);
					m_cur_Q = new_Q;

					if (!KEEP_STATS)
					{
						for (auto &p : m_list_active)
						{
							p.set_copied_input(new_Q);
							if ((p.terminal_state() & mask) != 0)
								p.run_delegate();
						}
					}
					else
					{
						for (auto & p : m_list_active)
						{
							p.set_copied_input(new_Q);
							auto *stats(p.device().stats());
							stats->m_stat_call_count.inc();
							if ((p.terminal_state() & mask))
							{
								auto g(stats->m_stat_total_time.guard());
								p.run_delegate();
							}
						}
					}
				}
			}

			constexpr const netlist_time_ext &next_scheduled_time() const noexcept { return m_next_scheduled_time; }
			void set_next_scheduled_time(netlist_time_ext ntime) noexcept { m_next_scheduled_time = ntime; }

			bool is_rail_net() const noexcept { return !(m_railterminal == nullptr); }
			core_terminal_t & railterminal() const noexcept { return *m_railterminal; }

			void add_to_active_list(core_terminal_t &term) noexcept
			{
				if (!m_list_active.empty())
				{
					term.set_copied_input(m_cur_Q);
					m_list_active.push_front(&term);
				}
				else
				{
					m_list_active.push_front(&term);
					railterminal().device().do_inc_active();
					if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
					{
#if (AVOID_NOOP_QUEUE_PUSHES)
						if (m_next_scheduled_time > exec().time()
							&& (m_cur_Q != m_new_Q))
#else
						if (m_next_scheduled_time > exec().time())
#endif
						{
							m_in_queue = queue_status::QUEUED;     // pending
							exec().qpush(m_next_scheduled_time, this);
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

			void remove_from_active_list(core_terminal_t &term) noexcept
			{
				gsl_Expects(!m_list_active.empty());
				m_list_active.remove(&term);
				if (m_list_active.empty())
				{
#if (AVOID_NOOP_QUEUE_PUSHES)
					if (!!is_queued())
					{
						exec().qremove(this);
						m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
					}
#endif
					railterminal().device().do_dec_active();
				}
			}

			// -----------------------------------------------------------------------------
			// setup stuff - cold
			// -----------------------------------------------------------------------------

			bool is_logic() const noexcept;
			bool is_analog() const noexcept;

			void rebuild_list();     // rebuild m_list after a load

			//std::vector<core_terminal_t *> &core_terms() noexcept { return state().m_core_terms[this]; }

			void update_inputs() noexcept
			{
#if NL_USE_COPY_INSTEAD_OF_REFERENCE
				for (auto & term : m_core_terms)
					term->m_Q = m_cur_Q;
#endif
				// nothing needs to be done if define not set
			}

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
			void set_Q_and_push(const netlist_sig_t &newQ, const netlist_time &delay) noexcept
			{
				gsl_Expects(delay >= netlist_time::zero());

				if (newQ != m_new_Q)
				{
					m_new_Q = newQ;
					push_to_queue(delay);
				}
			}

			// only used for logic nets
			void set_Q_time(const netlist_sig_t &newQ, const netlist_time_ext &at) noexcept
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

		private:
			state_var<netlist_sig_t>     m_new_Q;
			state_var<netlist_sig_t>     m_cur_Q;
			state_var<queue_status>      m_in_queue;
			plib::linkedlist_t<core_terminal_t> m_list_active;
			state_var<netlist_time_ext>  m_next_scheduled_time;

			core_terminal_t * m_railterminal;
			//std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

		};
	} // namespace detail

	class analog_net_t : public detail::net_t
	{
	public:

		analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal = nullptr);

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

		logic_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal = nullptr);

		using detail::net_t::Q;
		using detail::net_t::initial;
		using detail::net_t::set_Q_and_push;
		using detail::net_t::set_Q_time;
	};


} // namespace netlist


#endif // NL_CORE_NETS_H_
