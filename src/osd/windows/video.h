// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__

#include "render.h"
#include "winmain.h"
#include "modules/osdwindow.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

inline osd_rect RECT_to_osd_rect(const RECT &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

class win_monitor_info : public osd_monitor_info
{
public:
	win_monitor_info(const HMONITOR handle, const char *monitor_device, float aspect);
	virtual ~win_monitor_info();

	virtual void refresh() override;

	// static

	static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
	static osd_monitor_info *monitor_from_handle(HMONITOR monitor);

	HMONITOR handle() { return m_handle; }

private:
	HMONITOR            m_handle;                 // handle to the monitor
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	MONITORINFOEX       m_info;                   // most recently retrieved info
#endif
};

#endif
