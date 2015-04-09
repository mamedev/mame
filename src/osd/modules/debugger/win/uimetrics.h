// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  uimetrics.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_UI_METRICS_H__
#define __DEBUG_WIN_UI_METRICS_H__

#include "debugwin.h"

#include "emu.h"

#include "modules/lib/osdobj_common.h"


class ui_metrics
{
public:
	ui_metrics(osd_options const &options);
	ui_metrics(ui_metrics const &that);
	~ui_metrics();

	HFONT debug_font() const { return m_debug_font; }
	UINT32 debug_font_height() const { return m_debug_font_height; }
	UINT32 debug_font_width() const { return m_debug_font_width; }
	UINT32 debug_font_ascent() const { return m_debug_font_ascent; }

	UINT32 hscroll_height() const { return m_hscroll_height; }
	UINT32 vscroll_width() const { return m_vscroll_width; }

private:
	HFONT m_debug_font;
	UINT32 m_debug_font_height;
	UINT32 m_debug_font_width;
	UINT32 m_debug_font_ascent;

	UINT32 const m_hscroll_height;
	UINT32 const m_vscroll_width;
};

#endif
