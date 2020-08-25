// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74125.h
 *
 */

#ifndef NLD_74125_H_
#define NLD_74125_H_

#include "../nl_setup.h"

#define TTL_74125_GATE(...)                                                    \
		NET_REGISTER_DEV(TTL_74125_GATE, __VA_ARGS__)

#define TTL_74126_GATE(...)                                                    \
		NET_REGISTER_DEV(TTL_74126_GATE, __VA_ARGS__)

#endif /* NLD_74125_H_ */
