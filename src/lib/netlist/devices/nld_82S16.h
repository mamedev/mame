// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_82S16.h
 *
 *  DM82S16: 256 Bit bipolar ram
 *
 *          +--------------+
 *       A1 |1     ++    16| VCC
 *       A0 |2           15| A2
 *     CE1Q |3           14| A3
 *     CE2Q |4   82S16   13| DIN
 *     CE3Q |5           12| WEQ
 *    DOUTQ |6           11| A7
 *       A4 |7           10| A6
 *      GND |8            9| A5
 *          +--------------+
 *
 *
 *  Naming conventions follow Signetics datasheet
 *
 */

#ifndef NLD_82S16_H_
#define NLD_82S16_H_

#include "nl_base.h"

#define TTL_82S16(_name)                                     \
		NET_REGISTER_DEV(TTL_82S16, _name)
#define TTL_82S16_DIP(_name)                                 \
		NET_REGISTER_DEV(TTL_82S16_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(82S16)
{
	NETLIB_CONSTRUCTOR(82S16)
	{
		enregister("A0",    m_A[0]);
		enregister("A1",    m_A[1]);
		enregister("A2",    m_A[2]);
		enregister("A3",    m_A[3]);
		enregister("A4",    m_A[4]);
		enregister("A5",    m_A[5]);
		enregister("A6",    m_A[6]);
		enregister("A7",    m_A[7]);

		enregister("CE1Q",  m_CE1Q);
		enregister("CE2Q",  m_CE2Q);
		enregister("CE3Q",  m_CE3Q);

		enregister("WEQ",   m_WEQ);
		enregister("DIN",   m_DIN);

		enregister("DOUTQ",m_DOUTQ);

		save(NLNAME(m_ram));

	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	logic_input_t m_A[8];
	logic_input_t m_CE1Q;
	logic_input_t m_CE2Q;
	logic_input_t m_CE3Q;
	logic_input_t m_WEQ;
	logic_input_t m_DIN;
	logic_output_t m_DOUTQ;

	//netlist_state_t<UINT8[256]> m_ram;
	UINT64 m_ram[4]; // 256 bits
};

NETLIB_OBJECT_DERIVED(82S16_dip, 82S16)
{
	NETLIB_CONSTRUCTOR_DERIVED(82S16_dip, 82S16)
	{
		enregister("2",     m_A[0]);
		enregister("1",     m_A[1]);
		enregister("15",    m_A[2]);
		enregister("14",    m_A[3]);
		enregister("7",     m_A[4]);
		enregister("9",     m_A[5]);
		enregister("10",    m_A[6]);
		enregister("11",    m_A[7]);

		enregister("3",     m_CE1Q);
		enregister("4",     m_CE2Q);
		enregister("5",     m_CE3Q);

		enregister("12",    m_WEQ);
		enregister("13",    m_DIN);

		enregister("6",    m_DOUTQ);
	}
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_82S16_H_ */
