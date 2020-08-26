// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74113_H_
#define NLD_74113_H_

#include "../nl_setup.h"

// usage: TTL_74113(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74113(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

// usage: TTL_74113A(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74113A(...)                                                       \
		NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

#endif /* NLD_74113_H_ */
