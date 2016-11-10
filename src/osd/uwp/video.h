// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__

#include "modules/osdhelper.h"

inline osd_rect RECT_to_osd_rect(const RECT &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

#endif
