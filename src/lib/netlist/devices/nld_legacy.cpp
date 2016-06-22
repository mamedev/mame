// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_legacy.c
 *
 */

#include "nld_legacy.h"
#include "nl_setup.h"

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(nicRSFF)
	{
		NETLIB_CONSTRUCTOR(nicRSFF)
		, m_S(*this, "S")
		, m_R(*this, "R")
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_S;
		logic_input_t m_R;

		logic_output_t m_Q;
		logic_output_t m_QQ;
	};


	NETLIB_OBJECT(nicDelay)
	{
		NETLIB_CONSTRUCTOR(nicDelay)
		, m_I(*this, "1")
		, m_Q(*this, "2")
		, m_L_to_H(*this, "L_TO_H", 10)
		, m_H_to_L(*this, "H_TO_L", 10)
		, m_last(*this, "m_last", 0)
		{
		}

		//NETLIB_UPDATE_PARAMI();
		NETLIB_RESETI();
		NETLIB_UPDATEI();

	protected:
		logic_input_t m_I;
		logic_output_t m_Q;

		param_int_t m_L_to_H;
		param_int_t m_H_to_L;

		state_var_u8 m_last;
	};

	NETLIB_RESET(nicRSFF)
	{
		m_Q.initial(0);
		m_QQ.initial(1);
	}

	NETLIB_UPDATE(nicRSFF)
	{
		if (!INPLOGIC(m_S))
		{
			OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(20));
			OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(20));
		}
		else if (!INPLOGIC(m_R))
		{
			OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(20));
			OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(20));
		}
	}

	NETLIB_RESET(nicDelay)
	{
		//m_Q.initial(0);
	}

	NETLIB_UPDATE(nicDelay)
	{
		netlist_sig_t nval = INPLOGIC(m_I);
		if (nval && !m_last)
		{
			// L_to_H
			OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(m_L_to_H.Value()));
		}
		else if (!nval && m_last)
		{
			// H_to_L
			OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(m_H_to_L.Value()));
		}
		m_last = nval;
	}

	NETLIB_DEVICE_IMPL(nicRSFF)
	NETLIB_DEVICE_IMPL(nicDelay)

	} //namespace devices
} // namespace netlist
