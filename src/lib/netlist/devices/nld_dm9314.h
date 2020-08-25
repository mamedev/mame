// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

#ifndef NLD_DM9314_H_
#define NLD_DM9314_H_

#include "../nl_setup.h"

// usage       : TTL_9314(name, pEQ, pMRQ, pS0Q, pS1Q, pS2Q, pS3Q, pD0, pD1, pD2, pD3)
// auto connect: VCC, GND
#define TTL_9314(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9314, __VA_ARGS__)

#endif /* NLD_DM9314_H_ */
