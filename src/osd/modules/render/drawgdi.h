// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawgdi.h - Win32 GDI drawing
//
//============================================================

#pragma once

#ifndef __DRAWGDI__
#define __DRAWGDI__

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "window.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

class renderer_gdi : public osd_renderer
{
public:
	renderer_gdi(osd_window *window)
		: osd_renderer(window, FLAG_NONE)
		, m_bmdata(nullptr)
		, m_bmsize(0)
	{
	}
	virtual ~renderer_gdi();

	static bool init(running_machine &machine) { return false; }
	static void exit() { }

	virtual int create() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override {};
	virtual void record() override {};
	virtual void toggle_fsfx() override {};

private:
	/* gdi_info is the information for the current screen */
	BITMAPINFO              m_bminfo;
	UINT8 *                 m_bmdata;
	size_t                  m_bmsize;
};

#endif // __DRAWGDI__
