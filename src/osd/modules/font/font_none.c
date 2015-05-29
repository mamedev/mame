// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * font_none.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_none : public osd_font
{
public:
	virtual ~osd_font_none() { }

	virtual bool open(const char *font_path, const char *name, int &height);
	virtual void close();
	virtual bool get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);
private:
};

bool osd_font_none::open(const char *font_path, const char *_name, int &height)
{
	return false;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_none::close()
{
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_none::get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	return false;
}

class font_none : public osd_module, public font_module
{
public:
	font_none() : osd_module(OSD_FONT_PROVIDER, "none"), font_module()
	{
	}

	virtual int init(const osd_options &options) { return 0; }

	osd_font *font_alloc()
	{
		return global_alloc(osd_font_none);
	}
};

MODULE_DEFINITION(FONT_NONE, font_none)
