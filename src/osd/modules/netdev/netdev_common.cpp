// license:BSD-3-Clause
// copyright-holders:Carl

#include "netdev_common.h"


namespace osd {

network_device_base::network_device_base(network_handler &handler)
	: m_handler(handler)
	, m_stopped(true)
{
}

network_device_base::~network_device_base()
{
}

void network_device_base::start()
{
	m_stopped = false;
}

void network_device_base::stop()
{
	m_stopped = true;
}

void network_device_base::poll()
{
	uint8_t *buf;
	int len;
	while (!m_stopped && (len = recv_dev(&buf)))
	{
		m_handler.recv_cb(buf, len);
	}
}

} // namespace osd
