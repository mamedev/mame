// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.c
 *
 */

#include "nld_7493.h"
#include "nl_setup.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7493ff)
	{
		NETLIB_CONSTRUCTOR(7493ff)
		, m_I(*this, "CLK")
		, m_Q(*this, "Q")
		, m_reset(*this, "m_reset", 0)
		, m_state(*this, "m_state", 0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		logic_input_t m_I;
		logic_output_t m_Q;

		state_var_u8 m_reset;
		state_var_u8 m_state;
	};

	NETLIB_OBJECT(7493)
	{
		NETLIB_CONSTRUCTOR(7493)
		, m_R1(*this, "R1")
		, m_R2(*this, "R2")
		, A(*this, "A")
		, B(*this, "B")
		, C(*this, "C")
		, D(*this, "D")
		{
			register_subalias("CLKA", A.m_I);
			register_subalias("CLKB", B.m_I);

			register_subalias("QA", A.m_Q);
			register_subalias("QB", B.m_Q);
			register_subalias("QC", C.m_Q);
			register_subalias("QD", D.m_Q);

			connect_late(C.m_I, B.m_Q);
			connect_late(D.m_I, C.m_Q);
		}

		NETLIB_RESETI() { }
		NETLIB_UPDATEI();

		logic_input_t m_R1;
		logic_input_t m_R2;

		NETLIB_SUB(7493ff) A;
		NETLIB_SUB(7493ff) B;
		NETLIB_SUB(7493ff) C;
		NETLIB_SUB(7493ff) D;
	};

	NETLIB_OBJECT_DERIVED(7493_dip, 7493)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7493_dip, 7493)
		{
			register_subalias("1", B.m_I);
			register_subalias("2", m_R1);
			register_subalias("3", m_R2);

			// register_subalias("4", ); --> NC
			// register_subalias("5", ); --> VCC
			// register_subalias("6", ); --> NC
			// register_subalias("7", ); --> NC

			register_subalias("8", C.m_Q);
			register_subalias("9", B.m_Q);
			// register_subalias("10", ); -. GND
			register_subalias("11", D.m_Q);
			register_subalias("12", A.m_Q);
			// register_subalias("13", ); -. NC
			register_subalias("14", A.m_I);
		}
	};

	NETLIB_RESET(7493ff)
	{
		m_reset = 1;
		m_state = 0;
		m_I.set_state(logic_t::STATE_INP_HL);
	}

	NETLIB_UPDATE(7493ff)
	{
		constexpr netlist_time out_delay = NLTIME_FROM_NS(18);
		if (m_reset)
		{
			m_state ^= 1;
			OUTLOGIC(m_Q, m_state, out_delay);
		}
	}

	NETLIB_UPDATE(7493)
	{
		const netlist_sig_t r = INPLOGIC(m_R1) & INPLOGIC(m_R2);

		if (r)
		{
			A.m_I.inactivate();
			B.m_I.inactivate();
			OUTLOGIC(A.m_Q, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(B.m_Q, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(C.m_Q, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(D.m_Q, 0, NLTIME_FROM_NS(40));
			A.m_reset = B.m_reset = C.m_reset = D.m_reset = 0;
			A.m_state = B.m_state = C.m_state = D.m_state = 0;
		}
		else
		{
			A.m_I.activate_hl();
			B.m_I.activate_hl();
			A.m_reset = B.m_reset = C.m_reset = D.m_reset = 1;
		}
	}

	NETLIB_DEVICE_IMPL(7493)
	NETLIB_DEVICE_IMPL(7493_dip)

	} //namespace devices
} // namespace netlist
