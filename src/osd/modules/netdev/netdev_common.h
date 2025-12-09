// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_OSD_NETDEV_NETDEV_COMMON_H
#define MAME_OSD_NETDEV_NETDEV_COMMON_H

#pragma once

#include "interface/nethandler.h"

#include <cstdint>


namespace osd {

class network_device_base : public network_device
{
public:
	virtual ~network_device_base();

	virtual void start() override;
	virtual void stop() override;
	virtual void poll() override;

protected:
	network_device_base(network_handler &handler);

	virtual int recv_dev(uint8_t **buf) = 0;

private:
	network_handler &m_handler;
	bool m_stopped;
};

} // namespace osd

#endif // MAME_OSD_NETDEV_NETDEV_COMMON_H
