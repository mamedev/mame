// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * font_none.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

class osd_font_none : public osd_font
{
public:
	virtual ~osd_font_none() { }

	virtual bool open(std::string const &font_path, std::string const &name, int &height) override { return false; }
	virtual void close() override { }
	virtual bool get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs) override { return false; }
};

class font_none : public osd_module, public font_module
{
public:
	font_none() : osd_module(OSD_FONT_PROVIDER, "none"), font_module() { }

	virtual int init(const osd_options &options) override { return 0; }

	virtual osd_font::ptr font_alloc() override { return std::make_unique<osd_font_none>(); }
	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) override { return false; }
};

MODULE_DEFINITION(FONT_NONE, font_none)
