// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74279.c
 *
 */

#include "nld_74279.h"
#include "nld_truthtable.h"

namespace netlist
{
	namespace devices
	{

		NETLIB_TRUTHTABLE(74279A, 3, 1, 0);
		NETLIB_TRUTHTABLE(74279B, 4, 1, 0);

		NETLIB_OBJECT(74279_dip)
		{
			NETLIB_CONSTRUCTOR(74279_dip)
			, m_1(*this, "1")
			, m_2(*this, "2")
			, m_3(*this, "3")
			, m_4(*this, "4")
			{
				register_subalias("1", m_1.m_I[2]);  //R
				register_subalias("2", m_1.m_I[0]);
				register_subalias("3", m_1.m_I[1]);
				register_subalias("4", m_1.m_Q[0]);

				register_subalias("5", m_2.m_I[1]);  //R
				register_subalias("6", m_2.m_I[0]);
				register_subalias("7", m_2.m_Q[0]);

				register_subalias("9", m_3.m_Q[0]);
				register_subalias("10", m_3.m_I[2]); //R
				register_subalias("11", m_3.m_I[0]);
				register_subalias("12", m_3.m_I[1]);

				register_subalias("13", m_4.m_Q[0]);
				register_subalias("14", m_4.m_I[1]); //R
				register_subalias("15", m_4.m_I[0]);

			}

			//NETLIB_RESETI();
			//NETLIB_UPDATEI();

		protected:
			NETLIB_SUB(74279B) m_1;
			NETLIB_SUB(74279A) m_2;
			NETLIB_SUB(74279B) m_3;
			NETLIB_SUB(74279A) m_4;
		};

		nld_74279A::truthtable_t nld_74279A::m_ttbl(3,1,0);
		nld_74279B::truthtable_t nld_74279B::m_ttbl(4,1,0);

		const char *nld_74279A::m_desc[] = {
				"S,R,_Q|Q",
				"0,X,X|1|22",
				"1,0,X|0|27",
				"1,1,0|0|27", //15
				"1,1,1|1|22",
				""
		};


		const char *nld_74279B::m_desc[] = {
				"S1,S2,R,_Q|Q",
				"0,X,X,X|1|22",
				"X,0,X,X|1|22",
				"1,1,0,X|0|27",
				"1,1,1,0|0|27", // 15
				"1,1,1,1|1|22",
				""
		};


		NETLIB_DEVICE_IMPL(74279_dip)

	} //namespace devices
} // namespace netlist
