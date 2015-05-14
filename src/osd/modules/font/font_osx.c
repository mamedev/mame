// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*
 * font_osx.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

#ifdef SDLMAME_MACOSX

#include <Carbon/Carbon.h>

#include "corealloc.h"
#include "fileio.h"

#define POINT_SIZE 144.0
#define EXTRA_HEIGHT 1.0
#define EXTRA_WIDTH 1.15

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_osx : public osd_font
{
public:
	virtual ~osd_font_osx() { }

	virtual bool open(const char *font_path, const char *name, int &height);
	virtual void close();
	virtual bool get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);
private:
	CTFontRef m_font;
};

bool osd_font_osx::open(const char *font_path, const char *name, int &height)
{
	m_font = NULL;
	osd_printf_verbose("FONT NAME %s\n", name);
#if 0
	if (!strcmp(name, "default"))
	{
		name = "LucidaGrande";
	}
#endif

	CFStringRef const font_name = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	if (kCFNotFound != CFStringFind(font_name, CFSTR(".BDF"), kCFCompareCaseInsensitive | kCFCompareBackwards | kCFCompareAnchored | kCFCompareNonliteral).location)
	{
		// handle bdf fonts in the core
		CFRelease(font_name);
		return false;
	}
	CTFontRef ct_font = NULL;
	if (font_name != NULL)
	{
		CTFontDescriptorRef const font_descriptor = CTFontDescriptorCreateWithNameAndSize(font_name, 0.0);
		if (font_descriptor != NULL)
		{
			ct_font = CTFontCreateWithFontDescriptor(font_descriptor, POINT_SIZE, &CGAffineTransformIdentity);
			CFRelease(font_descriptor);
		}
	}
	CFRelease(font_name);

	if (!ct_font)
	{
		osd_printf_verbose("Couldn't find/open font %s, using MAME default\n", name);
		return false;
	}

	CFStringRef const real_name = CTFontCopyPostScriptName(ct_font);
	char real_name_c_string[255];
	CFStringGetCString(real_name, real_name_c_string, 255, kCFStringEncodingUTF8);
	osd_printf_verbose("Matching font: %s\n", real_name_c_string);
	CFRelease(real_name);

	CGFloat line_height = 0.0;
	line_height += CTFontGetAscent(ct_font);
	line_height += CTFontGetDescent(ct_font);
	line_height += CTFontGetLeading(ct_font);
	height = ceilf(line_height * EXTRA_HEIGHT);

	m_font = ct_font;
	return true;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_osx::close()
{
	if (m_font != NULL)
	{
		CFRelease(m_font);
	}
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_osx::get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	UniChar uni_char;
	CGGlyph glyph;
	CTFontRef ct_font = m_font;
	const CFIndex count = 1;
	CGRect bounding_rect, success_rect;
	CGContextRef context_ref;

	if( chnum == ' ' )
	{
		uni_char = 'n';
		CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
		success_rect = CTFontGetBoundingRectsForGlyphs( ct_font, kCTFontDefaultOrientation, &glyph, &bounding_rect, count );
		uni_char = chnum;
		CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
	}
	else
	{
		uni_char = chnum;
		CTFontGetGlyphsForCharacters( ct_font, &uni_char, &glyph, count );
		success_rect = CTFontGetBoundingRectsForGlyphs( ct_font, kCTFontDefaultOrientation, &glyph, &bounding_rect, count );
	}

	if( CGRectEqualToRect( success_rect, CGRectNull ) == false )
	{
		size_t bitmap_width;
		size_t bitmap_height;

		bitmap_width = ceilf(bounding_rect.size.width * EXTRA_WIDTH);
		bitmap_width = bitmap_width == 0 ? 1 : bitmap_width;

		bitmap_height = ceilf( (CTFontGetAscent(ct_font) + CTFontGetDescent(ct_font) + CTFontGetLeading(ct_font)) * EXTRA_HEIGHT);

		xoffs = yoffs = 0;
		width = bitmap_width;

		size_t bits_per_component;
		CGColorSpaceRef color_space;
		CGBitmapInfo bitmap_info = kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst;

		color_space = CGColorSpaceCreateDeviceRGB();
		bits_per_component = 8;

		bitmap.allocate(bitmap_width, bitmap_height);

		context_ref = CGBitmapContextCreate( bitmap.raw_pixptr(0), bitmap_width, bitmap_height, bits_per_component, bitmap.rowpixels()*4, color_space, bitmap_info );

		if( context_ref != NULL )
		{
			CGFontRef font_ref;
			font_ref = CTFontCopyGraphicsFont( ct_font, NULL );
			CGContextSetTextPosition(context_ref, -bounding_rect.origin.x*EXTRA_WIDTH, CTFontGetDescent(ct_font)+CTFontGetLeading(ct_font) );
			CGContextSetRGBFillColor(context_ref, 1.0, 1.0, 1.0, 1.0);
			CGContextSetFont( context_ref, font_ref );
			CGContextSetFontSize( context_ref, POINT_SIZE );
			CGContextShowGlyphs( context_ref, &glyph, count );
			CGFontRelease( font_ref );
			CGContextRelease( context_ref );
		}

		CGColorSpaceRelease( color_space );
	}

	return bitmap.valid();
}


class font_osx : public osd_module, public font_module
{
public:
	font_osx()
	: osd_module(OSD_FONT_PROVIDER, "osx"), font_module()
	{
	}

	virtual int init(const osd_options &options) { return 0; }

	osd_font *font_alloc()
	{
		return global_alloc(osd_font_osx);
	}

};
#else /* SDLMAME_MACOSX */
	MODULE_NOT_SUPPORTED(font_osx, OSD_FONT_PROVIDER, "osx")
#endif

MODULE_DEFINITION(FONT_OSX, font_osx)
