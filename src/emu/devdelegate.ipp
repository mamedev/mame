// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    devdelegate.ipp

    Delegates that are late-bound to MAME devices.

***************************************************************************/

#ifndef MAME_EMU_DEVDELEGATE_IPP
#define MAME_EMU_DEVDELEGATE_IPP

#pragma once

#include "devdelegate.h"

#include "device.h"
#include "devfind.h"

#include <tuple>


namespace emu::detail {

template <class DeviceClass, bool Required>
inline device_delegate_helper::device_delegate_helper(device_finder<DeviceClass, Required> const &finder)
	: device_delegate_helper(finder.finder_target().first, finder.finder_tag())
{
}

template <class DeviceClass, bool Required>
inline void device_delegate_helper::set_tag(device_finder<DeviceClass, Required> const &finder)
{
	std::tie(m_base, m_tag) = finder.finder_target();
}

} // namespace emu::detail

#endif // MAME_EMU_DEVDELEGATE_IPP
