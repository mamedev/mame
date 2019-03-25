// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  video.h - Win32 video helpers
//
//============================================================
#ifndef MAME_OSD_WINDOWS_VIDEO_H
#define MAME_OSD_WINDOWS_VIDEO_H

#pragma once

#include "modules/osdhelper.h"

#include <windows.h>


constexpr osd_rect RECT_to_osd_rect(RECT const &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

inline RECT osd_rect_to_RECT(osd_rect const &r)
{
	RECT const result{ r.left(), r.top(), r.right(), r.bottom() };
	return result;
}

#endif // MAME_OSD_WINDOWS_VIDEO_H
