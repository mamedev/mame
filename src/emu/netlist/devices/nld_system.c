/*
 * nld_system.c
 *
 */

#include "nld_system.h"
#include "../analog/nld_solver.h"

// ----------------------------------------------------------------------------------------
// clock
// ----------------------------------------------------------------------------------------

NETLIB_START(clock)
{
	register_output("Q", m_Q);
	register_input("FB", m_feedback);

	register_param("FREQ", m_freq, 7159000.0 * 5.0);
	m_inc = netlist_time::from_hz(m_freq.Value()*2);

	connect(m_feedback, m_Q);
}

NETLIB_RESET(clock)
{
}

NETLIB_UPDATE_PARAM(clock)
{
	m_inc = netlist_time::from_hz(m_freq.Value()*2);
}

NETLIB_UPDATE(clock)
{
	OUTLOGIC(m_Q, !m_Q.net().as_logic().new_Q(), m_inc  );
}

// ----------------------------------------------------------------------------------------
// logic_input
// ----------------------------------------------------------------------------------------

NETLIB_START(ttl_input)
{
	register_output("Q", m_Q);
	register_param("IN", m_IN, 0);
}

NETLIB_RESET(ttl_input)
{
}

NETLIB_UPDATE(ttl_input)
{
	OUTLOGIC(m_Q, m_IN.Value() & 1, netlist_time::from_nsec(1));
}

NETLIB_UPDATE_PARAM(ttl_input)
{
	update();
}

// ----------------------------------------------------------------------------------------
// analog_input
// ----------------------------------------------------------------------------------------

NETLIB_START(analog_input)
{
	register_output("Q", m_Q);
	register_param("IN", m_IN, 0.0);
}

NETLIB_RESET(analog_input)
{
}

NETLIB_UPDATE(analog_input)
{
	OUTANALOG(m_Q, m_IN.Value());
}

NETLIB_UPDATE_PARAM(analog_input)
{
	update();
}

// ----------------------------------------------------------------------------------------
// nld_d_to_a_proxy
// ----------------------------------------------------------------------------------------

ATTR_COLD void nld_d_to_a_proxy::start()
{
	nld_base_d_to_a_proxy::start();

	register_sub(m_RV, "RV");
	register_terminal("1", m_RV.m_P);
	register_terminal("2", m_RV.m_N);

	register_output("_Q", m_Q);
	register_subalias("Q", m_RV.m_P);

	connect(m_RV.m_N, m_Q);
	m_Q.initial(0.0);
}

ATTR_COLD void nld_d_to_a_proxy::reset()
{
	m_RV.do_reset();
}

ATTR_COLD netlist_core_terminal_t &nld_d_to_a_proxy::out()
{
	return m_RV.m_P;
}

ATTR_HOT ATTR_ALIGN void nld_d_to_a_proxy::update()
{
	const int state = INPLOGIC(m_I);
	if (state != m_last_state)
	{
		m_last_state = state;
		const nl_double R = state ? m_logic_family->m_R_high : m_logic_family->m_R_low;
		const nl_double V = state ? m_logic_family->m_high_V : m_logic_family->m_low_V;

		// We only need to update the net first if this is a time stepping net
		if (m_RV.m_P.net().as_analog().solver()->is_timestep())
		{
			m_RV.update_dev();
			m_RV.set(1.0 / R, V, 0.0);
			m_RV.m_P.schedule_after(NLTIME_FROM_NS(1));
		}
		else
		{
			m_RV.set(1.0 / R, V, 0.0);
			m_RV.update_dev();
		}
	}
}
