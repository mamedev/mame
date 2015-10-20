// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4020.h
 *
 *  CD4020: 14-Stage Ripple Carry Binary Counters
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

#include "nl_base.h"
#include "nld_cmos.h"

/* FIXME: only used in mario.c */
#define CD4020_WI(_name, _IP, _RESET, _VDD, _VSS)                              \
		NET_REGISTER_DEV(CD4020_WI, _name)                                        \
		NET_CONNECT(_name, IP, _IP)                                            \
		NET_CONNECT(_name, RESET,  _RESET)                                     \
		NET_CONNECT(_name, VDD,  _VDD)                                         \
		NET_CONNECT(_name, VSS,  _VSS)

#define CD4020(_name)                                                          \
		NET_REGISTER_DEV(CD4020, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(CD4020_sub,

	NETLIB_LOGIC_FAMILY(CD4XXX)
	ATTR_HOT void update_outputs(const UINT16 cnt);

	logic_input_t m_IP;

	UINT16 m_cnt;

	logic_output_t m_Q[14];
);

NETLIB_DEVICE(CD4020,
	NETLIB_LOGIC_FAMILY(CD4XXX)
	NETLIB_NAME(CD4020_sub) sub;
	NETLIB_NAME(vdd_vss) m_supply;
	logic_input_t m_RESET;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_4020_H_ */
