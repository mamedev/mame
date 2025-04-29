// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.cpp

    Dummy sound interface.

*******************************************************************c********/

#include "sound_module.h"

#include "modules/osdmodule.h"


namespace osd {

namespace {

class sound_none : public osd_module, public sound_module
{
public:
	sound_none() : osd_module(OSD_SOUND_PROVIDER, "none")
	{
	}
	virtual ~sound_none() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(SOUND_NONE, osd::sound_none)
