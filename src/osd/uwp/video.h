// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.h - UWP implementation of MAME video routines
//
//============================================================

#ifndef __UWP_VIDEO__
#define __UWP_VIDEO__

#include "modules/osdhelper.h"

inline osd_rect RECT_to_osd_rect(const RECT &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

#endif
