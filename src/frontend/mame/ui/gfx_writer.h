// license:BSD-3-Clause
// copyright-holders:Fabrice Lambert

#ifndef MAME_FRONTEND_MAME_UI_GFXWRITER_H
#define MAME_FRONTEND_MAME_UI_GFXWRITER_H

#pragma once

#include "machine.h"
#include "viewgfx.h"

class gfx_writer
{
private:
	running_machine& mMachine;
	gfx_viewer::gfxset& mGfxSet;
private:
	bitmap_argb32 getBitmap(int xCells, int yCells, gfx_viewer::gfxset::setinfo& set, gfx_element& gfx) const;
	void drawCell(gfx_element& gfx, int index, bitmap_argb32& bitmap, int dstx, int dsty, int color, int rotate, device_palette_interface* dpalette) const;
public:
	gfx_writer(running_machine& machine, gfx_viewer::gfxset& gfx);
	void writePng();
};

#endif
