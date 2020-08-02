// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7448_H_
#define NLD_7448_H_

#include "netlist/nl_setup.h"

#if !NL_AUTO_DEVICES
#if !(NL_USE_TRUTHTABLE_7448)
// usage       : TTL_7448(name, pA, pB, pC, pD, pLTQ, pBIQ, pRBIQ)
// auto connect: VCC, GND
#define TTL_7448(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_7448, __VA_ARGS__)

#endif
#endif

#endif /* NLD_7448_H_ */
