// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.h
 *
 *  DM7483: 4-Bit Binary Adder with Fast Carry
 *
 *          +--------------+
 *       A4 |1     ++    16| B4
 *       S3 |2           15| S4
 *       A3 |3           14| C4
 *       B3 |4    7483   13| C0
 *      VCC |5           12| GND
 *       S2 |6           11| B1
 *       B2 |7           10| A1
 *       A2 |8            9| S1
 *          +--------------+
 *
 *          S = (A + B + C0) & 0x0f
 *
 *          C4 = (A + B + C) > 15 ? 1 : 0
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_7483_H_
#define NLD_7483_H_

#include "nl_base.h"

#define TTL_7483(name, cA1, cA2, cA3, cA4, cB1, cB2, cB3, cB4, cCI)                \
		NET_REGISTER_DEV(TTL_7483, name)                                               \
		NET_CONNECT(name, A1, cA1)                                                 \
		NET_CONNECT(name, A2, cA2)                                                 \
		NET_CONNECT(name, A3, cA3)                                                 \
		NET_CONNECT(name, A4, cA4)                                                 \
		NET_CONNECT(name, B1, cB1)                                                 \
		NET_CONNECT(name, B2, cB2)                                                 \
		NET_CONNECT(name, B3, cB3)                                                 \
		NET_CONNECT(name, B4, cB4)                                                 \
		NET_CONNECT(name, C0, cCI)

#define TTL_7483_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_7483_DIP, name)

namespace netlist
{
	namespace devices
	{

NETLIB_OBJECT(7483)
{
	NETLIB_CONSTRUCTOR(7483)
	, m_lastr(0)
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

		enregister("S1", m_S1);
		enregister("S2", m_S2);
		enregister("S3", m_S3);
		enregister("S4", m_S4);
		enregister("C4", m_C4);

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

	} //namespace devices
} // namespace netlist

#endif /* NLD_7483_H_ */
