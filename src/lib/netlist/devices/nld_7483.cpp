// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.c
 *
 */

#include "nld_7483.h"

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(7483)
	{
		NETLIB_CONSTRUCTOR(7483)
		, m_lastr(0)
		, m_S1(*this, "S1")
		, m_S2(*this, "S2")
		, m_S3(*this, "S3")
		, m_S4(*this, "S4")
		, m_C4(*this, "C4")
		{
			enregister("A1", m_A1);
			enregister("A2", m_A2);
			enregister("A3", m_A3);
			enregister("A4", m_A4);
			enregister("B1", m_B1);
			enregister("B2", m_B2);
			enregister("B3", m_B3);
			enregister("B4", m_B4);
			enregister("C0", m_C0);

			save(NLNAME(m_lastr));
		}
		NETLIB_RESETI();
		NETLIB_UPDATEI();

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

		UINT8 m_lastr;

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

	NETLIB_UPDATE(7483)
	{
		UINT8 a = (INPLOGIC(m_A1) << 0) | (INPLOGIC(m_A2) << 1) | (INPLOGIC(m_A3) << 2) | (INPLOGIC(m_A4) << 3);
		UINT8 b = (INPLOGIC(m_B1) << 0) | (INPLOGIC(m_B2) << 1) | (INPLOGIC(m_B3) << 2) | (INPLOGIC(m_B4) << 3);

		UINT8 r = a + b + INPLOGIC(m_C0);

		if (r != m_lastr)
		{
			m_lastr = r;
			OUTLOGIC(m_S1, (r >> 0) & 1, NLTIME_FROM_NS(23));
			OUTLOGIC(m_S2, (r >> 1) & 1, NLTIME_FROM_NS(23));
			OUTLOGIC(m_S3, (r >> 2) & 1, NLTIME_FROM_NS(23));
			OUTLOGIC(m_S4, (r >> 3) & 1, NLTIME_FROM_NS(23));
			OUTLOGIC(m_C4, (r >> 4) & 1, NLTIME_FROM_NS(23));
		}
	}

	NETLIB_DEVICE_IMPL(7483)
	NETLIB_DEVICE_IMPL(7483_dip)

	} //namespace devices
} // namespace netlist
