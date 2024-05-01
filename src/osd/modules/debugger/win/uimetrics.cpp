// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  uimetrics.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "uimetrics.h"

#include "debug/debugvw.h"

#include "strconv.h"

#include <algorithm>


namespace osd::debugger::win {

COLORREF const ui_metrics::s_themes[][COLOR_COUNT] = {
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


ui_metrics::ui_metrics(osd_options const &options) :
	m_debug_font(nullptr),
	m_debug_font_height(0),
	m_debug_font_width(0),
	m_debug_font_ascent(0),
	m_hscroll_height(GetSystemMetrics(SM_CYHSCROLL)),
	m_vscroll_width(GetSystemMetrics(SM_CXVSCROLL))
{
	// create a temporary DC
	HDC const temp_dc = GetDC(nullptr);
	if (temp_dc)
	{
		float const size = options.debugger_font_size();
		char const *const face = options.debugger_font();

		// create a standard font
		auto t_face = osd::text::to_tstring((!*face || !strcmp(OSDOPTVAL_AUTO, face)) ? "Lucida Console" : face);
		m_debug_font = CreateFont(-MulDiv((size <= 0) ? 9 : size, GetDeviceCaps(temp_dc, LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
					ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, t_face.c_str());

		if (m_debug_font == nullptr)
			fatalerror("Unable to create debugger font\n");

		// get the metrics
		HGDIOBJ const old_font = SelectObject(temp_dc, m_debug_font);
		TEXTMETRIC metrics;
		if (GetTextMetrics(temp_dc, &metrics))
		{
			m_debug_font_width = metrics.tmAveCharWidth;
			m_debug_font_height = metrics.tmHeight;
			m_debug_font_ascent = metrics.tmAscent + metrics.tmExternalLeading;
		}
		SelectObject(temp_dc, old_font);
		ReleaseDC(nullptr, temp_dc);
	}

	// set default color theme
	set_color_theme(THEME_LIGHT_BACKGROUND);
}


ui_metrics::ui_metrics(ui_metrics const &that) :
	m_debug_font(nullptr),
	m_debug_font_height(that.m_debug_font_height),
	m_debug_font_width(that.m_debug_font_width),
	m_debug_font_ascent(that.m_debug_font_ascent),
	m_hscroll_height(that.m_hscroll_height),
	m_vscroll_width(that.m_vscroll_width)
{
	LOGFONT lf;
	if (GetObject(that.m_debug_font, sizeof(lf), &lf))
		m_debug_font = CreateFontIndirect(&lf);
}


ui_metrics::~ui_metrics()
{
	if (m_debug_font)
		DeleteObject(m_debug_font);
}


std::pair<COLORREF, COLORREF> ui_metrics::view_colors(u8 attrib) const
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


void ui_metrics::set_color_theme(int index)
{
	if ((0 <= index) && (std::size(s_themes) > index))
	{
		std::copy(std::begin(s_themes[index]), std::end(s_themes[index]), m_colors);
		m_color_theme = index;
	}
}

} // namespace osd::debugger::win
