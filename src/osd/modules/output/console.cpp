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
	virtual void notify(const output_item &item, std::int32_t seconds, std::int64_t attoseconds) override
	{ osd_printf_info("%s = %d\n", item.qualified_name(), item.value()); }
	virtual void pause() override
	{ osd_printf_info("pause = 1\n"); }
	virtual void resume() override
	{ osd_printf_info("pause = 0\n"); }
	virtual void update() override { }
};

} // anonymous namespace

} // namespace osd


MODULE_DEFINITION(OUTPUT_CONSOLE, osd::output_console)
