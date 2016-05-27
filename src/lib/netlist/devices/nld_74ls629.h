// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74LS629.h
 *
 *  SN74LS629: VOLTAGE-CONTROLLED OSCILLATORS
 *
 *          +--------------+
 *      2FC |1     ++    16| VCC
 *      1FC |2           15| QSC VCC
 *     1RNG |3           14| 2RNG
 *     1CX1 |4  74LS629  13| 2CX1
 *     1CX2 |5           12| 2CX2
 *     1ENQ |6           11| 2ENQ
 *       1Y |7           10| 2Y
 *  OSC GND |8            9| GND
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  NOTE: The CX1 and CX2 pins are not connected!
 *        The capacitor value has to be specified as a parameter.
 *        There are more comments on the challenges of emulating this
 *        chip in the *.c file
 *
 */

#ifndef NLD_74LS629_H_
#define NLD_74LS629_H_

#include "nl_base.h"
#include "analog/nld_twoterm.h"

#define SN74LS629(name, p_cap)                                                      \
		NET_REGISTER_DEV(SN74LS629, name)                                          \
		NETDEV_PARAMI(name, CAP, p_cap)

#define SN74LS629_DIP(name, p_cap1, p_cap2)                                        \
		NET_REGISTER_DEV(SN74LS629_DIP, name)                                    \
		NETDEV_PARAMI(name, 1.CAP, p_cap1)                                        \
		NETDEV_PARAMI(name, 2.CAP, p_cap2)

namespace netlist
{
	namespace devices
	{

NETLIB_OBJECT(SN74LS629clk)
{
	NETLIB_CONSTRUCTOR(SN74LS629clk)
	, m_enableq(1)
	, m_out(0)
	, m_inc(netlist_time::zero)
	{
		enregister("FB",    m_FB);
		enregister("Y",    m_Y);

		connect_late(m_FB, m_Y);

		save(NLNAME(m_enableq));
		save(NLNAME(m_inc));
		save(NLNAME(m_out));
	}

	NETLIB_RESETI()
	{
		m_enableq = 1;
		m_out = 0;
		m_inc = netlist_time::zero;
	}

	NETLIB_UPDATEI();

public:
	logic_input_t m_FB;
	logic_output_t m_Y;

	netlist_sig_t m_enableq;
	netlist_sig_t m_out;
	netlist_time m_inc;
};

NETLIB_OBJECT(SN74LS629)
{
	NETLIB_CONSTRUCTOR(SN74LS629)
	, m_clock(*this, "OSC")
	, m_R_FC(*this, "R_FC")
	, m_R_RNG(*this, "R_RNG")
	, m_CAP(*this, "CAP", 1e-6)
	{
		enregister("ENQ", m_ENQ);
		enregister("RNG",    m_RNG);
		enregister("FC",     m_FC);
		register_subalias("GND",    m_R_FC.m_N);

		connect_late(m_FC, m_R_FC.m_P);
		connect_late(m_RNG, m_R_RNG.m_P);
		connect_late(m_R_FC.m_N, m_R_RNG.m_N);

		register_subalias("Y", m_clock.m_Y);
	}

	NETLIB_RESETI()
	{
		m_R_FC.set_R(90000.0);
		m_R_RNG.set_R(90000.0);
		m_clock.do_reset();
	}
	NETLIB_UPDATEI();

	NETLIB_UPDATE_PARAMI() { update_dev(); }

public:
	NETLIB_SUB(SN74LS629clk) m_clock;
	NETLIB_SUB(R_base) m_R_FC;
	NETLIB_SUB(R_base) m_R_RNG;

	logic_input_t m_ENQ;
	analog_input_t m_RNG;
	analog_input_t m_FC;

	param_double_t m_CAP;
};

NETLIB_OBJECT(SN74LS629_dip)
{
	NETLIB_CONSTRUCTOR(SN74LS629_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	{
		register_subalias("1",  m_2.m_FC);
		register_subalias("2",  m_1.m_FC);
		register_subalias("3",  m_1.m_RNG);

		register_subalias("6",  m_1.m_ENQ);
		register_subalias("7",  m_1.m_clock.m_Y);

		register_subalias("8",  m_1.m_R_FC.m_N);
		register_subalias("9",  m_1.m_R_FC.m_N);
		connect_late(m_1.m_R_FC.m_N, m_2.m_R_FC.m_N);

		register_subalias("10",  m_2.m_clock.m_Y);

		register_subalias("11",  m_2.m_ENQ);
		register_subalias("14",  m_2.m_RNG);
	}

	NETLIB_UPDATEI() { }

	NETLIB_RESETI()
	{
		m_1.do_reset();
		m_2.do_reset();
	}

private:
	NETLIB_SUB(SN74LS629) m_1;
	NETLIB_SUB(SN74LS629) m_2;
};

	} //namespace devices
} // namespace netlist


#endif /* NLD_74LS629_H_ */
