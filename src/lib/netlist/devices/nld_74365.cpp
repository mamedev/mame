// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_74365.cpp
 *
 */

#include "nld_74365.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	/* FIXME: This should be a single device, i.e. one tristate buffer only.
	 *
	 * FIXME: Implement tristate output.
	 *
	 */


	NETLIB_OBJECT(74365)
	{
		NETLIB_CONSTRUCTOR(74365)
		, m_G1Q(*this, "G1Q", NETLIB_DELEGATE(inputs))
		, m_G2Q(*this, "G2Q", NETLIB_DELEGATE(inputs))
		, m_A(*this, { "A1", "A2", "A3", "A4", "A5", "A6" }, NETLIB_DELEGATE(inputs))
		, m_Y(*this, { "Y1", "Y2", "Y3", "Y4", "Y5", "Y6" })
		, m_power_pins(*this)
		{
		}

		NETLIB_UPDATEI()
		{
			inputs();
		}

		friend class NETLIB_NAME(74365_dip);
	private:
		NETLIB_HANDLERI(inputs)
		{
			if (!m_G1Q() && !m_G2Q())
			{
				for (std::size_t i=0; i<6; i++)
					m_Y[i].push(m_A[i](), NLTIME_FROM_NS(20)); // FIXME: Correct timing
			}
			else
			{
				for (std::size_t i=0; i<6; i++)
					m_Y[i].push(0, NLTIME_FROM_NS(20)); // FIXME: Correct timing
			}
		}

		logic_input_t m_G1Q;
		logic_input_t m_G2Q;
		object_array_t<logic_input_t, 6> m_A;
		object_array_t<logic_output_t, 6> m_Y;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(74365_dip)
	{
		NETLIB_CONSTRUCTOR(74365_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_G1Q);
			register_subalias("2", A.m_A[0]);
			register_subalias("3", A.m_Y[0]);
			register_subalias("4", A.m_A[1]);
			register_subalias("5", A.m_Y[1]);
			register_subalias("6", A.m_A[2]);
			register_subalias("7", A.m_Y[2]);
			register_subalias("8", "A.GND");

			register_subalias("9",  A.m_A[3]);
			register_subalias("10", A.m_Y[3]);
			register_subalias("11", A.m_A[4]);
			register_subalias("12", A.m_Y[4]);
			register_subalias("13", A.m_A[5]);
			register_subalias("14", A.m_Y[5]);
			register_subalias("15", A.m_G2Q);
			register_subalias("16", "A.VCC");
		}
		NETLIB_RESETI() {}
		NETLIB_UPDATEI() {}
	private:
		NETLIB_SUB(74365) A;
	};

	NETLIB_DEVICE_IMPL(74365, "TTL_74365", "+G1Q,+G2Q,+A1,+A2,+A3,+A4,+A5,+A6,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74365_dip, "TTL_74365_DIP", "")

	} //namespace devices
} // namespace netlist
