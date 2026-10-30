// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  debuggerprefs.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "debuggerprefs.h"

#include "debug/debugvw.h"


namespace osd::debugger::win {

namespace {

float get_font_size(float size)
{
	return (size <= 0.0F) ? 9.0F : size;
}

std::string get_font_face(char const *face)
{
	return (!*face || !strcmp(OSDOPTVAL_AUTO, face)) ? "Lucida Console" : face;
}

} // anonymous namespace


COLORREF const debugger_preferences::s_themes[][COLOR_COUNT] = {
		{
			RGB(0x00, 0x00, 0x00),  // foreground normal
			RGB(0xff, 0x00, 0x00),  // foreground changed
			RGB(0x00, 0x00, 0xff),  // foreground invalid
			RGB(0x00, 0x80, 0x00),  // foreground comment
			RGB(0xff, 0xff, 0xff),  // background normal
			RGB(0xff, 0x80, 0x80),  // background selected
			RGB(0xe0, 0xe0, 0xe0),  // background ancillary
			RGB(0xff, 0xff, 0x00),  // background current
			RGB(0xff, 0xc0, 0x80),  // background current selected
			RGB(0xc0, 0xe0, 0xff)   // background visited
		},
		{
			RGB(0xe0, 0xe0, 0xe0),  // foreground normal
			RGB(0xff, 0x60, 0xff),  // foreground changed
			RGB(0x00, 0xc0, 0xe0),  // foreground invalid
			RGB(0x00, 0xe0, 0x00),  // foreground comment
			RGB(0x00, 0x00, 0x00),  // background normal
			RGB(0xe0, 0x00, 0x00),  // background selected
			RGB(0x40, 0x40, 0x40),  // background ancillary
			RGB(0x00, 0x00, 0xc0),  // background current
			RGB(0xb0, 0x60, 0x00),  // background current selected
			RGB(0x00, 0x40, 0x80)   // background visited
		} };


debugger_preferences::debugger_preferences(osd_options const &options) :
	m_font_family(get_font_face(options.debugger_font())),
	m_font_size(get_font_size(options.debugger_font_size()))
{
	// set default color theme
	set_color_theme(THEME_LIGHT_BACKGROUND);
}


debugger_preferences::~debugger_preferences()
{
}


std::pair<COLORREF, COLORREF> debugger_preferences::view_colors(u8 attrib) const
{
	std::pair<COLORREF, COLORREF> result;

	if (attrib & DCA_SELECTED)
		result.second = (attrib & DCA_CURRENT) ? m_colors[COLOR_BG_CURRENT_SELECTED] : m_colors[COLOR_BG_SELECTED];
	else if (attrib & DCA_CURRENT)
		result.second = m_colors[COLOR_BG_CURRENT];
	else if (attrib & DCA_ANCILLARY)
		result.second = m_colors[COLOR_BG_ANCILLARY];
	else if (attrib & DCA_VISITED)
		result.second = m_colors[COLOR_BG_VISITED];
	else
		result.second = m_colors[COLOR_BG_NORMAL];

	if (DCA_COMMENT & attrib)
	{
		result.first = m_colors[COLOR_FG_COMMENT];
	}
	else
	{
		if (attrib & DCA_INVALID)
			result.first = m_colors[COLOR_FG_INVALID];
		else if (attrib & DCA_CHANGED)
			result.first = m_colors[COLOR_FG_CHANGED];
		else
			result.first = m_colors[COLOR_FG_NORMAL];

		if (attrib & DCA_DISABLED)
		{
			result.first = RGB(
					(GetRValue(result.first) + GetRValue(result.second) + 1) >> 1,
					(GetGValue(result.first) + GetGValue(result.second) + 1) >> 1,
					(GetBValue(result.first) + GetBValue(result.second) + 1) >> 1);
		}
	}

	return result;
}


void debugger_preferences::set_color_theme(int index)
{
	if ((0 <= index) && (std::size(s_themes) > index))
	{
		std::copy(std::begin(s_themes[index]), std::end(s_themes[index]), m_colors);
		m_color_theme = index;
	}
}

} // namespace osd::debugger::win
