// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.h
 *
 *  DM7448: BCD to 7-Segment decoders/drivers
 *
 *           +--------------+
 *         B |1     ++    16| VCC
 *         C |2           15| f
 * LAMP TEST |3           14| g
 *    BI/RBQ |4    7448   13| a
 *       RBI |5           12| b
 *         D |6           11| c
 *         A |7           10| d
 *       GND |8            9| e
 *           +--------------+
 *
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7448_H_
#define NLD_7448_H_

#include "../nl_base.h"

#define TTL_7448(_name, _A0, _A1, _A2, _A3, _LTQ, _BIQ, _RBIQ)                      \
		NET_REGISTER_DEV(7448, _name)                                               \
		NET_CONNECT(_name, A, _A0)                                                  \
		NET_CONNECT(_name, B, _A1)                                                  \
		NET_CONNECT(_name, C, _A2)                                                  \
		NET_CONNECT(_name, D, _A3)                                                  \
		NET_CONNECT(_name, LTQ, _LTQ)                                               \
		NET_CONNECT(_name, BIQ, _BIQ)                                               \
		NET_CONNECT(_name, RBIQ, _RBIQ)

#define TTL_7448_DIP(_name)                                                         \
		NET_REGISTER_DEV(7448_dip, _name)

NETLIB_SUBDEVICE(7448_sub,
	ATTR_HOT void update_outputs(UINT8 v);
	static const UINT8 tab7448[16][7];

	netlist_ttl_input_t m_A;
	netlist_ttl_input_t m_B;
	netlist_ttl_input_t m_C;
	netlist_ttl_input_t m_D;
	netlist_ttl_input_t m_RBIQ;

	netlist_state_t<UINT8> m_state;

	netlist_ttl_output_t m_a;
	netlist_ttl_output_t m_b;
	netlist_ttl_output_t m_c;
	netlist_ttl_output_t m_d;
	netlist_ttl_output_t m_e;
	netlist_ttl_output_t m_f;
	netlist_ttl_output_t m_g;
);

NETLIB_DEVICE(7448,
public:
	NETLIB_NAME(7448_sub) sub;

	netlist_ttl_input_t m_LTQ;
	netlist_ttl_input_t m_BIQ;
);

NETLIB_DEVICE_DERIVED(7448_dip, 7448,
);

#endif /* NLD_7448_H_ */
