// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.c
 *
 */

#include "nl_base.h"
#include "nl_factory.h"

#include <array>

namespace netlist
{
	namespace devices
	{
	#if !(NL_USE_TRUTHTABLE_7448)
	NETLIB_OBJECT(7448)
	{
		NETLIB_CONSTRUCTOR(7448)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_C(*this, "C", NETLIB_DELEGATE(inputs))
		, m_D(*this, "D", NETLIB_DELEGATE(inputs))
		, m_LTQ(*this, "LTQ", NETLIB_DELEGATE(inputs))
		, m_BIQ(*this, "BIQ", NETLIB_DELEGATE(inputs))
		, m_RBIQ(*this, "RBIQ", NETLIB_DELEGATE(inputs))
		, m_state(*this, "m_state", 0)
		, m_Q(*this, {"a", "b", "c", "d", "e", "f", "g"})
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_state = 0;
		}

		friend class NETLIB_NAME(7448_dip);
	private:
		void update_outputs(unsigned v) noexcept
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

		NETLIB_HANDLERI(inputs)
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

		static const std::array<uint8_t, 16> tab7448;
	};

	NETLIB_OBJECT(7448_dip)
	{
		NETLIB_CONSTRUCTOR(7448_dip)
		, A(*this, "A")
		{
			register_subalias("1", A.m_B);
			register_subalias("2", A.m_C);
			register_subalias("3", A.m_LTQ);
			register_subalias("4", A.m_BIQ);
			register_subalias("5", A.m_RBIQ);
			register_subalias("6", A.m_D);
			register_subalias("7", A.m_A);
			register_subalias("8", "A.GND");

			register_subalias("9",  A.m_Q[4]); // e
			register_subalias("10", A.m_Q[3]); // d
			register_subalias("11", A.m_Q[2]); // c
			register_subalias("12", A.m_Q[1]); // b
			register_subalias("13", A.m_Q[0]); // a
			register_subalias("14", A.m_Q[6]); // g
			register_subalias("15", A.m_Q[5]); // f
			register_subalias("16", "A.VCC");
		}
	private:
		NETLIB_SUB(7448) A;
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

	NETLIB_DEVICE_IMPL(7448, "TTL_7448", "+A,+B,+C,+D,+LTQ,+BIQ,+RBIQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7448_dip, "TTL_7448_DIP", "")

	#endif


	} //namespace devices
} // namespace netlist
