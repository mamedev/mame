// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawnone.h - stub "nothing" drawer
//
//============================================================

// MAMEOS headers
#include "window.h"

#pragma once

#ifndef __DRAWNONE__
#define __DRAWNONE__

class renderer_none : public osd_renderer
{
public:
	renderer_none(osd_window *window)
	: osd_renderer(window, FLAG_NONE) { }

	virtual ~renderer_none() { }

	static bool init(running_machine &machine) { return false; }
	static void exit() { }

	virtual int create() override { return 0; }
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override { return 0; }
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
};

#endif // __DRAWNONE__
