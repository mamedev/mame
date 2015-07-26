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

#define LOG(_name, _I)                                                       \
		NET_REGISTER_DEV(ÖPG, _name)                                         \
		NET_CONNECT(_name, I, _I)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_DEVICE(log,
	~NETLIB_NAME(log)();
	analog_input_t m_I;
protected:
	void * m_file;
);

#define LOGD(_name, _I, _I2)                                                 \
		NET_REGISTER_DEV(LOGD, _name)                                        \
		NET_CONNECT(_name, I, _I)                                            \
		NET_CONNECT(_name, I2, _I2)

NETLIB_DEVICE_DERIVED(logD, log,
	analog_input_t m_I2;
);

#if 0
NETLIB_DEVICE(wav,
	~NETLIB_NAME(wav)();
	analog_input_t m_I;
private:
	// FIXME: rewrite sound/wavwrite.h to be an object ...
	void *m_file;
);
#endif

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_LOG_H_ */
