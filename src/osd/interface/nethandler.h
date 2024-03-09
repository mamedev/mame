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
	bool get_promisc() noexcept { return m_promisc; }

protected:
	~network_handler() = default;

	bool m_promisc;
	std::array<u8, 6> m_mac;
};

} // namespace osd

#endif // MAME_OSD_INTERFACE_NETHANDLER_H
