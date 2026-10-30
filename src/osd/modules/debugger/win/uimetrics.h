// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  uimetrics.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_UIMETRICS_H
#define MAME_DEBUGGER_WIN_UIMETRICS_H

#pragma once

#include "debugwin.h"

#include "modules/lib/osdobj_common.h"

#include <utility>


namespace osd::debugger::win {

class ui_metrics
{
public:
	ui_metrics(debugger_preferences const &prefs);
	ui_metrics(ui_metrics const &that);
	~ui_metrics();

	void set_dpi(UINT dpi);

	UINT dpi() const { return m_dpi; }

	HFONT debug_font() const { return m_debug_font; }
	uint32_t debug_font_height() const { return m_debug_font_height; }
	uint32_t debug_font_width() const { return m_debug_font_width; }
	uint32_t debug_font_ascent() const { return m_debug_font_ascent; }

	uint32_t hscroll_height() const { return m_hscroll_height; }
	uint32_t vscroll_width() const { return m_vscroll_width; }

	std::pair<COLORREF, COLORREF> view_colors(u8 attrib) const;

private:
	debugger_preferences const &m_prefs;

	HFONT m_debug_font;

	UINT m_dpi;

	uint32_t m_debug_font_height;
	uint32_t m_debug_font_width;
	uint32_t m_debug_font_ascent;

	uint32_t m_hscroll_height;
	uint32_t m_vscroll_width;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_UIMETRICS_H
