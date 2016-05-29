// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_log.h
 *
 *  Devices supporting analysis and logging
 *
 *  nld_log:
 *
 *          +---------+
 *          |    ++   |
 *        I |         | ==> Log to file "netlist_" + name() + ".log"
 *          |         |
 *          +---------+
 *
 */

#ifndef NLD_LOG_H_
#define NLD_LOG_H_

#include "nl_setup.h"

#define LOG(name, cI)                                                        \
		NET_REGISTER_DEV(??PG, name)                                         \
		NET_CONNECT(name, I, cI)

#define LOGD(name, cI, cI2)                                                 \
		NET_REGISTER_DEV(LOGD, name)                                        \
		NET_CONNECT(name, I, cI)                                            \
		NET_CONNECT(name, I2, cI2)

#endif /* NLD_LOG_H_ */
