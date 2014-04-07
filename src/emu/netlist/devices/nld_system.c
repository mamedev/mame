/*
 * nld_system.c
 *
 */

#include "nld_system.h"

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
	OUTLOGIC(m_Q, !m_Q.net().new_Q(), m_inc  );
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
	OUTANALOG(m_Q, m_IN.Value(), NLTIME_IMMEDIATE);
}

NETLIB_UPDATE_PARAM(analog_input)
{
	update();
}
