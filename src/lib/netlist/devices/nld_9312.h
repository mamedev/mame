// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9312.h
 *
 *  DM9312: One of Eight Line Data Selectors/Multiplexers
 *
 *          +--------------+
 *       D0 |1     ++    16| VCC
 *       D1 |2           15| Y
 *       D2 |3           14| YQ
 *       D3 |4    9312   13| C
 *       D4 |5           12| B
 *       D5 |6           11| A
 *       D6 |7           10| G   Strobe
 *      GND |8            9| D7
 *          +--------------+
 *                  __
 *          +---+---+---+---++---+---+
 *          | C | B | A | G || Y | YQ|
 *          +===+===+===+===++===+===+
 *          | X | X | X | 1 ||  0| 1 |
 *          | 0 | 0 | 0 | 0 || D0|D0Q|
 *          | 0 | 0 | 1 | 0 || D1|D1Q|
 *          | 0 | 1 | 0 | 0 || D2|D2Q|
 *          | 0 | 1 | 1 | 0 || D3|D3Q|
 *          | 1 | 0 | 0 | 0 || D4|D4Q|
 *          | 1 | 0 | 1 | 0 || D5|D5Q|
 *          | 1 | 1 | 0 | 0 || D6|D6Q|
 *          | 1 | 1 | 1 | 0 || D7|D7Q|
 *          +---+---+---+---++---+---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9312_H_
#define NLD_9312_H_

#include "nld_truthtable.h"

#define TTL_9312(name)                                                \
		NET_REGISTER_DEV(TTL_9312, name)

#define TTL_9312_DIP(name)                                            \
		NET_REGISTER_DEV(TTL_9312_DIP, name)

namespace netlist
{
	namespace devices
	{

#if (USE_TRUTHTABLE)
/* The truthtable implementation is a lot faster than
 * the carefully crafted code :-(
 */
NETLIB_TRUTHTABLE(9312, 12, 2, 0);
#else

NETLIB_DEVICE(9312,
public:
//      C, B, A, G,D0,D1,D2,D3,D4,D5,D6,D7| Y,YQ
	logic_input_t m_A;
	logic_input_t m_B;
	logic_input_t m_C;
	logic_input_t m_G;
	logic_input_t m_D[8];
	logic_output_t m_Y;
	logic_output_t m_YQ;

	UINT8 m_last_chan;
	UINT8 m_last_G;
);

#endif

NETLIB_OBJECT(9312_dip)
{
	NETLIB_CONSTRUCTOR(9312_dip)
	, m_sub(*this, "1")
	{
	#if (1 && USE_TRUTHTABLE)

		register_subalias("13", m_sub.m_I[0]);
		register_subalias("12", m_sub.m_I[1]);
		register_subalias("11", m_sub.m_I[2]);
		register_subalias("10", m_sub.m_I[3]);

		register_subalias("1", m_sub.m_I[4]);
		register_subalias("2", m_sub.m_I[5]);
		register_subalias("3", m_sub.m_I[6]);
		register_subalias("4", m_sub.m_I[7]);
		register_subalias("5", m_sub.m_I[8]);
		register_subalias("6", m_sub.m_I[9]);
		register_subalias("7", m_sub.m_I[10]);
		register_subalias("9", m_sub.m_I[11]);

		register_subalias("15", m_sub.m_Q[0]); // Y
		register_subalias("14", m_sub.m_Q[1]); // YQ

	#else

		register_subalias("13", m_sub.m_C);
		register_subalias("12", m_sub.m_B);
		register_subalias("11", m_sub.m_A);
		register_subalias("10", m_sub.m_G);

		register_subalias("1", m_sub.m_D[0]);
		register_subalias("2", m_sub.m_D[1]);
		register_subalias("3", m_sub.m_D[2]);
		register_subalias("4", m_sub.m_D[3]);
		register_subalias("5", m_sub.m_D[4]);
		register_subalias("6", m_sub.m_D[5]);
		register_subalias("7", m_sub.m_D[6]);
		register_subalias("9", m_sub.m_D[7]);

		register_subalias("15", m_sub.m_Y); // Y
		register_subalias("14", m_sub.m_YQ); // YQ

	#endif

	}

	//NETLIB_RESETI();
	//NETLIB_UPDATEI();

protected:
	NETLIB_SUB(9312) m_sub;
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_9312_H_ */
