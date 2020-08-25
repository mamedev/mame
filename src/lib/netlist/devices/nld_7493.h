// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7493_H_
#define NLD_7493_H_

#include "../nl_setup.h"

// usage: TTL_7493(name, cCLKA, cCLKB, cR1, cR2)
#define TTL_7493(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7493, __VA_ARGS__)

#endif /* NLD_7493_H_ */
