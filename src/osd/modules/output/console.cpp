// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    console.cpp

    Console output interface.

*******************************************************************c********/

#include "output_module.h"
#include "modules/osdmodule.h"

#include "osdcore.h"

class output_console : public osd_module, public output_module
{
public:
	output_console()
	: osd_module(OSD_OUTPUT_PROVIDER, "console"), output_module()
	{
	}
	virtual ~output_console() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override { }

	// output_module

	virtual void notify(const char *outname, int32_t value) override { osd_printf_info("%s = %d\n", ((outname==nullptr) ? "none" : outname), value); }

};

MODULE_DEFINITION(OUTPUT_CONSOLE, output_console)
