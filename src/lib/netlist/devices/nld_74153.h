// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74153.h
 *
 *  DM74153: Dual 4-Line to 1-Line Data Selectors Multiplexers
 *
 *          +--------------+
 *       G1 |1     ++    16| VCC
 *        B |2           15| G2
 *      1C3 |3           14| A
 *      1C2 |4   74153   13| 2C3
 *      1C1 |5           12| 2C2
 *      1C0 |6           11| 2C1
 *       Y1 |7           10| 2C0
 *      GND |8            9| Y2
 *          +--------------+
 *
 *
 *          Function table
 *
 *          +-----+-----++----+----+----+----++----+----+
 *          |  B  |  A  || C0 | C1 | C2 | C3 ||  G |  Y |
 *          +=====+=====++====+====+====+====++====+====+
 *          |  X  |  X  ||  X |  X |  X |  X ||  H |  L |
 *          |  L  |  L  ||  L |  X |  X |  X ||  L |  L |
 *          |  L  |  L  ||  H |  X |  X |  X ||  L |  H |
 *          |  L  |  H  ||  X |  L |  X |  X ||  L |  L |
 *          |  L  |  H  ||  X |  H |  X |  X ||  L |  H |
 *          |  H  |  L  ||  X |  X |  L |  X ||  L |  L |
 *          |  H  |  L  ||  X |  X |  H |  X ||  L |  H |
 *          |  H  |  H  ||  X |  X |  X |  L ||  L |  L |
 *          |  H  |  H  ||  X |  X |  X |  H ||  L |  H |
 *          +-----+-----++----+----+----+----++----+----+
 *
 *  A, B : Select Inputs
 *  C*   : Data inputs
 *  G    : Strobe
 *  Y    : Output
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74153_H_
#define NLD_74153_H_

#include "nl_base.h"

#define TTL_74153(_name, _C0, _C1, _C2, _C3, _A, _B, _G)                            \
		NET_REGISTER_DEV(TTL_74153, _name)                                              \
		NET_CONNECT(_name, C0, _C0)                                                 \
		NET_CONNECT(_name, C1, _C1)                                                 \
		NET_CONNECT(_name, C2, _C2)                                                 \
		NET_CONNECT(_name, C3, _C3)                                                 \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)                                                   \
		NET_CONNECT(_name, G, _G)

#define TTL_74153_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_74153_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(74153sub)
{
	NETLIB_CONSTRUCTOR(74153sub)
	, m_chan(0)
	{
		enregister("C0", m_C[0]);
		enregister("C1", m_C[1]);
		enregister("C2", m_C[2]);
		enregister("C3", m_C[3]);
		enregister("G", m_G);

		enregister("AY", m_Y); //FIXME: Change netlists

		save(NLNAME(m_chan));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	logic_input_t m_C[4];
	logic_input_t m_G;

	logic_output_t m_Y;

	int m_chan;
};

NETLIB_OBJECT(74153)
{
	NETLIB_CONSTRUCTOR(74153)
	, m_sub(*this, "sub")
	{

		register_subalias("C0", m_sub.m_C[0]);
		register_subalias("C1",  m_sub.m_C[1]);
		register_subalias("C2",  m_sub.m_C[2]);
		register_subalias("C3",  m_sub.m_C[3]);
		enregister("A", m_A);
		enregister("B", m_B);
		register_subalias("G",  m_sub.m_G);

		register_subalias("AY",  m_sub.m_Y); //FIXME: Change netlists
	}
	NETLIB_RESETI() { }
	NETLIB_UPDATEI();
public:
	NETLIB_SUB(74153sub) m_sub;
	logic_input_t m_A;
	logic_input_t m_B;
};

NETLIB_OBJECT(74153_dip)
{
	NETLIB_CONSTRUCTOR(74153_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	{

		register_subalias("1", m_1.m_G);
		enregister("2", m_B);    // m_2.m_B
		register_subalias("3", m_1.m_C[3]);
		register_subalias("4", m_1.m_C[2]);
		register_subalias("5", m_1.m_C[1]);
		register_subalias("6", m_1.m_C[0]);
		register_subalias("7", m_1.m_Y);

		register_subalias("9", m_2.m_Y);
		register_subalias("10", m_2.m_C[0]);
		register_subalias("11", m_2.m_C[1]);
		register_subalias("12", m_2.m_C[2]);
		register_subalias("13", m_2.m_C[3]);

		enregister("14", m_A);   // m_2.m_B
		register_subalias("15", m_2.m_G);

	}
	//NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	NETLIB_SUB(74153sub) m_1;
	NETLIB_SUB(74153sub) m_2;
	logic_input_t m_A;
	logic_input_t m_B;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74153_H_ */
