// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_4013_H_
#define NLD_4013_H_

#include "netlist/nl_setup.h"

// usage: CD4013(name, cCLOCK, cDATA, cRESET, cSET)
#define CD4013(...)                                                            \
		NET_REGISTER_DEVEXT(CD4013, __VA_ARGS__)

#endif /* NLD_4013_H_ */
