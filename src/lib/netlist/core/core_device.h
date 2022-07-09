// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file device.h
///

#ifndef NL_CORE_DEVICE_H_
#define NL_CORE_DEVICE_H_

#include "../nltypes.h"
#include "base_objects.h"

#include "../plib/pstring.h"

namespace netlist
{
	// -------------------------------------------------------------------------
	// core_device_t construction parameters
	// -------------------------------------------------------------------------

	struct core_device_data_t
	{
		friend class core_device_t;
		friend class base_device_t;
		friend class analog::NETLIB_NAME(two_terminal);
		friend class logic_family_std_proxy_t;

		template <unsigned m_NI, unsigned m_NO>
		friend class devices::factory_truth_table_t;

		template <class C, typename... Args>
		friend class factory::device_element_t;
		friend class factory::library_element_t;

		template <typename CX>
		friend struct sub_device_wrapper;

		friend class solver::matrix_solver_t;

	private:
		core_device_data_t(netlist_state_t &o, const pstring &n)
		: owner(o)
		, name(n)
		{
		}
		netlist_state_t &owner;
		const pstring &  name;
	};

	// The type use to pass data on
	using core_device_param_t = const core_device_data_t &;

	// -------------------------------------------------------------------------
	// core_device_t
	// -------------------------------------------------------------------------
	// FIXME: belongs into detail namespace

	class core_device_t : public detail::netlist_object_t
	{
	public:
		using constructor_data_t = core_device_data_t;
		using constructor_param_t = core_device_param_t;

		core_device_t(core_device_param_t data);

		core_device_t(const core_device_t &) = delete;
		core_device_t &operator=(const core_device_t &) = delete;
		core_device_t(core_device_t &&) noexcept = delete;
		core_device_t &operator=(core_device_t &&) noexcept = delete;

		virtual ~core_device_t() noexcept = default;

		void do_inc_active() noexcept;

		void do_dec_active() noexcept;

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

		stats_t *stats() const noexcept { return m_stats.get(); }

		virtual void reset() {}

		void handler_noop() {}

	protected:
		using activate_delegate = plib::pmfp<void(bool)>;

		activate_delegate m_activate;

		log_type &log();

	public:
		virtual void time_step([[maybe_unused]] time_step_type ts_type,
			[[maybe_unused]] nl_fptype                         st) noexcept
		{
		}
		virtual void update_terminals() noexcept {}

		virtual void update_param() noexcept {}
		virtual bool is_dynamic() const noexcept { return false; }
		virtual bool is_time_step() const noexcept { return false; }

	private:
		// FIXME: should this be a state_var?
		bool                              m_hint_deactivate;
		state_var_s32                     m_active_outputs;
		device_arena::unique_ptr<stats_t> m_stats;
	};

	inline void core_device_t::do_inc_active() noexcept
	{
		gsl_Expects(m_active_outputs >= 0);

		if (!m_activate.isnull() && m_hint_deactivate)
		{
			if (++m_active_outputs == 1)
			{
				if (m_stats)
					m_stats->m_stat_inc_active.inc();
				m_activate(true); // inc_active();
			}
		}
	}

	inline void core_device_t::do_dec_active() noexcept
	{
		gsl_Expects(m_active_outputs >= 1);

		if (!m_activate.isnull() && m_hint_deactivate)
			if (--m_active_outputs == 0)
			{
				m_activate(false); // dec_active();
			}
	}

	// -------------------------------------------------------------------------
	// core_device_t construction parameters
	// -------------------------------------------------------------------------

	using base_device_data_t = core_device_data_t;
	// The type use to pass data on
	using base_device_param_t = const base_device_data_t &;

	// -------------------------------------------------------------------------
	// base_device_t
	// -------------------------------------------------------------------------

	class base_device_t : public core_device_t
	{
	public:
		using constructor_data_t = base_device_data_t;
		using constructor_param_t = base_device_param_t;

		base_device_t(base_device_param_t data);

		PCOPYASSIGNMOVE(base_device_t, delete)

		~base_device_t() noexcept override = default;

		template <class O, class C, typename... Args>
		void create_and_register_sub_device(O &owner, const pstring &name,
			device_arena::unique_ptr<C> &dev, Args &&...args)
		{
			// dev = state().make_pool_object<C>(owner, name,
			// std::forward<Args>(args)...);
			using dev_constructor_data_t = typename C::constructor_data_t;
			dev = state().make_pool_object<C>(
				dev_constructor_data_t{state(), owner.name() + "." + name},
				std::forward<Args>(args)...);
			state().register_device(dev->name(),
				device_arena::owned_ptr<core_device_t>(dev.get(), false));
		}

		void register_sub_alias(const pstring &name,
			const detail::core_terminal_t &    term);
		void register_sub_alias(const pstring &name, const pstring &aliased);

		void connect(const pstring &t1, const pstring &t2);
		void connect(const detail::core_terminal_t &t1,
			const detail::core_terminal_t &         t2);

	protected:
		// NETLIB_UPDATE_TERMINALSI() { }

	private:
	};

} // namespace netlist

#endif // NL_CORE_DEVICE_H_
