// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_proxy.cpp
 *
 */

//#include <memory>
#include "nlid_proxy.h"
#include "solver/nld_solver.h"
//#include "plib/pstream.h"
//#include "plib/pfmtlog.h"
//#include "nld_log.h"

namespace netlist
{
	namespace devices
	{

	// ----------------------------------------------------------------------------------------
	// nld_d_to_a_proxy
	// ----------------------------------------------------------------------------------------

	nld_d_to_a_proxy::nld_d_to_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *out_proxied)
	: nld_base_d_to_a_proxy(anetlist, name, out_proxied, m_RV.m_P)
	, m_GNDHack(*this, "_Q")
	, m_RV(*this, "RV")
	, m_last_state(*this, "m_last_var", -1)
	, m_is_timestep(false)
	{
		//register_sub(m_RV);
		//register_term("1", m_RV.m_P);
		//register_term("2", m_RV.m_N);

		register_subalias("Q", m_RV.m_P);

		connect_late(m_RV.m_N, m_GNDHack);
		printf("%s %s\n", out_proxied->name().cstr(), out_proxied->device().name().cstr());
		auto x = netlist().setup().find_terminal(out_proxied->name(), detail::device_object_t::type_t::OUTPUT, false);
		if (x) printf("==> %s\n", x->name().cstr());
	}


	void nld_d_to_a_proxy::reset()
	{
		//m_Q.initial(0.0);
		m_last_state = -1;
		m_RV.do_reset();
		m_is_timestep = m_RV.m_P.net().solver()->has_timestep_devices();
		m_RV.set(NL_FCONST(1.0) / logic_family().m_R_low, logic_family().m_low_V, 0.0);
	}

	NETLIB_UPDATE(d_to_a_proxy)
	{
		const int state = static_cast<int>(m_I());
		if (state != m_last_state)
		{
			m_last_state = state;
			const nl_double R = state ? logic_family().m_R_high : logic_family().m_R_low;
			const nl_double V = state ? logic_family().m_high_V : logic_family().m_low_V;

			// We only need to update the net first if this is a time stepping net
			if (m_is_timestep)
			{
				m_RV.update_dev();
			}
			m_RV.set(NL_FCONST(1.0) / R, V, 0.0);
			m_RV.m_P.schedule_after(NLTIME_FROM_NS(1));
		}
	}


	} //namespace devices
} // namespace netlist
