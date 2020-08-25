// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef NLD_74164_H_
#define NLD_74164_H_

#include "../nl_setup.h"

// usage: TTL_74164(name, cA, cB, cCLRQ, cCLK)
#define TTL_74164(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74164, __VA_ARGS__)

#endif /* NLD_74164_H_ */
