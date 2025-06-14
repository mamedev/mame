// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    console.cpp

    Console output interface.

***************************************************************************/

#include "output_module.h"

#include "modules/osdmodule.h"
#include "osdcore.h"

namespace osd {

namespace {

class output_console : public osd_module, public output_module
{
public:
	output_console() : osd_module(OSD_OUTPUT_PROVIDER, "console")
	{
	}
	virtual ~output_console() { }

	virtual int init(osd_interface &osd, const osd_options &options) override { return 0; }
	virtual void exit() override { }

	// output_module

	virtual void notify(const char *outname, int32_t value) override { osd_printf_info("%s = %d\n", ((outname==nullptr) ? "none" : outname), value); }

};

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(OUTPUT_CONSOLE, osd::output_console)
