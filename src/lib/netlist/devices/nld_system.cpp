// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.c
 *
 */

#include <solver/nld_solver.h>
#include "nld_system.h"

NETLIB_NAMESPACE_DEVICES_START()

// ----------------------------------------------------------------------------------------
// netlistparams
// ----------------------------------------------------------------------------------------

NETLIB_START(netlistparams)
{
	register_param("USE_DEACTIVATE", m_use_deactivate, 0);
}

NETLIB_RESET(netlistparams)
{
}

NETLIB_UPDATE_PARAM(netlistparams)
{
}

NETLIB_UPDATE(netlistparams)
{
}


// ----------------------------------------------------------------------------------------
// clock
// ----------------------------------------------------------------------------------------

NETLIB_START(clock)
{
	register_output("Q", m_Q);
	register_input("FB", m_feedback);

	register_param("FREQ", m_freq, 7159000.0 * 5.0);
	m_inc = netlist_time::from_hz(m_freq.Value()*2);

	connect_late(m_feedback, m_Q);
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
// extclock
// ----------------------------------------------------------------------------------------

NETLIB_START(extclock)
{
	register_output("Q", m_Q);
	register_input("FB", m_feedback);

	register_param("FREQ", m_freq, 7159000.0 * 5.0);
	register_param("PATTERN", m_pattern, "1,1");
	register_param("OFFSET", m_offset, 0.0);
	m_inc[0] = netlist_time::from_hz(m_freq.Value()*2);

	connect_late(m_feedback, m_Q);
	{
		netlist_time base = netlist_time::from_hz(m_freq.Value()*2);
		pstring_list_t pat(m_pattern.Value(),",");
		m_off = netlist_time::from_double(m_offset.Value());

		int pati[256];
		m_size = pat.size();
		int total = 0;
		for (int i=0; i<m_size; i++)
		{
			pati[i] = pat[i].as_long();
			total += pati[i];
		}
		netlist_time ttotal = netlist_time::zero;
		for (int i=0; i<m_size - 1; i++)
		{
			m_inc[i] = base * pati[i];
			ttotal += m_inc[i];
		}
		m_inc[m_size - 1] = base * total - ttotal;
	}
	save(NLNAME(m_cnt));
	save(NLNAME(m_off));
}

NETLIB_RESET(extclock)
{
	m_cnt = 0;
	m_off = netlist_time::from_double(m_offset.Value());
	m_Q.initial(0);
}

NETLIB_UPDATE_PARAM(extclock)
{
}

NETLIB_UPDATE(extclock)
{
	if (m_cnt != 0)
	{
		OUTLOGIC(m_Q, (m_cnt & 1) ^ 1, m_inc[m_cnt]);
		m_cnt = (m_cnt + 1) % m_size;
	}
	else
	{
		OUTLOGIC(m_Q, (m_cnt & 1) ^ 1, m_inc[0] + m_off);
		m_cnt = 1;
		m_off = netlist_time::zero;
	}
}

// ----------------------------------------------------------------------------------------
// logic_input
// ----------------------------------------------------------------------------------------

NETLIB_START(logic_input)
{
	/* make sure we get the family first */
	register_param("FAMILY", m_FAMILY, "FAMILY(TYPE=TTL)");
	set_logic_family(netlist().setup().family_from_model(m_FAMILY.Value()));

	register_output("Q", m_Q);
	register_param("IN", m_IN, 0);
}

NETLIB_RESET(logic_input)
{
}

NETLIB_STOP(logic_input)
{
	if (m_logic_family != NULL)
		if (!m_logic_family->m_is_static)
			pfree(m_logic_family);
}


NETLIB_UPDATE(logic_input)
{
	OUTLOGIC(m_Q, m_IN.Value() & 1, netlist_time::from_nsec(1));
}

NETLIB_UPDATE_PARAM(logic_input)
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

void nld_d_to_a_proxy::start()
{
	nld_base_d_to_a_proxy::start();

	register_sub("RV", m_RV);
	register_terminal("1", m_RV.m_P);
	register_terminal("2", m_RV.m_N);

	register_output("_Q", m_Q);
	register_subalias("Q", m_RV.m_P);

	connect_direct(m_RV.m_N, m_Q);

	save(NLNAME(m_last_state));
}

void nld_d_to_a_proxy::reset()
{
	m_Q.initial(0.0);
	m_last_state = -1;
	m_RV.do_reset();
	m_is_timestep = m_RV.m_P.net().as_analog().solver()->is_timestep();
	m_RV.set(NL_FCONST(1.0) / logic_family().m_R_low, logic_family().m_low_V, 0.0);
}

ATTR_HOT void nld_d_to_a_proxy::update()
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

NETLIB_START(res_sw)
{
	register_sub("R", m_R);
	register_input("I", m_I);
	register_param("RON", m_RON, 1.0);
	register_param("ROFF", m_ROFF, 1.0E20);

	register_subalias("1", m_R.m_P);
	register_subalias("2", m_R.m_N);

	save(NLNAME(m_last_state));
}

NETLIB_RESET(res_sw)
{
	m_last_state = 0;
	m_R.set_R(m_ROFF.Value());
}

NETLIB_UPDATE(res_sw)
{
	const int state = INPLOGIC(m_I);
	if (state != m_last_state)
	{
		m_last_state = state;
		const nl_double R = state ? m_RON.Value() : m_ROFF.Value();

		// We only need to update the net first if this is a time stepping net
		if (0) // m_R.m_P.net().as_analog().solver()->is_timestep())
		{
			m_R.update_dev();
			m_R.set_R(R);
			m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
		}
		else
		{
			m_R.set_R(R);
			m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
			//m_R.update_dev();
		}
	}
}

NETLIB_UPDATE_PARAM(res_sw)
{
	// nothing, not intended to be called
}

/* -----------------------------------------------------------------------------
 * nld_function
 * ----------------------------------------------------------------------------- */

NETLIB_START(function)
{
	register_param("N", m_N, 2);
	register_param("FUNC", m_func, "");
	register_output("Q", m_Q);

	for (int i=0; i < m_N; i++)
		register_input(pfmt("A{1}")(i), m_I[i]);

	pstring_list_t cmds(m_func.Value(), " ");
	m_precompiled.clear();

	for (std::size_t i=0; i < cmds.size(); i++)
	{
		pstring cmd = cmds[i];
		rpn_inst rc;
		if (cmd == "+")
			rc.m_cmd = ADD;
		else if (cmd == "-")
			rc.m_cmd = SUB;
		else if (cmd == "*")
			rc.m_cmd = MULT;
		else if (cmd == "/")
			rc.m_cmd = DIV;
		else if (cmd.startsWith("A"))
		{
			rc.m_cmd = PUSH_INPUT;
			rc.m_param = cmd.substr(1).as_long();
		}
		else
		{
			bool err = false;
			rc.m_cmd = PUSH_CONST;
			rc.m_param = cmd.as_double(&err);
			if (err)
				netlist().log().fatal("nld_function: unknown/misformatted token <{1}> in <{2}>", cmd, m_func.Value());
		}
		m_precompiled.add(rc);
	}

}

NETLIB_RESET(function)
{
	m_Q.initial(0.0);
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
				stack[ptr++] = INPANALOG(m_I[(int) rc.m_param]);
				break;
			case PUSH_CONST:
				stack[ptr++] = rc.m_param;
				break;
		}
	}
	OUTANALOG(m_Q, stack[ptr-1]);
}


NETLIB_NAMESPACE_DEVICES_END()
