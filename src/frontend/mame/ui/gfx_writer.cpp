#include <filesystem>

#include "emu.h"
#include "gfx_writer.h"

gfxWriter::gfxWriter(running_machine& machine, gfx_viewer::gfxset& set) :
	mMachine{ machine },
	mSet{ set }
{
}

void gfxWriter::writePng()
{
}
