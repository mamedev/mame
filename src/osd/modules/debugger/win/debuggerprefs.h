// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debuggerprefs.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DEBUGGERPREFS_H
#define MAME_DEBUGGER_WIN_DEBUGGERPREFS_H

#pragma once

#include "debugwin.h"

#include "modules/lib/osdobj_common.h"

#include <string>
#include <utility>


namespace osd::debugger::win {

class debugger_preferences
{
public:
	enum
	{
		THEME_LIGHT_BACKGROUND,
		THEME_DARK_BACKGROUND
	};

	debugger_preferences(osd_options const &options);
	~debugger_preferences();

	std::string const &debugger_font_family() const { return m_font_family; }
	float debugger_font_size() const { return m_font_size; }

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

	std::string const m_font_family;
	float const m_font_size;

	COLORREF m_colors[COLOR_COUNT];
	int m_color_theme;

	static COLORREF const s_themes[][COLOR_COUNT];
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DEBUGGERPREFS_H
