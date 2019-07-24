// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_proxy.cpp
 *
 */

#include "nlid_proxy.h"
#include "netlist/solver/nld_solver.h"

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
			out().push(1, netlist_time::quantum());
		else if (m_I.Q_Analog() < logic_family()->low_thresh_V(0.0, supply_V))
			out().push(0, netlist_time::quantum());
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
	: nld_base_d_to_a_proxy(anetlist, name, out_proxied, m_RN.m_P)
	, m_RP(*this, "RP")
	, m_RN(*this, "RN")
	, m_last_state(*this, "m_last_var", -1)
	, m_is_timestep(false)
	{
		const std::vector<std::pair<pstring, pstring>> power_syms = { {"VCC", "VEE"}, {"VCC", "GND"}, {"VDD", "VSS"}};

		register_subalias("Q", m_RN.m_P);

		bool f = false;
		detail::core_terminal_t *tp(nullptr);
		detail::core_terminal_t *tn(nullptr);
		for (auto & pwr_sym : power_syms)
		{
			pstring devname = out_proxied->device().name();
			auto tp_t = setup().find_terminal(devname + "." + pwr_sym.first,
					/*detail::terminal_type::INPUT,*/ false);
			auto tn_t = setup().find_terminal(devname + "." + pwr_sym.second,
					/*detail::terminal_type::INPUT,*/ false);
			if (f && (tp_t != nullptr && tn_t != nullptr))
				log().warning(MI_MULTIPLE_POWER_TERMINALS_ON_DEVICE(out_proxied->device().name(),
					tp->name(), tn->name(),
					tp_t ? tp_t->name() : "",
					tn_t ? tn_t->name() : ""));
			else if (tp_t != nullptr && tn_t != nullptr)
			{
				/* alternative logic */
				tp = tp_t;
				tn = tn_t;
				f = true;
			}
		}
		//FIXME: Use power terminals and change info to warning or error
		if (!f)
		{
			if (logic_family()->fixed_V() == 0.0)
				log().error(MI_NO_POWER_TERMINALS_ON_DEVICE_1(setup().de_alias(out_proxied->device().name())));
			else
				log().info(MI_NO_POWER_TERMINALS_ON_DEVICE_1(setup().de_alias(out_proxied->device().name())));
			m_GNDHack = plib::make_unique<analog_output_t>(*this, "_QGND");
			m_VCCHack = plib::make_unique<analog_output_t>(*this, "_QVCC");

			connect(m_RN.m_N, *m_GNDHack);
			connect(m_RP.m_P, *m_VCCHack);
			connect(m_RN.m_P, m_RP.m_N);
		}
		else
		{
			log().verbose("D/A Proxy: Found power terminals on device {1}", out_proxied->device().name());
			if (setup().is_validation())
			{
				// During validation, don't connect to terminals found
				// This will cause terminals not connected to a rail net to
				// fail connection stage.
				connect(m_RN.m_N, m_RP.m_P);
			}
			else
			{
				connect(m_RN.m_N, *tn);
				connect(m_RP.m_P, *tp);
			}
			connect(m_RN.m_P, m_RP.m_N);
		}
		//printf("vcc: %f\n", logic_family()->fixed_V());
	}


	void nld_d_to_a_proxy::reset()
	{
		// FIXME: Variable voltage
		double supply_V = logic_family()->fixed_V();
		if (supply_V == 0.0) supply_V = 5.0;

		//m_Q.initial(0.0);
		m_last_state = -1;
		m_RN.reset();
		m_RP.reset();
		if (m_GNDHack)
			m_GNDHack->initial(0);
		if (m_VCCHack)
			m_VCCHack->initial(supply_V);
		m_is_timestep = m_RN.m_P.net().solver()->has_timestep_devices();
		m_RN.set_G_V_I(plib::constants<nl_double>::one() / logic_family()->R_low(),
				logic_family()->low_offset_V(), 0.0);
		m_RP.set_G_V_I(G_OFF,
				0.0, 0.0);
	}

	NETLIB_UPDATE(d_to_a_proxy)
	{
		const auto state = static_cast<int>(m_I());
		if (state != m_last_state)
		{
			// We only need to update the net first if this is a time stepping net
			if (m_is_timestep)
			{
				m_RN.update(); // RN, RP are connected ...
			}
			if (state)
			{
				m_RN.set_G_V_I(G_OFF,
						0.0, 0.0);
				m_RP.set_G_V_I(plib::constants<nl_double>::one() / logic_family()->R_high(),
						logic_family()->high_offset_V(), 0.0);
			}
			else
			{
				m_RN.set_G_V_I(plib::constants<nl_double>::one() / logic_family()->R_low(),
						logic_family()->low_offset_V(), 0.0);
				m_RP.set_G_V_I(G_OFF,
						0.0, 0.0);
			}
			m_RN.solve_later(); // RN, RP are connected ...
			m_last_state = state;
		}
	}


	} //namespace devices
} // namespace netlist
