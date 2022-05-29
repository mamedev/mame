// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file device.h
///

#ifndef NL_CORE_DEVICE_H_
#define NL_CORE_DEVICE_H_

#include "../nltypes.h"
#include "../plib/pstring.h"
#include "base_objects.h"
#include "logic_family.h"

namespace netlist
{
	// -----------------------------------------------------------------------------
	// core_device_t
	// -----------------------------------------------------------------------------
	// FIXME: belongs into detail namespace
	class core_device_t : public detail::netlist_object_t
	{
	public:
		core_device_t(netlist_state_t &owner, const pstring &name);
		core_device_t(core_device_t &owner, const pstring &name);

		PCOPYASSIGNMOVE(core_device_t, delete)

		virtual ~core_device_t() noexcept = default;

		void do_inc_active() noexcept
		{
			gsl_Expects(m_active_outputs >= 0);

			if (!m_activate.isnull() && m_hint_deactivate)
			{
				if (++m_active_outputs == 1)
				{
					if (m_stats)
						m_stats->m_stat_inc_active.inc();
					m_activate(true);//inc_active();
				}
			}
		}

		void do_dec_active() noexcept
		{
			gsl_Expects(m_active_outputs >= 1);

			if (!m_activate.isnull() && m_hint_deactivate)
				if (--m_active_outputs == 0)
				{
					m_activate(false); //dec_active();
				}
		}

		void set_hint_deactivate(bool v) noexcept { m_hint_deactivate = v; }
		bool get_hint_deactivate() const noexcept { return m_hint_deactivate; }
		// Has to be set in device reset
		void set_active_outputs(int n) noexcept { m_active_outputs = n; }

		// stats
		struct stats_t
		{
			// NL_KEEP_STATISTICS
			plib::pperftime_t<true>  m_stat_total_time;
			plib::pperfcount_t<true> m_stat_call_count;
			plib::pperfcount_t<true> m_stat_inc_active;
		};

		stats_t * stats() const noexcept { return m_stats.get(); }

		virtual void reset() { }

		void handler_noop()
		{
		}

	protected:
		using activate_delegate = plib::pmfp<void (bool)>;

		activate_delegate m_activate;

		log_type & log();

	public:
		virtual void timestep([[maybe_unused]] timestep_type ts_type,
			[[maybe_unused]] nl_fptype st) noexcept { }
		virtual void update_terminals() noexcept { }

		virtual void update_param() noexcept {}
		virtual bool is_dynamic() const noexcept { return false; }
		virtual bool is_timestep() const noexcept { return false; }

	private:
		bool            m_hint_deactivate;
		state_var_s32   m_active_outputs;
		device_arena::unique_ptr<stats_t> m_stats;
	};

	// -----------------------------------------------------------------------------
	// base_device_t
	// -----------------------------------------------------------------------------

	class base_device_t :   public core_device_t
	{
	public:
		base_device_t(netlist_state_t &owner, const pstring &name);
		base_device_t(base_device_t &owner, const pstring &name);

		PCOPYASSIGNMOVE(base_device_t, delete)

		~base_device_t() noexcept override = default;

		template <class O, class C, typename... Args>
		void create_and_register_subdevice(O& owner, const pstring &name, device_arena::unique_ptr<C> &dev, Args&&... args)
		{
			dev = state().make_pool_object<C>(owner, name, std::forward<Args>(args)...);
		}

		void register_subalias(const pstring &name, const detail::core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect(const pstring &t1, const pstring &t2);
		void connect(const detail::core_terminal_t &t1, const detail::core_terminal_t &t2);
	protected:

		//NETLIB_UPDATE_TERMINALSI() { }

	private:
	};

} // namespace netlist


#endif // NL_CORE_DEVICE_H_
