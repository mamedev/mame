// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    nethandler.cpp

    OSD interface to virtual networking handlers

***************************************************************************/

#include "nethandler.h"

#include <algorithm>


namespace osd {

network_handler::network_handler() noexcept
	: m_promisc(false)
{
	std::fill(std::begin(m_mac), std::end(m_mac), 0);
}

} // namespace osd
