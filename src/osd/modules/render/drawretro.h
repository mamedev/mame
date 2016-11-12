
//============================================================
//
//  drawretro.h - basic retro drawing
//
//============================================================

#pragma once

#ifndef __DRAWRETRO__
#define __DRAWRETRO__

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "window.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

class renderer_retro : public osd_renderer
{
public:
	renderer_retro(std::shared_ptr<osd_window> window)
		: osd_renderer(window, FLAG_NONE)
		, m_bmdata(nullptr)
		, m_bmsize(0)
	{
	}
	virtual ~renderer_retro();

	static bool init(running_machine &machine) { return false; }
	static void exit() { }

	virtual int create() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override {};
	virtual void record() override {};
	virtual void toggle_fsfx() override {};
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;

private:

	uint8_t *               m_bmdata;
	size_t                  m_bmsize;
};

#endif // __DRAWGDI__
