// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.c
 *
 */

#include "nld_7448.h"

namespace netlist
{
	namespace devices
	{


	/*
	 * FIXME: Using truthtable is a lot slower than the explicit device
	 */

	#define USE_TRUTHTABLE_7448	(0)

	#if (USE_TRUTHTABLE_7448 && USE_TRUTHTABLE)

	NETLIB_TRUTHTABLE(7448, 7, 7, 0);

	#else

	NETLIB_OBJECT(7448)
	{
		NETLIB_CONSTRUCTOR(7448)
		, m_state(0)
		, m_Q(*this, {"a", "b", "c", "d", "e", "f", "g"})
		{
			enregister("A", m_A);
			enregister("B", m_B);
			enregister("C", m_C);
			enregister("D", m_D);
			enregister("LTQ", m_LTQ);
			enregister("BIQ", m_BIQ);
			enregister("RBIQ", m_RBIQ);

			save(NLNAME(m_state));
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		ATTR_HOT void update_outputs(UINT8 v);
		static const UINT8 tab7448[16][7];

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;
		logic_input_t m_LTQ;
		logic_input_t m_BIQ;
		logic_input_t m_RBIQ;

		UINT8 m_state;

		object_array_t<logic_output_t, 7> m_Q;  /* a .. g */

	};

	#endif

	NETLIB_OBJECT_DERIVED(7448_dip, 7448)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7448_dip, 7448)
		{
			register_subalias("1", m_B);
			register_subalias("2", m_C);
			register_subalias("3", m_LTQ);
			register_subalias("4", m_BIQ);
			register_subalias("5",m_RBIQ);
			register_subalias("6", m_D);
			register_subalias("7", m_A);

			register_subalias("9",  m_Q[4]); // e
			register_subalias("10", m_Q[3]); // d
			register_subalias("11", m_Q[2]); // c
			register_subalias("12", m_Q[1]); // b
			register_subalias("13", m_Q[0]); // a
			register_subalias("14", m_Q[6]); // g
			register_subalias("15", m_Q[5]); // f
		}
	};

	#if (USE_TRUTHTABLE_7448 && USE_TRUTHTABLE)
	nld_7448::truthtable_t nld_7448::m_ttbl;
	const char *nld_7448::m_desc[] = {
			" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g",

			"  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100",
			"  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100",

			// BI/RBO is input output. In the next case it is used as an output will go low.
			"  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100", // RBI

			"  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100", // LT

			// This condition has precedence
			"  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100", // BI
			""
	};

	NETLIB_START(7448_dip)
	{
		NETLIB_NAME(7448)::start();

		register_subalias("1", m_I[4]); // B
		register_subalias("2", m_I[5]); // C
		register_subalias("3", m_I[0]); // LTQ
		register_subalias("4", m_I[1]); // BIQ
		register_subalias("5", m_I[2]); // RBIQ
		register_subalias("6", m_I[6]); // D
		register_subalias("7", m_I[3]); // A

		register_subalias("9",  m_Q[4]); // e
		register_subalias("10", m_Q[3]); // d
		register_subalias("11", m_Q[2]); // c
		register_subalias("12", m_Q[1]); // b
		register_subalias("13", m_Q[0]); // a
		register_subalias("14", m_Q[6]); // g
		register_subalias("15", m_Q[5]); // f
	}

	#else

	NETLIB_UPDATE(7448)
	{
		if (!INPLOGIC(m_BIQ) || (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ)))
		{
			m_A.inactivate();
			m_B.inactivate();
			m_C.inactivate();
			m_D.inactivate();
			m_RBIQ.inactivate();
			if (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ))
			{
				update_outputs(8);
			}
			else if (!INPLOGIC(m_BIQ))
			{
				update_outputs(15);
			}
		} else {
			m_RBIQ.activate();
			m_D.activate();
			m_C.activate();
			m_B.activate();
			m_A.activate();
			UINT8 v;

			v = (INPLOGIC(m_A) << 0) | (INPLOGIC(m_B) << 1) | (INPLOGIC(m_C) << 2) | (INPLOGIC(m_D) << 3);
			if ((!INPLOGIC(m_RBIQ) && (v==0)))
					v = 15;
			update_outputs(v);
		}
	}

	NETLIB_RESET(7448)
	{
		m_state = 0;
		m_A.inactivate();
		m_B.inactivate();
		m_C.inactivate();
		m_D.inactivate();
		m_RBIQ.inactivate();
	}

	NETLIB_FUNC_VOID(7448, update_outputs, (UINT8 v))
	{
		nl_assert(v<16);
		if (v != m_state)
		{
			// max transfer time is 100 NS */

			for (int i=0; i<7; i++)
				OUTLOGIC(m_Q[i], tab7448[v][i], NLTIME_FROM_NS(100));
			m_state = v;
		}
	}

	const UINT8 NETLIB_NAME(7448)::tab7448[16][7] =
	{
			{   1, 1, 1, 1, 1, 1, 0 },  /* 00 - not blanked ! */
			{   0, 1, 1, 0, 0, 0, 0 },  /* 01 */
			{   1, 1, 0, 1, 1, 0, 1 },  /* 02 */
			{   1, 1, 1, 1, 0, 0, 1 },  /* 03 */
			{   0, 1, 1, 0, 0, 1, 1 },  /* 04 */
			{   1, 0, 1, 1, 0, 1, 1 },  /* 05 */
			{   0, 0, 1, 1, 1, 1, 1 },  /* 06 */
			{   1, 1, 1, 0, 0, 0, 0 },  /* 07 */
			{   1, 1, 1, 1, 1, 1, 1 },  /* 08 */
			{   1, 1, 1, 0, 0, 1, 1 },  /* 09 */
			{   0, 0, 0, 1, 1, 0, 1 },  /* 10 */
			{   0, 0, 1, 1, 0, 0, 1 },  /* 11 */
			{   0, 1, 0, 0, 0, 1, 1 },  /* 12 */
			{   1, 0, 0, 1, 0, 1, 1 },  /* 13 */
			{   0, 0, 0, 1, 1, 1, 1 },  /* 14 */
			{   0, 0, 0, 0, 0, 0, 0 },  /* 15 */
	};
	#endif

	NETLIB_DEVICE_IMPL(7448)
	NETLIB_DEVICE_IMPL(7448_dip)


	} //namespace devices
} // namespace netlist
