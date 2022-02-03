// license:CC0
// copyright-holders:Couriersud

#include "devices/net_lib.h"

NETLIST_START(RTEST)

	RES(R1, RES_K(10))
	ALIAS(1, R1.1)
	ALIAS(2, R1.2)

NETLIST_END()
