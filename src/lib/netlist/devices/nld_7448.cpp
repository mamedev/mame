// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.c
 *
 */

#include "nld_7448.h"
#include "nlid_system.h"
#include "nlid_truthtable.h"

#include <array>

namespace netlist
{
	namespace devices
	{
	#if !(NL_USE_TRUTHTABLE_7448)
	NETLIB_OBJECT(7448)
	{
		NETLIB_CONSTRUCTOR(7448)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_LTQ(*this, "LTQ")
		, m_BIQ(*this, "BIQ")
		, m_RBIQ(*this, "RBIQ")
		, m_state(*this, "m_state", 0)
		, m_Q(*this, {{"a", "b", "c", "d", "e", "f", "g"}})
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		void update_outputs(unsigned v) NL_NOEXCEPT
		{
			nl_assert(v<16);
			if (v != m_state)
			{
				// max transfer time is 100 NS */

				uint8_t t = tab7448[v];
				for (std::size_t i = 0; i < 7; i++)
					m_Q[i].push((t >> (6-i)) & 1, NLTIME_FROM_NS(100));
				m_state = v;
			}
		}

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_input_t m_LTQ;
		logic_input_t m_BIQ;
		logic_input_t m_RBIQ;

		state_var<unsigned> m_state;

		object_array_t<logic_output_t, 7> m_Q;  /* a .. g */
		nld_power_pins m_power_pins;
	private:
		static const std::array<uint8_t, 16> tab7448;
	};

	NETLIB_OBJECT_DERIVED(7448_dip, 7448)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7448_dip, 7448)
		{
			register_subalias("1", m_B);
			register_subalias("2", m_C);
			register_subalias("3", m_LTQ);
			register_subalias("4", m_BIQ);
			register_subalias("5", m_RBIQ);
			register_subalias("6", m_D);
			register_subalias("7", m_A);
			register_subalias("8", "GND");

			register_subalias("9",  m_Q[4]); // e
			register_subalias("10", m_Q[3]); // d
			register_subalias("11", m_Q[2]); // c
			register_subalias("12", m_Q[1]); // b
			register_subalias("13", m_Q[0]); // a
			register_subalias("14", m_Q[6]); // g
			register_subalias("15", m_Q[5]); // f
			register_subalias("16", "VCC");
		}
	};
	#endif


	#if !(NL_USE_TRUTHTABLE_7448)

#define BITS7(b6,b5,b4,b3,b2,b1,b0) ((b6)<<6) | ((b5)<<5) | ((b4)<<4) | ((b3)<<3) | ((b2)<<2) | ((b1)<<1) | ((b0)<<0)

	const std::array<uint8_t, 16> NETLIB_NAME(7448)::tab7448 =
	{
			BITS7(   1, 1, 1, 1, 1, 1, 0 ),  /* 00 - not blanked ! */
			BITS7(   0, 1, 1, 0, 0, 0, 0 ),  /* 01 */
			BITS7(   1, 1, 0, 1, 1, 0, 1 ),  /* 02 */
			BITS7(   1, 1, 1, 1, 0, 0, 1 ),  /* 03 */
			BITS7(   0, 1, 1, 0, 0, 1, 1 ),  /* 04 */
			BITS7(   1, 0, 1, 1, 0, 1, 1 ),  /* 05 */
			BITS7(   0, 0, 1, 1, 1, 1, 1 ),  /* 06 */
			BITS7(   1, 1, 1, 0, 0, 0, 0 ),  /* 07 */
			BITS7(   1, 1, 1, 1, 1, 1, 1 ),  /* 08 */
			BITS7(   1, 1, 1, 0, 0, 1, 1 ),  /* 09 */
			BITS7(   0, 0, 0, 1, 1, 0, 1 ),  /* 10 */
			BITS7(   0, 0, 1, 1, 0, 0, 1 ),  /* 11 */
			BITS7(   0, 1, 0, 0, 0, 1, 1 ),  /* 12 */
			BITS7(   1, 0, 0, 1, 0, 1, 1 ),  /* 13 */
			BITS7(   0, 0, 0, 1, 1, 1, 1 ),  /* 14 */
			BITS7(   0, 0, 0, 0, 0, 0, 0 ),  /* 15 */
	};

	NETLIB_UPDATE(7448)
	{
		if (!m_BIQ() || (m_BIQ() && !m_LTQ()))
		{
			m_A.inactivate();
			m_B.inactivate();
			m_C.inactivate();
			m_D.inactivate();
			m_RBIQ.inactivate();
			if (m_BIQ() && !m_LTQ())
			{
				update_outputs(8);
			}
			else if (!m_BIQ())
			{
				update_outputs(15);
			}
		} else {
			m_RBIQ.activate();
			m_D.activate();
			m_C.activate();
			m_B.activate();
			m_A.activate();
			unsigned v = (m_A() << 0) | (m_B() << 1) | (m_C() << 2) | (m_D() << 3);
			if ((!m_RBIQ() && (v==0)))
					v = 15;
			update_outputs(v);
		}
	}

	NETLIB_RESET(7448)
	{
		m_state = 0;
#if 0
		m_A.set_state(logic_t::STATE_INP_PASSIVE);
		m_B.set_state(logic_t::STATE_INP_PASSIVE);
		m_C.set_state(logic_t::STATE_INP_PASSIVE);
		m_D.set_state(logic_t::STATE_INP_PASSIVE);
		m_RBIQ.set_state(logic_t::STATE_INP_PASSIVE);
#endif
	}

	NETLIB_DEVICE_IMPL(7448, "TTL_7448", "+A,+B,+C,+D,+LTQ,+BIQ,+RBIQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7448_dip, "TTL_7448_DIP", "")

	#endif


	} //namespace devices
} // namespace netlist
