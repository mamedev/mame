// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.c
 *
 */

#include "../solver/nld_solver.h"
#include "../solver/nld_matrix_solver.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{
	// ----------------------------------------------------------------------------------------
	// netlistparams
	// ----------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------
	// clock
	// ----------------------------------------------------------------------------------------

	NETLIB_UPDATE_PARAM(clock)
	{
		m_inc = netlist_time::from_double(1.0 / (m_freq() * 2.0));
	}

	NETLIB_UPDATE(clock)
	{
		m_Q.push(!m_feedback(), m_inc);
	}

	// ----------------------------------------------------------------------------------------
	// extclock
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(extclock)
	{
		m_cnt = 0;
		m_off = netlist_time::from_double(m_offset());
		//m_Q.initial(0);
	}

	NETLIB_UPDATE(extclock)
	{
		m_Q.push((m_cnt & 1) ^ 1, m_inc[m_cnt] + m_off);
		m_off = netlist_time::zero();
		if (++m_cnt >= m_size)
			m_cnt = 0;
	}

	// -----------------------------------------------------------------------------
	// nld_res_sw
	// -----------------------------------------------------------------------------

	NETLIB_RESET(res_sw)
	{
		m_last_state = 0;
		m_R.set_R(m_ROFF());
	}

	NETLIB_UPDATE(res_sw)
	{
		const netlist_sig_t state = m_I();
		if (state != m_last_state)
		{
			m_last_state = state;
			const nl_double R = state ? m_RON() : m_ROFF();

			// We only need to update the net first if this is a time stepping net
			if ((0)) // m_R->m_P.net().as_analog().solver()->is_timestep())
			{
				m_R.update();
				m_R.set_R(R);
				m_R.m_P.schedule_solve_after(NLTIME_FROM_NS(1));
			}
			else
			{
				m_R.set_R(R);
				m_R.m_P.schedule_solve_after(NLTIME_FROM_NS(1));
				//m_R->update();
			}
		}
	}

	/* -----------------------------------------------------------------------------
	 * nld_function
	 * ----------------------------------------------------------------------------- */

	NETLIB_RESET(function)
	{
		//m_Q.initial(0.0);
	}

	NETLIB_UPDATE(function)
	{
		for (std::size_t i=0; i < static_cast<unsigned>(m_N()); i++)
		{
			m_vals[i] = (*m_I[i])();
		}
		m_Q.push(m_compiled.evaluate(m_vals));
	}


	NETLIB_DEVICE_IMPL(dummy_input, "DUMMY_INPUT",            "")
	NETLIB_DEVICE_IMPL(frontier, "FRONTIER_DEV",           "+I,+G,+Q")
	NETLIB_DEVICE_IMPL(function, "AFUNC",                  "N,FUNC")
	NETLIB_DEVICE_IMPL(analog_input,        "ANALOG_INPUT",           "IN")
	NETLIB_DEVICE_IMPL(clock,               "CLOCK",                  "FREQ")
	NETLIB_DEVICE_IMPL(extclock,            "EXTCLOCK",               "FREQ,PATTERN")
	NETLIB_DEVICE_IMPL(res_sw,              "RES_SWITCH",             "+IN,+P1,+P2")
	NETLIB_DEVICE_IMPL(mainclock,           "MAINCLOCK",              "FREQ")
	NETLIB_DEVICE_IMPL(gnd,                 "GND",                    "")
	NETLIB_DEVICE_IMPL(netlistparams,       "PARAMETER",              "")

	NETLIB_DEVICE_IMPL(logic_input, "LOGIC_INPUT", "IN,FAMILY")
	NETLIB_DEVICE_IMPL_ALIAS(logic_input_ttl, logic_input, "TTL_INPUT", "IN")

	} //namespace devices
} // namespace netlist
