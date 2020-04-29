// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_NE566_H_
#define NLD_NE566_H_

#include "netlist/nl_setup.h"

#define NE566(name)                                                             \
		NET_REGISTER_DEV(NE566, name)

#define NE566_DIP(name)                                                         \
		NET_REGISTER_DEV(NE566_DIP, name)

#endif /* NLD_NE566_H_ */
