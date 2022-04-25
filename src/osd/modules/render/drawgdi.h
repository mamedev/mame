// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawgdi.h - Win32 GDI drawing
//
//============================================================
#ifndef MAME_OSD_MODULES_RENDER_DRAWGDI_H
#define MAME_OSD_MODULES_RENDER_DRAWGDI_H

#pragma once


// standard windows headers
#include <windows.h>

// MAME headers

// MAMEOS headers
#include "window.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

/* renderer_gdi is the information for the current screen */
class renderer_gdi : public osd_renderer
{
public:
	renderer_gdi(std::shared_ptr<osd_window> window)
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
	virtual void save() override {}
	virtual void record() override {}
	virtual void toggle_fsfx() override {}

private:
	BITMAPINFO                  m_bminfo;
	std::unique_ptr<uint8_t []> m_bmdata;
	size_t                      m_bmsize;
};

#endif // MAME_OSD_MODULES_RENDER_DRAWGDI_H
