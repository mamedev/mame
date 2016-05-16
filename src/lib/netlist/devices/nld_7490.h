// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7490.h
 *
 *  DM7490: Decade Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7490   11| QD
 *      VCC |5           10| GND
 *      R91 |6            9| QB
 *      R92 |7            8| QC
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
 *          +-------++----+----+----+----+
 *
 *          Note A Output QA is connected to input B for BCD count
 *
 *          Reset Count Function table
 *
 *          +-----+-----+-----+-----++----+----+----+----+
 *          | R01 | R02 | R91 | R92 || QD | QC | QB | QA |
 *          +=====+=====+=====+=====++====+====+====+====+
 *          |  1  |  1  |  0  |  X  ||  0 |  0 |  0 |  0 |
 *          |  1  |  1  |  X  |  0  ||  0 |  0 |  0 |  0 |
 *          |  X  |  X  |  1  |  1  ||  1 |  0 |  0 |  1 |
 *          |  X  |  0  |  X  |  0  ||       COUNT       |
 *          |  0  |  X  |  0  |  X  ||       COUNT       |
 *          |  0  |  X  |  X  |  0  ||       COUNT       |
 *          |  X  |  0  |  0  |  X  ||       COUNT       |
 *          +-----+-----+-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7490_H_
#define NLD_7490_H_

#include "nl_base.h"

#define TTL_7490(_name, _A, _B, _R1, _R2, _R91, _R92)                               \
		NET_REGISTER_DEV(TTL_7490, _name)                                               \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)                                                   \
		NET_CONNECT(_name, R1,  _R1)                                                \
		NET_CONNECT(_name, R2,  _R2)                                                \
		NET_CONNECT(_name, R91, _R91)                                               \
		NET_CONNECT(_name, R92, _R92)

#define TTL_7490_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7490_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(7490)
{
	NETLIB_CONSTRUCTOR(7490)
	, m_cnt(0)
	, m_last_A(0)
	, m_last_B(0)
	{
		enregister("A", m_A);
		enregister("B", m_B);
		enregister("R1",  m_R1);
		enregister("R2",  m_R2);
		enregister("R91", m_R91);
		enregister("R92", m_R92);

		enregister("QA", m_Q[0]);
		enregister("QB", m_Q[1]);
		enregister("QC", m_Q[2]);
		enregister("QD", m_Q[3]);

		save(NLNAME(m_cnt));
		save(NLNAME(m_last_A));
		save(NLNAME(m_last_B));
	}

	NETLIB_UPDATEI();
	NETLIB_RESETI();

protected:
	ATTR_HOT void update_outputs();

	logic_input_t m_R1;
	logic_input_t m_R2;
	logic_input_t m_R91;
	logic_input_t m_R92;
	logic_input_t m_A;
	logic_input_t m_B;

	UINT8 m_cnt;
	UINT8 m_last_A;
	UINT8 m_last_B;

	logic_output_t m_Q[4];
};

NETLIB_OBJECT_DERIVED(7490_dip, 7490)
{
	NETLIB_CONSTRUCTOR_DERIVED(7490_dip, 7490)
	{
		register_subalias("1", m_B);
		register_subalias("2", m_R1);
		register_subalias("3", m_R2);

		// register_subalias("4", ); --> NC
		// register_subalias("5", ); --> VCC
		register_subalias("6", m_R91);
		register_subalias("7", m_R92);

		register_subalias("8", m_Q[2]);
		register_subalias("9", m_Q[1]);
		// register_subalias("10", ); --> GND
		register_subalias("11", m_Q[3]);
		register_subalias("12", m_Q[0]);
		// register_subalias("13", ); --> NC
		register_subalias("14", m_A);
	}
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7490_H_ */
