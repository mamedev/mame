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

#include <memory>
#include "nl_base.h"
#include "plib/pstream.h"
#include "plib/pfmtlog.h"

#define LOG(name, cI)                                                        \
		NET_REGISTER_DEV(??PG, name)                                         \
		NET_CONNECT(name, I, cI)

#define LOGD(name, cI, cI2)                                                 \
		NET_REGISTER_DEV(LOGD, name)                                        \
		NET_CONNECT(name, I, cI)                                            \
		NET_CONNECT(name, I2, cI2)


NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(log)
{
	NETLIB_CONSTRUCTOR(log)
	{
		enregister("I", m_I);

		pstring filename = plib::pfmt("{1}.log")(this->name());
		m_strm = plib::make_unique<plib::pofilestream>(filename);
	}
	NETLIB_DESTRUCTOR(log);
	NETLIB_UPDATEI();
	NETLIB_RESETI() { }
protected:
	analog_input_t m_I;
	std::unique_ptr<plib::pofilestream> m_strm;
};

NETLIB_OBJECT_DERIVED(logD, log)
{
	NETLIB_CONSTRUCTOR_DERIVED(logD, log)
	{
		enregister("I2", m_I2);
	}
	NETLIB_UPDATEI();
	NETLIB_RESETI() { };
	analog_input_t m_I2;
};

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
