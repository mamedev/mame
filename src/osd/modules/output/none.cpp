// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.cpp

    Dummy output interface.

***************************************************************************/

#include "output_module.h"

#include "modules/osdmodule.h"

namespace osd {

namespace {

class output_none : public osd_module, public output_module
{
public:
	output_none() : osd_module(OSD_OUTPUT_PROVIDER, "none")
	{
	}
	virtual ~output_none() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	// output_module

	virtual void notify(const char *outname, int32_t value) override { }

};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(OUTPUT_NONE, osd::output_none)
