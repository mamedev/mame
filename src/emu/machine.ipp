// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    machine.ipp

    Controls execution of the core MAME system.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MACHINE_IPP__
#define __MACHINE_IPP__

#include "strformat.h"

//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

/*-------------------------------------------------
    popmessage - pop up a user-visible message
-------------------------------------------------*/

template <typename Format, typename... Params>
inline void running_machine::popmessage(Format &&fmt, Params &&... args) const
{
	if (is_null<Format>::value(fmt))
	{
		// if the format is NULL, it is a signal to clear the popmessage
		ui().popup_time(0, " ");
	}
	else
	{
		// otherwise, generate the buffer and call the UI to display the message
		std::string const temp(string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));

		// pop it in the UI
		ui().popup_time(temp.length() / 40 + 2, "%s", temp.c_str());
	}
}


/*-------------------------------------------------
    logerror - log to the debugger and any other
    OSD-defined output streams
-------------------------------------------------*/

template <typename Format, typename... Params>
inline void running_machine::logerror(Format &&fmt, Params &&... args) const
{
	// process only if there is a target
	if (!m_logerror_list.empty())
	{
		g_profiler.start(PROFILER_LOGERROR);

		// dump to the buffer
		m_string_buffer.clear();
		m_string_buffer.seekp(0);
		util::stream_format(m_string_buffer, std::forward<Format>(fmt), std::forward<Params>(args)...);
		m_string_buffer.put('\0');

		// log to all callbacks
		char const *const str(&m_string_buffer.vec()[0]);
		for (auto &cb : m_logerror_list)
			(cb->m_func)(*this, str);

		g_profiler.stop();
	}
}

#endif  // __MACHINE_IPP__
