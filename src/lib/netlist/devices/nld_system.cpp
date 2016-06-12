// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.c
 *
 */

#include <solver/nld_solver.h>
#include <solver/nld_matrix_solver.h>
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
		m_inc = netlist_time::from_hz(m_freq.Value()*2);
	}

	NETLIB_UPDATE(clock)
	{
		OUTLOGIC(m_Q, !INPLOGIC(m_feedback), m_inc  );
	}

	// ----------------------------------------------------------------------------------------
	// extclock
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(extclock)
	{
		m_cnt = 0;
		m_off = netlist_time::from_double(m_offset.Value());
		//m_Q.initial(0);
	}

	NETLIB_UPDATE(extclock)
	{
		OUTLOGIC(m_Q, (m_cnt & 1) ^ 1, m_inc[m_cnt] + m_off);
		m_cnt = (m_cnt + 1) % m_size;
		m_off = netlist_time::zero();
	}

	// ----------------------------------------------------------------------------------------
	// logic_input
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(logic_input)
	{
		m_Q.initial(0);
	}

	NETLIB_UPDATE(logic_input)
	{
		OUTLOGIC(m_Q, m_IN.Value() & 1, netlist_time::from_nsec(1));
	}

	NETLIB_UPDATE_PARAM(logic_input)
	{
	}

	// ----------------------------------------------------------------------------------------
	// analog_input
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(analog_input)
	{
		m_Q.initial(0.0);
	}

	NETLIB_UPDATE(analog_input)
	{
		OUTANALOG(m_Q, m_IN.Value());
	}

	NETLIB_UPDATE_PARAM(analog_input)
	{
	}

	// ----------------------------------------------------------------------------------------
	// nld_d_to_a_proxy
	// ----------------------------------------------------------------------------------------

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
		const int state = INPLOGIC(m_I);
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


	// -----------------------------------------------------------------------------
	// nld_res_sw
	// -----------------------------------------------------------------------------


	NETLIB_UPDATE(res_sw)
	{
		const int state = INPLOGIC(m_I);
		if (state != m_last_state)
		{
			m_last_state = state;
			const nl_double R = state ? m_RON.Value() : m_ROFF.Value();

			// We only need to update the net first if this is a time stepping net
			if (0) // m_R->m_P.net().as_analog().solver()->is_timestep())
			{
				m_R.update_dev();
				m_R.set_R(R);
				m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
			}
			else
			{
				m_R.set_R(R);
				m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
				//m_R->update_dev();
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
		//nl_double val = INPANALOG(m_I[0]) * INPANALOG(m_I[1]) * 0.2;
		//OUTANALOG(m_Q, val);
		nl_double stack[20];
		unsigned ptr = 0;
		unsigned e = m_precompiled.size();
		for (unsigned i = 0; i<e; i++)
		{
			rpn_inst &rc = m_precompiled[i];
			switch (rc.m_cmd)
			{
				case ADD:
					ptr--;
					stack[ptr-1] = stack[ptr] + stack[ptr-1];
					break;
				case MULT:
					ptr--;
					stack[ptr-1] = stack[ptr] * stack[ptr-1];
					break;
				case SUB:
					ptr--;
					stack[ptr-1] = stack[ptr-1] - stack[ptr];
					break;
				case DIV:
					ptr--;
					stack[ptr-1] = stack[ptr-1] / stack[ptr];
					break;
				case PUSH_INPUT:
					stack[ptr++] = INPANALOG((*m_I[(int) rc.m_param]));
					break;
				case PUSH_CONST:
					stack[ptr++] = rc.m_param;
					break;
			}
		}
		OUTANALOG(m_Q, stack[ptr-1]);
	}


	NETLIB_DEVICE_IMPL(dummy_input)
	NETLIB_DEVICE_IMPL(frontier)
	NETLIB_DEVICE_IMPL(function)
	NETLIB_DEVICE_IMPL(logic_input)
	NETLIB_DEVICE_IMPL(analog_input)
	NETLIB_DEVICE_IMPL(clock)
	NETLIB_DEVICE_IMPL(extclock)
	NETLIB_DEVICE_IMPL(res_sw)
	NETLIB_DEVICE_IMPL(mainclock)
	NETLIB_DEVICE_IMPL(gnd)
	NETLIB_DEVICE_IMPL(netlistparams)

	} //namespace devices
} // namespace netlist
