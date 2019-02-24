// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_NE555_H_
#define NLD_NE555_H_

#include "netlist/nl_setup.h"

#define NE555(name)                                                             \
		NET_REGISTER_DEV(NE555, name)

#define NE555_DIP(name)                                                         \
		NET_REGISTER_DEV(NE555_DIP, name)

#endif /* NLD_NE555_H_ */
