// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    nethandler.h

    OSD interface to virtual networking handlers

***************************************************************************/
#ifndef MAME_OSD_INTERFACE_NETHANDLER_H
#define MAME_OSD_INTERFACE_NETHANDLER_H

#pragma once

#include "osdcomm.h"

#include <array>
#include <string_view>


namespace osd {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// base for virtual network interface handler

class network_handler
{
public:
	network_handler() noexcept;

	virtual void recv_cb(u8 *buf, int len) = 0;

	std::array<u8, 6> const &get_mac() noexcept { return m_mac; }

protected:
	~network_handler() = default;

	std::array<u8, 6> m_mac;
};


// interface to network device

class network_device
{
public:
	virtual ~network_device() = default;

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void poll() = 0;
	virtual int send(const void *buf, int len) = 0;
};


// description of an available network device

struct network_device_info
{
	int id;
	std::string_view description;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_NETHANDLER_H
