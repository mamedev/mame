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
	enum
	{
		THEME_LIGHT_BACKGROUND,
		THEME_DARK_BACKGROUND
	};

	ui_metrics(osd_options const &options);
	ui_metrics(ui_metrics const &that);
	~ui_metrics();

	HFONT debug_font() const { return m_debug_font; }
	uint32_t debug_font_height() const { return m_debug_font_height; }
	uint32_t debug_font_width() const { return m_debug_font_width; }
	uint32_t debug_font_ascent() const { return m_debug_font_ascent; }

	uint32_t hscroll_height() const { return m_hscroll_height; }
	uint32_t vscroll_width() const { return m_vscroll_width; }

	std::pair<COLORREF, COLORREF> view_colors(u8 attrib) const;
	int get_color_theme() const { return m_color_theme; }
	void set_color_theme(int index);

private:
	enum
	{
		COLOR_FG_NORMAL,
		COLOR_FG_CHANGED,
		COLOR_FG_INVALID,
		COLOR_FG_COMMENT,

		COLOR_BG_NORMAL,
		COLOR_BG_SELECTED,
		COLOR_BG_ANCILLARY,
		COLOR_BG_CURRENT,
		COLOR_BG_CURRENT_SELECTED,
		COLOR_BG_VISITED,

		COLOR_COUNT
	};

	HFONT m_debug_font;
	uint32_t m_debug_font_height;
	uint32_t m_debug_font_width;
	uint32_t m_debug_font_ascent;

	uint32_t const m_hscroll_height;
	uint32_t const m_vscroll_width;

	COLORREF m_colors[COLOR_COUNT];
	int m_color_theme;

	static COLORREF const s_themes[][COLOR_COUNT];
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_UIMETRICS_H
