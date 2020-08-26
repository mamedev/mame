// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_4020_H_
#define NLD_4020_H_

#include "../nl_setup.h"

// usage       : CD4020(name)
#define CD4020(...)                                                    \
	NET_REGISTER_DEVEXT(CD4020, __VA_ARGS__)

// usage       : CD4024(name)
#define CD4024(...)                                                    \
	NET_REGISTER_DEVEXT(CD4024, __VA_ARGS__)

#endif /* NLD_4020_H_ */
