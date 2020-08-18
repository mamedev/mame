// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74LS629_H_
#define NLD_74LS629_H_

#include "netlist/nl_setup.h"

#define SN74LS629(name, ...)                                                  \
		NET_REGISTER_DEVEXT(SN74LS629, name,__VA_ARGS__)

#endif /* NLD_74LS629_H_ */
