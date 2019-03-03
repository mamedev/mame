// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_proxy.cpp
 *
 */

#include "nlid_proxy.h"
#include "netlist/solver/nld_solver.h"
//#include "plib/pstream.h"
//#include "plib/pfmtlog.h"
//#include "nld_log.h"

namespace netlist
{
	namespace devices
	{

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	nld_base_proxy::nld_base_proxy(netlist_state_t &anetlist, const pstring &name,
			logic_t *inout_proxied, detail::core_terminal_t *proxy_inout)
			: device_t(anetlist, name)
	{
		m_logic_family = inout_proxied->logic_family();
		m_term_proxied = inout_proxied;
		m_proxy_term = proxy_inout;
	}

	// ----------------------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_a_to_d_proxy::nld_base_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
			logic_input_t *in_proxied, detail::core_terminal_t *in_proxy)
			: nld_base_proxy(anetlist, name, in_proxied, in_proxy)
	, m_Q(*this, "Q")
	{
	}

	nld_a_to_d_proxy::nld_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name, logic_input_t *in_proxied)
			: nld_base_a_to_d_proxy(anetlist, name, in_proxied, &m_I)
	, m_I(*this, "I")
	{
	}

	NETLIB_RESET(a_to_d_proxy)
	{
	}

	NETLIB_UPDATE(a_to_d_proxy)
	{
		nl_assert(m_logic_family != nullptr);
		// FIXME: Variable supply voltage!
		double supply_V = logic_family()->fixed_V();
		if (supply_V == 0.0) supply_V = 5.0;

		if (m_I.Q_Analog() > logic_family()->high_thresh_V(0.0, supply_V))
			out().push(1, NLTIME_FROM_NS(1));
		else if (m_I.Q_Analog() < logic_family()->low_thresh_V(0.0, supply_V))
			out().push(0, NLTIME_FROM_NS(1));
		else
		{
			// do nothing
		}
	}

	// ----------------------------------------------------------------------------------------
	// nld_d_to_a_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_d_to_a_proxy::nld_base_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
			logic_output_t *out_proxied, detail::core_terminal_t &proxy_out)
	: nld_base_proxy(anetlist, name, out_proxied, &proxy_out)
	, m_I(*this, "I")
	{
	}

	nld_d_to_a_proxy::nld_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name, logic_output_t *out_proxied)
	: nld_base_d_to_a_proxy(anetlist, name, out_proxied, m_RV.m_P)
	, m_GNDHack(*this, "_Q")
	, m_RV(*this, "RV")
	, m_last_state(*this, "m_last_var", -1)
	, m_is_timestep(false)
	{
		const std::vector<std::pair<pstring, pstring>> power_syms = { {"VCC", "VEE"}, {"VCC", "GND"}, {"VDD", "VSS"}};

		register_subalias("Q", m_RV.m_P);

		connect(m_RV.m_N, m_GNDHack);
		bool f = false;
		for (auto & pwr_sym : power_syms)
		{
			pstring devname = out_proxied->device().name();
			auto tp = setup().find_terminal(devname + "." + pwr_sym.first,
					detail::terminal_type::INPUT, false);
			auto tn = setup().find_terminal(devname + "." + pwr_sym.second,
					detail::terminal_type::INPUT, false);
			if (tp != nullptr && tn != nullptr)
			{
				/* alternative logic */
				f = true;
			}
		}
		//FIXME: Use power terminals!
		if (!f)
			log().warning(MW_1_NO_POWER_TERMINALS_ON_DEVICE_1, out_proxied->device().name());
		else
			log().verbose("D/A Proxy: Found power terminals on device {1}", out_proxied->device().name());
	}


	void nld_d_to_a_proxy::reset()
	{
		// FIXME: Variable voltage
		double supply_V = logic_family()->fixed_V();
		if (supply_V == 0.0) supply_V = 5.0;

		//m_Q.initial(0.0);
		m_last_state = -1;
		m_RV.reset();
		m_is_timestep = m_RV.m_P.net().solver()->has_timestep_devices();
		m_RV.set(plib::constants<nl_double>::one() / logic_family()->R_low(),
				logic_family()->low_V(0.0, supply_V), 0.0);
	}

	NETLIB_UPDATE(d_to_a_proxy)
	{
		const auto state = static_cast<int>(m_I());
		if (state != m_last_state)
		{
			// FIXME: Variable voltage
			double supply_V = logic_family()->fixed_V();
			if (supply_V == 0.0) supply_V = 5.0;
			m_last_state = state;
			const nl_double R = state ? logic_family()->R_high() : logic_family()->R_low();
			const nl_double V = state ? logic_family()->high_V(0.0, supply_V) : logic_family()->low_V(0.0, supply_V);

			// We only need to update the net first if this is a time stepping net
			if (m_is_timestep)
			{
				m_RV.update();
			}
			m_RV.set(plib::constants<nl_double>::one() / R, V, 0.0);
			m_RV.solve_later();
		}
	}


	} //namespace devices
} // namespace netlist
