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

//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

template <typename Format, typename... Params>
inline void device_t::popmessage(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr)
		m_machine->popmessage(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

template <typename Format, typename... Params>
inline void device_t::logerror(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr && m_machine->allow_logging())
	{
		g_profiler.start(PROFILER_LOGERROR);

		// dump to the buffer
		m_string_buffer.clear();
		m_string_buffer.seekp(0);
		util::stream_format(m_string_buffer, "[%s] ", tag());
		util::stream_format(m_string_buffer, std::forward<Format>(fmt), std::forward<Params>(args)...);
		m_string_buffer.put('\0');

		m_machine->strlog(&m_string_buffer.vec()[0]);

		g_profiler.stop();
	}
}

#endif // __DEVICE_IPP__
