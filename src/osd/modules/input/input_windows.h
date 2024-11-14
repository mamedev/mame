// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Brad Hughes
//============================================================
//
//  input_windows.h - Common code used by Windows input modules
//
//============================================================
#ifndef MAME_OSD_INPUT_INPUT_WINDOWS_H
#define MAME_OSD_INPUT_INPUT_WINDOWS_H

#pragma once

#include "input_common.h"

#include "window.h"
#include "winmain.h"

// standard windows headers
#include <windows.h>


//============================================================
//  TYPEDEFS
//============================================================


class wininput_event_handler
{
protected:
	wininput_event_handler() = default;
	virtual ~wininput_event_handler() = default;

public:
	virtual bool handle_input_event(input_event eventid, void const *data)
	{
		return false;
	}
};


template <typename Info>
class wininput_module : public input_module_impl<Info, osd_common_t>, public wininput_event_handler
{
protected:
	using input_module_impl<Info, osd_common_t>::input_module_impl;
};

#endif // MAME_OSD_INPUT_INPUT_WINDOWS_H
