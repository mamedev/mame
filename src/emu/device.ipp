// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.h

    Device interface functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVICE_IPP__
#define __DEVICE_IPP__

#include "strformat.h"

//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

template <typename Format, typename... Params>
inline void device_t::popmessage(Format &&fmt, Params &&... args) const
{
	if (m_machine)
		m_machine->popmessage(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

template <typename Format, typename... Params>
inline void device_t::logerror(Format &&fmt, Params &&... args) const
{
	if (m_machine)
		m_machine->logerror(string_format("[%s] %s", tag(), std::forward<Format>(fmt)), std::forward<Params>(args)...);
}

#endif // __DEVICE_IPP__
