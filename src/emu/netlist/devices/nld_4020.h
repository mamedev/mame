// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.h
 *
 *  CD4020: CMOS Ripple-Carry Binary Counters/Dividers
 *
 *          +--------------+
 *      Q12 |1     ++    16| VDD
 *      Q13 |2           15| Q11
 *      Q14 |3           14| Q10
 *       Q6 |4    4020   13| Q8
 *       Q5 |5           12| Q9
 *       Q7 |6           11| RESET
 *       Q4 |7           10| IP (Input pulses)
 *      VSS |8            9| Q1
 *          +--------------+
 *
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *         This needs a cmos d-a/a-d proxy implementation.
 *
 */

#ifndef NLD_4020_H_
#define NLD_4020_H_

#include "../nl_base.h"
#include "nld_cmos.h"

#define CD_4020(_name, _IP, _RESET, _VDD, _VSS)                                \
		NET_REGISTER_DEV(4020, _name)                                          \
		NET_CONNECT(_name, IP, _IP)                                            \
		NET_CONNECT(_name, RESET,  _RESET)                                     \
		NET_CONNECT(_name, VDD,  _VDD)                                         \
		NET_CONNECT(_name, VSS,  _VSS)

#define CD_4020_DIP(_name)                                                     \
		NET_REGISTER_DEV(4020_dip, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(4020_sub,

	NETLIB_LOGIC_FAMILY(CD4000)
	ATTR_HOT void update_outputs(const UINT16 cnt);

	logic_input_t m_IP;

	UINT16 m_cnt;

	logic_output_t m_Q[14];
);

NETLIB_DEVICE(4020,
	NETLIB_LOGIC_FAMILY(CD4000)
	NETLIB_NAME(4020_sub) sub;
	NETLIB_NAME(vdd_vss) m_supply;
	logic_input_t m_RESET;
);

NETLIB_DEVICE_DERIVED_PURE(4020_dip, 4020);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_4020_H_ */
