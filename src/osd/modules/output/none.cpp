// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.cpp

    Dummy output interface.

*******************************************************************c********/

#include "output_module.h"
#include "modules/osdmodule.h"

class output_none : public osd_module, public output_module
{
public:
	output_none()
	: osd_module(OSD_OUTPUT_PROVIDER, "none"), output_module()
	{
	}
	virtual ~output_none() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override { }

	// output_module

	virtual void notify(const char *outname, INT32 value) override { }

};

MODULE_DEFINITION(OUTPUT_NONE, output_none)
