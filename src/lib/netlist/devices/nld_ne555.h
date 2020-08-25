// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_NE555_H_
#define NLD_NE555_H_

#include "../nl_setup.h"

// usage       : NE555(name)
#define NE555(...)                                                     \
	NET_REGISTER_DEVEXT(NE555, __VA_ARGS__)

/// usage       : MC1455P(name)
#define MC1455P(...)                                                   \
	NET_REGISTER_DEVEXT(MC1455P, __VA_ARGS__)

#endif /* NLD_NE555_H_ */
