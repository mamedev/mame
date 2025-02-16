#ifndef MAME_FRONTEND_MAME_UI_GFXWRITER_H
#define MAME_FRONTEND_MAME_UI_GFXWRITER_H

#pragma once

#include "machine.h"
#include "viewgfx.h"

class gfxWriter
{
private:
	running_machine& mMachine;
	gfx_viewer::gfxset& mSet;
public:
	gfxWriter(running_machine& machine, gfx_viewer::gfxset& set);
	void writePng();
};

#endif
