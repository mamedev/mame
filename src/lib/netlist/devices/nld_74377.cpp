// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74377.c
 *
 */

#include "nld_74377.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };

	NETLIB_OBJECT(74377_gate)
	{
		NETLIB_CONSTRUCTOR(74377_gate)
		, m_E(*this, "E")
		, m_D(*this, "D")
		, m_CP(*this, "CP")
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_cp(*this, "m_cp", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
		}
		NETLIB_UPDATEI()
		{
			netlist_sig_t last_cp = m_cp;

			m_cp = m_CP();

			if (!m_E() && !last_cp && m_cp)
			{
				netlist_sig_t d = m_D();
				m_Q.push(d, delay[d]);
				m_QQ.push(d ^ 1, delay[d ^ 1]);
			}
		}

		friend class NETLIB_NAME(74377_dip);
		friend class NETLIB_NAME(74378_dip);
		friend class NETLIB_NAME(74379_dip);
	private:
		logic_input_t m_E;
		logic_input_t m_D;
		logic_input_t m_CP;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var_sig m_cp;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(74377_dip)
	{
		NETLIB_CONSTRUCTOR(74377_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_E(*this, "E")
		, m_F(*this, "F")
		, m_G(*this, "G")
		, m_H(*this, "H")
		{
			register_subalias("1", m_A.m_E);
			register_subalias("2", m_A.m_Q);
			register_subalias("3", m_A.m_D);
			register_subalias("4", m_B.m_D);
			register_subalias("5", m_B.m_Q);
			register_subalias("6", m_C.m_Q);
			register_subalias("7", m_C.m_D);
			register_subalias("8", m_D.m_D);
			register_subalias("9", m_D.m_Q);
			register_subalias("10", "A.GND");

			register_subalias("11", m_A.m_CP);
			register_subalias("12", m_E.m_Q);
			register_subalias("13", m_E.m_D);
			register_subalias("14", m_F.m_D);
			register_subalias("15", m_F.m_Q);
			register_subalias("16", m_G.m_Q);
			register_subalias("17", m_G.m_D);
			register_subalias("18", m_H.m_D);
			register_subalias("19", m_H.m_Q);
			register_subalias("20", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");
			connect("A.GND", "E.GND");
			connect("A.GND", "F.GND");
			connect("A.GND", "G.GND");
			connect("A.GND", "H.GND");
			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect("A.VCC", "E.VCC");
			connect("A.VCC", "F.VCC");
			connect("A.VCC", "G.VCC");
			connect("A.VCC", "H.VCC");
			connect(m_A.m_E, m_B.m_E);
			connect(m_A.m_E, m_C.m_E);
			connect(m_A.m_E, m_D.m_E);
			connect(m_A.m_E, m_E.m_E);
			connect(m_A.m_E, m_F.m_E);
			connect(m_A.m_E, m_G.m_E);
			connect(m_A.m_E, m_H.m_E);
			connect(m_A.m_CP, m_B.m_CP);
			connect(m_A.m_CP, m_C.m_CP);
			connect(m_A.m_CP, m_D.m_CP);
			connect(m_A.m_CP, m_E.m_CP);
			connect(m_A.m_CP, m_F.m_CP);
			connect(m_A.m_CP, m_G.m_CP);
			connect(m_A.m_CP, m_H.m_CP);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(74377_gate) m_A;
		NETLIB_SUB(74377_gate) m_B;
		NETLIB_SUB(74377_gate) m_C;
		NETLIB_SUB(74377_gate) m_D;
		NETLIB_SUB(74377_gate) m_E;
		NETLIB_SUB(74377_gate) m_F;
		NETLIB_SUB(74377_gate) m_G;
		NETLIB_SUB(74377_gate) m_H;
	};

	NETLIB_OBJECT(74378_dip)
	{
		NETLIB_CONSTRUCTOR(74378_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_E(*this, "E")
		, m_F(*this, "F")
		{
			register_subalias("1", m_A.m_E);
			register_subalias("2", m_A.m_Q);
			register_subalias("3", m_A.m_D);
			register_subalias("4", m_B.m_D);
			register_subalias("5", m_B.m_Q);
			register_subalias("6", m_C.m_D);
			register_subalias("7", m_C.m_Q);
			register_subalias("8", "A.GND");

			register_subalias("9", m_A.m_CP);
			register_subalias("10", m_D.m_Q);
			register_subalias("11", m_D.m_D);
			register_subalias("12", m_E.m_Q);
			register_subalias("13", m_E.m_D);
			register_subalias("14", m_F.m_D);
			register_subalias("15", m_F.m_Q);
			register_subalias("16", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");
			connect("A.GND", "E.GND");
			connect("A.GND", "F.GND");
			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect("A.VCC", "E.VCC");
			connect("A.VCC", "F.VCC");
			connect(m_A.m_E, m_B.m_E);
			connect(m_A.m_E, m_C.m_E);
			connect(m_A.m_E, m_D.m_E);
			connect(m_A.m_E, m_E.m_E);
			connect(m_A.m_E, m_F.m_E);
			connect(m_A.m_CP, m_B.m_CP);
			connect(m_A.m_CP, m_C.m_CP);
			connect(m_A.m_CP, m_D.m_CP);
			connect(m_A.m_CP, m_E.m_CP);
			connect(m_A.m_CP, m_F.m_CP);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(74377_gate) m_A;
		NETLIB_SUB(74377_gate) m_B;
		NETLIB_SUB(74377_gate) m_C;
		NETLIB_SUB(74377_gate) m_D;
		NETLIB_SUB(74377_gate) m_E;
		NETLIB_SUB(74377_gate) m_F;
	};

	NETLIB_OBJECT(74379_dip)
	{
		NETLIB_CONSTRUCTOR(74379_dip)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		{
			register_subalias("1", m_A.m_E);
			register_subalias("2", m_A.m_Q);
			register_subalias("3", m_A.m_QQ);
			register_subalias("4", m_A.m_D);
			register_subalias("5", m_B.m_D);
			register_subalias("6", m_B.m_QQ);
			register_subalias("7", m_B.m_Q);
			register_subalias("8", "A.GND");

			register_subalias("9", m_A.m_CP);
			register_subalias("10", m_C.m_Q);
			register_subalias("11", m_C.m_QQ);
			register_subalias("12", m_C.m_D);
			register_subalias("13", m_D.m_D);
			register_subalias("14", m_D.m_QQ);
			register_subalias("15", m_D.m_Q);
			register_subalias("16", "A.VCC");

			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");
			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect(m_A.m_E, m_B.m_E);
			connect(m_A.m_E, m_C.m_E);
			connect(m_A.m_E, m_D.m_E);
			connect(m_A.m_CP, m_B.m_CP);
			connect(m_A.m_CP, m_C.m_CP);
			connect(m_A.m_CP, m_D.m_CP);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATEI();

	protected:
		NETLIB_SUB(74377_gate) m_A;
		NETLIB_SUB(74377_gate) m_B;
		NETLIB_SUB(74377_gate) m_C;
		NETLIB_SUB(74377_gate) m_D;
	};


	NETLIB_DEVICE_IMPL(74377_gate, "TTL_74377_GATE", "")
	NETLIB_DEVICE_IMPL(74377_dip,  "TTL_74377_DIP", "")
	NETLIB_DEVICE_IMPL(74378_dip,  "TTL_74378_DIP", "")
	NETLIB_DEVICE_IMPL(74379_dip,  "TTL_74379_DIP", "")

	} //namespace devices
} // namespace netlist
