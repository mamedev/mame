// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  uimetrics.c - Win32 debug window handling
//
//============================================================

#include "uimetrics.h"

#include "strconv.h"


ui_metrics::ui_metrics(osd_options const &options) :
	m_debug_font(NULL),
	m_debug_font_height(0),
	m_debug_font_width(0),
	m_debug_font_ascent(0),
	m_hscroll_height(GetSystemMetrics(SM_CYHSCROLL)),
	m_vscroll_width(GetSystemMetrics(SM_CXVSCROLL))
{
	// create a temporary DC
	HDC const temp_dc = GetDC(NULL);
	if (temp_dc != NULL)
	{
		float const size = options.debugger_font_size();
		char const *const face = options.debugger_font();

		// create a standard font
		TCHAR *t_face = tstring_from_utf8((!*face || !strcmp(OSDOPTVAL_AUTO, face)) ? "Lucida Console" : face);
		m_debug_font = CreateFont(-MulDiv((size <= 0) ? 9 : size, GetDeviceCaps(temp_dc, LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
					ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, t_face);
		osd_free(t_face);

		if (m_debug_font == NULL)
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
		ReleaseDC(NULL, temp_dc);
	}
}


ui_metrics::ui_metrics(ui_metrics const &that) :
	m_debug_font(NULL),
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
