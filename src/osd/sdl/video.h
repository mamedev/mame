// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  video.h - SDL implementation of MAME video routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLVIDEO__
#define __SDLVIDEO__

#include "osdsdl.h"
#include "modules/osdwindow.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class sdl_monitor_info : public osd_monitor_info
{
public:
	sdl_monitor_info(const UINT64 handle, const char *monitor_device, float aspect)
	: osd_monitor_info(&m_handle, monitor_device, aspect), m_handle(handle)
	{
		refresh();
	}

	// STATIC
	static void init();
	static void exit();
private:
	void virtual refresh() override;

	UINT64              m_handle;                 // handle to the monitor
};

#endif
