// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_9316_H_
#define NLD_9316_H_

#include "../nl_setup.h"

// usage: TTL_9316(name, cCLK, cENP, cENT, cCLRQ, cLOADQ, cA, cB, cC, cD)
#define TTL_9316(...)                                                          \
		NET_REGISTER_DEVEXT(TTL_74161, __VA_ARGS__)

#define TTL_74161(...)                                                         \
		NET_REGISTER_DEVEXT(TTL_74161, __VA_ARGS__)

#define TTL_74161_FIXME(...)                                                   \
		NET_REGISTER_DEVEXT(TTL_74161_FIXME, __VA_ARGS__)

#define TTL_74163(...)                                                         \
		NET_REGISTER_DEVEXT(TTL_74163, __VA_ARGS__)

#define TTL_9310(...)                                                          \
		NET_REGISTER_DEVEXT(TTL_9310, __VA_ARGS__)

#endif /* NLD_9316_H_ */
