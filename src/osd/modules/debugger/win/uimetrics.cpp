// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  uimetrics.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "uimetrics.h"

#include "debuggerprefs.h"

#include "strconv.h"

#include <algorithm>


namespace osd::debugger::win {

ui_metrics::ui_metrics(debugger_preferences const &prefs) :
	m_prefs(prefs),
	m_debug_font(nullptr),
	m_dpi(0),
	m_debug_font_height(0),
	m_debug_font_width(0),
	m_debug_font_ascent(0),
	m_hscroll_height(0),
	m_vscroll_width(0)
{
}


ui_metrics::ui_metrics(ui_metrics const &that) :
	m_prefs(that.m_prefs),
	m_debug_font(nullptr),
	m_dpi(that.m_dpi),
	m_debug_font_height(that.m_debug_font_height),
	m_debug_font_width(that.m_debug_font_width),
	m_debug_font_ascent(that.m_debug_font_ascent),
	m_hscroll_height(that.m_hscroll_height),
	m_vscroll_width(that.m_vscroll_width)
{
	LOGFONT lf;
	if (that.m_debug_font && GetObject(that.m_debug_font, sizeof(lf), &lf))
		m_debug_font = CreateFontIndirect(&lf);
}


ui_metrics::~ui_metrics()
{
	if (m_debug_font)
		DeleteObject(m_debug_font);
}


void ui_metrics::set_dpi(UINT dpi)
{
	if (dpi == m_dpi)
		return;

	if (m_debug_font)
	{
		DeleteObject(m_debug_font);
		m_debug_font = nullptr;
	}

	m_dpi = dpi;

	m_debug_font_height = 0;
	m_debug_font_width = 0;
	m_debug_font_ascent = 0;

	m_hscroll_height = GetSystemMetricsForDpi(SM_CYHSCROLL, dpi),
	m_vscroll_width = GetSystemMetricsForDpi(SM_CXVSCROLL, dpi);

	// create a temporary DC
	HDC const temp_dc = GetDC(nullptr);
	if (temp_dc)
	{
		// create a standard font
		auto t_face = osd::text::to_tstring(m_prefs.debugger_font_family());
		m_debug_font = CreateFont(
				-MulDiv(m_prefs.debugger_font_size(), dpi, 72), 0, 0, 0,
				FW_MEDIUM, FALSE, FALSE, FALSE,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				FIXED_PITCH, t_face.c_str());

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
}


std::pair<COLORREF, COLORREF> ui_metrics::view_colors(u8 attrib) const
{
	return m_prefs.view_colors(attrib);
}

} // namespace osd::debugger::win
