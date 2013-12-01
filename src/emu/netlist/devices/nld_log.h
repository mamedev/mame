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

#include "../nl_base.h"

#define NETDEV_LOG(_name, _I)                                                       \
        NET_REGISTER_DEV(log, _name)                                                \
        NET_CONNECT(_name, I, _I)

NETLIB_DEVICE(log,
    ~NETLIB_NAME(log)();
    netlist_analog_input_t m_I;
private:
    FILE *m_file;
);

#endif /* NLD_LOG_H_ */
