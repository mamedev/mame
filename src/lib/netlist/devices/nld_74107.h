// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74107_H_
#define NLD_74107_H_

#include "../nl_setup.h"

#define TTL_74107(...)                                                         \
		NET_REGISTER_DEVEXT(TTL_74107, __VA_ARGS__)

// usage: TTL_74107A(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74107A(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74107A, __VA_ARGS__)

#endif /* NLD_74107_H_ */
