// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
/*
 * font_osx.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

#ifdef SDLMAME_MACOSX

#include "fileio.h"

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

#include <cmath>


//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_osx : public osd_font
{
public:
	osd_font_osx() : m_font(nullptr), m_height(0), m_baseline(0) { }
	osd_font_osx(osd_font_osx &&obj) : m_font(obj.m_font), m_height(obj.m_height), m_baseline(obj.m_baseline) { obj.m_font = nullptr; }
	virtual ~osd_font_osx() { close(); }

	virtual bool open(std::string const &font_path, std::string const &name, int &height);
	virtual void close();
	virtual bool get_bitmap(char32_t chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs);

	osd_font_osx &operator=(osd_font_osx &&obj)
	{
		using std::swap;
		swap(m_font, obj.m_font);
		return *this;
	}

private:
	osd_font_osx(osd_font_osx const &) = delete;
	osd_font_osx &operator=(osd_font_osx const &) = delete;

	static constexpr CGFloat POINT_SIZE = 144.0;

	CTFontRef m_font;
	CGFloat m_height, m_baseline;
};

bool osd_font_osx::open(std::string const &font_path, std::string const &name, int &height)
{
	osd_printf_verbose("osd_font_osx::open: name=\"%s\"\n", name);

	CFStringRef font_name;
	if (!strcmp(name.c_str(), "default"))
	{
		// Arial Unicode MS comes with Mac OS X 10.5 and later and is the only Mac default font with
		// the Unicode characters used by the vgmplay and aristmk5 layouts.
		font_name = CFStringCreateWithCString(nullptr, "Arial Unicode MS", kCFStringEncodingUTF8);
	}
	else
	{
		font_name = CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8);
	}

	if (!font_name)
	{
		osd_printf_verbose("osd_font_osx::open: failed to create CFString from font name (invalid UTF-8?)\n");
		return false;
	}

	if (kCFNotFound != CFStringFind(font_name, CFSTR(".BDF"), kCFCompareCaseInsensitive | kCFCompareBackwards | kCFCompareAnchored | kCFCompareNonliteral).location)
	{
		// handle BDF fonts in the core
		CFRelease(font_name);
		return false;
	}

	CTFontDescriptorRef const font_descriptor(CTFontDescriptorCreateWithNameAndSize(font_name, 0.0));
	CFRelease(font_name);
	if (!font_descriptor)
	{
		osd_printf_verbose("osd_font_osx::open: failed to create CoreText font descriptor for \"%s\"\n", name);
		return false;
	}

	CTFontRef const ct_font(CTFontCreateWithFontDescriptor(font_descriptor, POINT_SIZE, &CGAffineTransformIdentity));
	CFRelease(font_descriptor);
	if (!ct_font)
	{
		osd_printf_verbose("osd_font_osx::open: failed to create CoreText font for \"%s\"\n", name);
		return false;
	}

	CFStringRef const real_name(CTFontCopyPostScriptName(ct_font));
	if (real_name)
	{
		char const *real_name_c_string(CFStringGetCStringPtr(real_name, kCFStringEncodingUTF8));
		std::unique_ptr<char []> buf;
		if (!real_name_c_string)
		{
			CFIndex const len(CFStringGetLength(real_name));
			CFIndex const space(CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8) + 1);
			buf.reset(new char[space]);
			CFStringGetCString(real_name, buf.get(), space, kCFStringEncodingUTF8);
			real_name_c_string = buf.get();
		}
		osd_printf_verbose("osd_font_osx::open: matching font: %s\n", real_name_c_string);
		CFRelease(real_name);
	}

	m_baseline = CTFontGetDescent(ct_font) + CTFontGetLeading(ct_font);
	m_height = CTFontGetAscent(ct_font) + m_baseline;
	height = std::ceil(m_height);

	close();
	m_font = ct_font;
	return true;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_osx::close()
{
	if (m_font)
		CFRelease(m_font);
	m_font = nullptr;
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_osx::get_bitmap(char32_t chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs)
{
	// turn character into a glyph index
	UniChar const uni_char(chnum);
	CFIndex const count(1);
	CGGlyph glyph;
	if (!CTFontGetGlyphsForCharacters(m_font, &uni_char, &glyph, count))
		osd_printf_verbose("osd_font_osd::get_bitmap: failed to get glyph for U+%04X\n", unsigned(chnum));

	// try to get glyph metrics
	#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100
	CGRect const bounds(CTFontGetBoundingRectsForGlyphs(m_font, kCTFontOrientationHorizontal, &glyph, nullptr, count));
	CGSize advance(CGSizeZero);
	CTFontGetAdvancesForGlyphs(m_font, kCTFontOrientationHorizontal, &glyph, &advance, count);
	#else
	CGRect const bounds(CTFontGetBoundingRectsForGlyphs(m_font, kCTFontHorizontalOrientation, &glyph, nullptr, count));
	CGSize advance(CGSizeZero);
	CTFontGetAdvancesForGlyphs(m_font, kCTFontHorizontalOrientation, &glyph, &advance, count);
	#endif


	// CGNullRect can indicate failure, but it's also a valid rectangle for spaces in some fonts (e.g. Hiragino family)
	if (CGRectEqualToRect(bounds, CGRectNull) && CGSizeEqualToSize(advance, CGSizeZero))
	{
		osd_printf_verbose("osd_font_osd::get_bitmap: failed to get glyph metrics for U+%04X\n", unsigned(chnum));
		return false;
	}

	// turn everything into integers for MAME and allocate output bitmap
	std::size_t const bitmap_width(std::max(std::ceil(bounds.size.width), CGFloat(1.0)));
	std::size_t const bitmap_height(m_height);
	width = std::ceil(advance.width);
	xoffs = std::ceil(bounds.origin.x);
	yoffs = 0;
	bitmap.allocate(bitmap_width, bitmap_height);

	// create graphics context and draw
	CGBitmapInfo const bitmap_info(kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
	CGColorSpaceRef const color_space(CGColorSpaceCreateDeviceRGB());
	CGContextRef const context_ref(CGBitmapContextCreate(bitmap.raw_pixptr(0), bitmap_width, bitmap_height, 8, bitmap.rowpixels() * 4, color_space, bitmap_info));
	if (context_ref)
	{
		CGFontRef const font_ref(CTFontCopyGraphicsFont(m_font, nullptr));
		CGContextSetTextPosition(context_ref, -bounds.origin.x, m_baseline);
		CGContextSetRGBFillColor(context_ref, 1.0, 1.0, 1.0, 1.0);
		CGContextSetFont(context_ref, font_ref);
		CGContextSetFontSize(context_ref, POINT_SIZE);
		#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1090
		CGPoint pos = CGPointMake(0, 0);
		CTFontDrawGlyphs(m_font, &glyph, &pos, 1, context_ref);
		#else
		CGContextShowGlyphs(context_ref, &glyph, count);
		#endif
		CGFontRelease(font_ref);
		CGContextRelease(context_ref);
	}
	CGColorSpaceRelease(color_space);

	// bitmap will be valid if we drew to it
	return bitmap.valid();
}


class font_osx : public osd_module, public font_module
{
public:
	font_osx() : osd_module(OSD_FONT_PROVIDER, "osx"), font_module() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual osd_font::ptr font_alloc() override { return std::make_unique<osd_font_osx>(); }
	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) override;

private:
	static CFComparisonResult sort_callback(CTFontDescriptorRef first, CTFontDescriptorRef second, void *refCon)
	{
		CFStringRef left(CFStringRef(CTFontDescriptorCopyLocalizedAttribute(first, kCTFontDisplayNameAttribute, nullptr)));
		if (!left)
			left = CFStringRef(CTFontDescriptorCopyAttribute(first, kCTFontNameAttribute));
		CFStringRef right(CFStringRef(CTFontDescriptorCopyLocalizedAttribute(second, kCTFontDisplayNameAttribute, nullptr)));
		if (!right)
			right = CFStringRef(CTFontDescriptorCopyAttribute(second, kCTFontNameAttribute));

		CFComparisonResult const result(
				(left && right) ? CFStringCompareWithOptions(left, right, CFRangeMake(0, CFStringGetLength(left)), kCFCompareCaseInsensitive | kCFCompareLocalized | kCFCompareNonliteral) :
				!left ? kCFCompareLessThan :
				!right ? kCFCompareGreaterThan :
				kCFCompareEqualTo);

		if (left)
			CFRelease(left);
		if (right)
			CFRelease(right);
		return result;
	}
};

bool font_osx::get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result)
{
	CFStringRef keys[] = { kCTFontCollectionRemoveDuplicatesOption };
	std::uintptr_t values[ARRAY_LENGTH(keys)] = { 1 };
	CFDictionaryRef const options = CFDictionaryCreate(kCFAllocatorDefault, (void const **)keys, (void const **)values, ARRAY_LENGTH(keys), &kCFTypeDictionaryKeyCallBacks, nullptr);
	CTFontCollectionRef const collection = CTFontCollectionCreateFromAvailableFonts(nullptr);
	CFRelease(options);
	if (!collection) return false;

	CFArrayRef const descriptors = CTFontCollectionCreateMatchingFontDescriptorsSortedWithCallback(collection, &sort_callback, nullptr);
	CFRelease(collection);
	if (!descriptors) return false;

	result.clear();
	CFIndex const count = CFArrayGetCount(descriptors);
	result.reserve(count);
	for (CFIndex i = 0; i != count; i++)
	{
		CTFontDescriptorRef const font = (CTFontDescriptorRef)CFArrayGetValueAtIndex(descriptors, i);
		CFStringRef const name = (CFStringRef)CTFontDescriptorCopyAttribute(font, kCTFontNameAttribute);
		CFStringRef const display = (CFStringRef)CTFontDescriptorCopyLocalizedAttribute(font, kCTFontDisplayNameAttribute, nullptr);

		if (name && display)
		{
			char const *utf;
			std::vector<char> buf;

			utf = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
			if (!utf)
			{
				buf.resize(CFStringGetMaximumSizeForEncoding(std::max(CFStringGetLength(name), CFStringGetLength(display)), kCFStringEncodingUTF8));
				CFStringGetCString(name, &buf[0], buf.size(), kCFStringEncodingUTF8);
			}
			std::string utf8name(utf ? utf : &buf[0]);

			utf = CFStringGetCStringPtr(display, kCFStringEncodingUTF8);
			if (!utf)
			{
				buf.resize(CFStringGetMaximumSizeForEncoding(CFStringGetLength(display), kCFStringEncodingUTF8));
				CFStringGetCString(display, &buf[0], buf.size(), kCFStringEncodingUTF8);
			}
			std::string utf8display(utf ? utf : &buf[0]);

			result.emplace_back(std::move(utf8name), std::move(utf8display));
		}

		if (name)
			CFRelease(name);
		if (display)
			CFRelease(display);
	}

	return true;
}

#else /* SDLMAME_MACOSX */

MODULE_NOT_SUPPORTED(font_osx, OSD_FONT_PROVIDER, "osx")

#endif

MODULE_DEFINITION(FONT_OSX, font_osx)
