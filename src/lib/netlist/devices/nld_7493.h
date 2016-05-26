// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.h
 *
 *  DM7493: Binary Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7493   11| QD
 *      VCC |5           10| GND
 *       NC |6            9| QB
 *       NC |7            8| QC
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+
 *          | COUNT || QD | QC | QB | QA |
 *          +=======++====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |
 *          |    2  ||  0 |  0 |  1 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |
 *          |    4  ||  0 |  1 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |
 *          |    6  ||  0 |  1 |  1 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |
 *          |    8  ||  1 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |
 *          |   10  ||  1 |  0 |  1 |  0 |
 *          |   11  ||  1 |  0 |  1 |  1 |
 *          |   12  ||  1 |  1 |  0 |  0 |
 *          |   13  ||  1 |  1 |  0 |  1 |
 *          |   14  ||  1 |  1 |  1 |  0 |
 *          |   15  ||  1 |  1 |  1 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note C Output QA is connected to input B
 *
 *          Reset Count Function table
 *
 *          +-----+-----++----+----+----+----+
 *          | R01 | R02 || QD | QC | QB | QA |
 *          +=====+=====++====+====+====+====+
 *          |  1  |  1  ||  0 |  0 |  0 |  0 |
 *          |  0  |  X  ||       COUNT       |
 *          |  X  |  0  ||       COUNT       |
 *          +-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7493_H_
#define NLD_7493_H_

#include "nl_base.h"

#define TTL_7493(name, cCLKA, cCLKB, cR1, cR2)                                     \
		NET_REGISTER_DEV(TTL_7493, name)                                               \
		NET_CONNECT(name, CLKA, cCLKA)                                             \
		NET_CONNECT(name, CLKB, cCLKB)                                             \
		NET_CONNECT(name, R1,  cR1)                                                \
		NET_CONNECT(name, R2,  cR2)

#define TTL_7493_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_7493_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(7493ff)
{
	NETLIB_CONSTRUCTOR(7493ff)
	{
		enregister("CLK", m_I);
		enregister("Q", m_Q);

		save(NLNAME(m_reset));
		save(NLNAME(m_state));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	logic_input_t m_I;
	logic_output_t m_Q;

	UINT8 m_reset;
	UINT8 m_state;
};

NETLIB_OBJECT(7493)
{
	NETLIB_CONSTRUCTOR(7493)
	, A(*this, "A")
	, B(*this, "B")
	, C(*this, "C")
	, D(*this, "D")
	{
		register_subalias("CLKA", A.m_I);
		register_subalias("CLKB", B.m_I);
		enregister("R1",  m_R1);
		enregister("R2",  m_R2);

		register_subalias("QA", A.m_Q);
		register_subalias("QB", B.m_Q);
		register_subalias("QC", C.m_Q);
		register_subalias("QD", D.m_Q);

		connect_late(C.m_I, B.m_Q);
		connect_late(D.m_I, C.m_Q);
	}

	NETLIB_RESETI() { }
	NETLIB_UPDATEI();

	logic_input_t m_R1;
	logic_input_t m_R2;

	NETLIB_SUB(7493ff) A;
	NETLIB_SUB(7493ff) B;
	NETLIB_SUB(7493ff) C;
	NETLIB_SUB(7493ff) D;
};

NETLIB_OBJECT_DERIVED(7493_dip, 7493)
{
	NETLIB_CONSTRUCTOR_DERIVED(7493_dip, 7493)
	{
		register_subalias("1", B.m_I);
		register_subalias("2", m_R1);
		register_subalias("3", m_R2);

		// register_subalias("4", ); --> NC
		// register_subalias("5", ); --> VCC
		// register_subalias("6", ); --> NC
		// register_subalias("7", ); --> NC

		register_subalias("8", C.m_Q);
		register_subalias("9", B.m_Q);
		// register_subalias("10", ); -. GND
		register_subalias("11", D.m_Q);
		register_subalias("12", A.m_Q);
		// register_subalias("13", ); -. NC
		register_subalias("14", A.m_I);
	}
};

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_7493_H_ */
