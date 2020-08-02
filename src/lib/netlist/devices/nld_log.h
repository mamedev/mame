// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_LOG_H_
#define NLD_LOG_H_

#include "netlist/nl_setup.h"

#define LOG(name, cI)                                                        \
		NET_REGISTER_DEV(LOG, name)                                         \
		NET_CONNECT(name, I, cI)

#define LOGD(name, cI, cI2)                                                 \
		NET_REGISTER_DEV(LOGD, name)                                        \
		NET_CONNECT(name, I, cI)                                            \
		NET_CONNECT(name, I2, cI2)

#endif /* NLD_LOG_H_ */
