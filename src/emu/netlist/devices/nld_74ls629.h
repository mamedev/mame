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

#include "../nl_base.h"
#include "../analog/nld_twoterm.h"

#define SN74LS629(_name, _cap)                                                      \
		NET_REGISTER_DEV(SN74LS629, _name)                                          \
		NETDEV_PARAMI(_name, CAP, _cap)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(SN74LS629clk,
	logic_input_t m_FB;
	logic_output_t m_Y;

	netlist_time m_inc;
	netlist_sig_t m_enableq;
	netlist_sig_t m_out;
);

NETLIB_DEVICE_WITH_PARAMS(SN74LS629,
public:
	NETLIB_NAME(SN74LS629clk) m_clock;
	NETLIB_NAME(R_base) m_R_FC;
	NETLIB_NAME(R_base) m_R_RNG;

	logic_input_t m_ENQ;
	analog_input_t m_RNG;
	analog_input_t m_FC;

	param_double_t m_CAP;
);

#define SN74LS629_DIP(_name, _cap1, _cap2)                                        \
		NET_REGISTER_DEV(SN74LS629_DIP, _name)                                    \
		NETDEV_PARAMI(_name, 1.CAP, _cap1)                                        \
		NETDEV_PARAMI(_name, 2.CAP, _cap2)

NETLIB_DEVICE(SN74LS629_dip,
	NETLIB_NAME(SN74LS629) m_1;
	NETLIB_NAME(SN74LS629) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_74LS629_H_ */
