// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.c
 *
 */

#include "nld_7483.h"
#include "../nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7483)
	{
		NETLIB_CONSTRUCTOR(7483)
		, m_C0(*this, "C0")
		, m_A1(*this, "A1", NETLIB_DELEGATE(7483, upd_a))
		, m_A2(*this, "A2", NETLIB_DELEGATE(7483, upd_a))
		, m_A3(*this, "A3", NETLIB_DELEGATE(7483, upd_a))
		, m_A4(*this, "A4", NETLIB_DELEGATE(7483, upd_a))
		, m_B1(*this, "B1", NETLIB_DELEGATE(7483, upd_b))
		, m_B2(*this, "B2", NETLIB_DELEGATE(7483, upd_b))
		, m_B3(*this, "B3", NETLIB_DELEGATE(7483, upd_b))
		, m_B4(*this, "B4", NETLIB_DELEGATE(7483, upd_b))
		, m_a(*this, "m_a", 0)
		, m_b(*this, "m_b", 0)
		, m_lastr(*this, "m_lastr", 0)
		, m_S1(*this, "S1")
		, m_S2(*this, "S2")
		, m_S3(*this, "S3")
		, m_S4(*this, "S4")
		, m_C4(*this, "C4")
		{
		}
		NETLIB_RESETI();
		NETLIB_UPDATEI();
		NETLIB_HANDLERI(upd_a);
		NETLIB_HANDLERI(upd_b);

	protected:
		logic_input_t m_C0;
		logic_input_t m_A1;
		logic_input_t m_A2;
		logic_input_t m_A3;
		logic_input_t m_A4;
		logic_input_t m_B1;
		logic_input_t m_B2;
		logic_input_t m_B3;
		logic_input_t m_B4;

		state_var_u8 m_a;
		state_var_u8 m_b;
		state_var_u8 m_lastr;

		logic_output_t m_S1;
		logic_output_t m_S2;
		logic_output_t m_S3;
		logic_output_t m_S4;
		logic_output_t m_C4;

	};

	NETLIB_OBJECT_DERIVED(7483_dip, 7483)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7483_dip, 7483)
		{
			register_subalias("1", m_A4);
			register_subalias("2", m_S3);
			register_subalias("3", m_A3);
			register_subalias("4", m_B3);
			// register_subalias("5", ); --> VCC
			register_subalias("6", m_S2);
			register_subalias("7", m_B2);
			register_subalias("8", m_A2);

			register_subalias("9", m_S1);
			register_subalias("10", m_A1);
			register_subalias("11", m_B1);
			// register_subalias("12", ); --> GND
			register_subalias("13", m_C0);
			register_subalias("14", m_C4);
			register_subalias("15", m_S4);
			register_subalias("16", m_B4);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();
	};

	NETLIB_RESET(7483)
	{
		m_lastr = 0;
	}

	NETLIB_HANDLER(7483, upd_a)
	{
		m_a = static_cast<uint8_t>((m_A1() << 0) | (m_A2() << 1) | (m_A3() << 2) | (m_A4() << 3));
		NETLIB_NAME(7483)::update();
	}

	NETLIB_HANDLER(7483, upd_b)
	{
		m_b = static_cast<uint8_t>((m_B1() << 0) | (m_B2() << 1) | (m_B3() << 2) | (m_B4() << 3));
		NETLIB_NAME(7483)::update();
	}

	NETLIB_UPDATE(7483)
	{
		auto r = static_cast<uint8_t>(m_a + m_b + m_C0());

		if (r != m_lastr)
		{
			m_lastr = r;
			m_S1.push((r >> 0) & 1, NLTIME_FROM_NS(23));
			m_S2.push((r >> 1) & 1, NLTIME_FROM_NS(23));
			m_S3.push((r >> 2) & 1, NLTIME_FROM_NS(23));
			m_S4.push((r >> 3) & 1, NLTIME_FROM_NS(23));
			m_C4.push((r >> 4) & 1, NLTIME_FROM_NS(23));
		}
	}

	NETLIB_DEVICE_IMPL(7483, "TTL_7483", "+A1,+A2,+A3,+A4,+B1,+B2,+B3,+B4,+C0")
	NETLIB_DEVICE_IMPL(7483_dip, "TTL_7483_DIP", "")

	} //namespace devices
} // namespace netlist
