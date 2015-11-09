// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 * font_windows.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <tchar.h>
#include <io.h>

#include "font_module.h"
#include "modules/osdmodule.h"

#include "strconv.h"
#include "corestr.h"
#include "corealloc.h"
#include "fileio.h"

//#define POINT_SIZE 144.0
#define DEFAULT_FONT_HEIGHT (200)

//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_windows : public osd_font
{
public:
	virtual ~osd_font_windows() { }

	virtual bool open(const char *font_path, const char *name, int &height);
	virtual void close();
	virtual bool get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);
private:
	HGDIOBJ m_font;
};

bool osd_font_windows::open(const char *font_path, const char *_name, int &height)
{
	// accept qualifiers from the name
	std::string name(_name);
	if (name.compare("default")==0) name = "Tahoma";
	bool bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	bool italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);

	// build a basic LOGFONT description of what we want
	LOGFONT logfont;
	logfont.lfHeight = DEFAULT_FONT_HEIGHT;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = bold ? FW_BOLD : FW_MEDIUM;
	logfont.lfItalic = italic;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = NONANTIALIASED_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	// copy in the face name
	TCHAR *face = tstring_from_utf8(name.c_str());
	_tcsncpy(logfont.lfFaceName, face, sizeof(logfont.lfFaceName) / sizeof(TCHAR));
	logfont.lfFaceName[sizeof(logfont.lfFaceName) / sizeof(TCHAR)-1] = 0;
	osd_free(face);

	// create the font
	height = logfont.lfHeight;
	m_font = CreateFontIndirect(&logfont);
	if (m_font == NULL)
		return false;

	// select it into a temp DC and get the real font name
	HDC dummyDC = CreateCompatibleDC(NULL);
	HGDIOBJ oldfont = SelectObject(dummyDC, m_font);
	TCHAR realname[100];
	GetTextFace(dummyDC, ARRAY_LENGTH(realname), realname);
	SelectObject(dummyDC, oldfont);
	DeleteDC(dummyDC);

	// if it doesn't match our request, fail
	char *utf = utf8_from_tstring(realname);
	int result = core_stricmp(utf, name.c_str());
	osd_free(utf);

	// if we didn't match, nuke our font and fall back
	if (result != 0)
	{
		DeleteObject(m_font);
		m_font = NULL;
		return false;
	}
	return true;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_windows::close()
{
	// delete the font ojbect
	if (m_font != NULL)
		DeleteObject(m_font);

}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_windows::get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	// create a dummy DC to work with
	HDC dummyDC = CreateCompatibleDC(NULL);
	HGDIOBJ oldfont = SelectObject(dummyDC, m_font);

	// get the text metrics
	TEXTMETRIC metrics = { 0 };
	GetTextMetrics(dummyDC, &metrics);

	// get the width of this character
	ABC abc;
	if (!GetCharABCWidths(dummyDC, chnum, chnum, &abc))
	{
		abc.abcA = 0;
		abc.abcC = 0;
		GetCharWidth32(dummyDC, chnum, chnum, reinterpret_cast<LPINT>(&abc.abcB));
	}
	width = abc.abcA + abc.abcB + abc.abcC;

	// determine desired bitmap size
	int bmwidth = (50 + abc.abcA + abc.abcB + abc.abcC + 50 + 31) & ~31;
	int bmheight = 50 + metrics.tmHeight + 50;

	// describe the bitmap we want
	BYTE bitmapinfodata[sizeof(BITMAPINFOHEADER)+2 * sizeof(RGBQUAD)] = { 0 };
	BITMAPINFO &info = *reinterpret_cast<BITMAPINFO *>(bitmapinfodata);
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = bmwidth;
	info.bmiHeader.biHeight = -bmheight;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 1;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biXPelsPerMeter = GetDeviceCaps(dummyDC, HORZRES) / GetDeviceCaps(dummyDC, HORZSIZE);
	info.bmiHeader.biYPelsPerMeter = GetDeviceCaps(dummyDC, VERTRES) / GetDeviceCaps(dummyDC, VERTSIZE);
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	RGBQUAD col1 = info.bmiColors[0];
	RGBQUAD col2 = info.bmiColors[1];
	col1.rgbBlue = col1.rgbGreen = col1.rgbRed = 0x00;
	col2.rgbBlue = col2.rgbGreen = col2.rgbRed = 0xff;

	// create a DIB to render to
	BYTE *bits;
	HBITMAP dib = CreateDIBSection(dummyDC, &info, DIB_RGB_COLORS, reinterpret_cast<VOID **>(&bits), NULL, 0);

	if (dib)
	{
		HGDIOBJ oldbitmap = SelectObject(dummyDC, dib);

		// clear the bitmap
		int rowbytes = bmwidth / 8;
		memset(bits, 0, rowbytes * bmheight);

		// now draw the character
		WCHAR tempchar = chnum;
		SetTextColor(dummyDC, RGB(0xff, 0xff, 0xff));
		SetBkColor(dummyDC, RGB(0x00, 0x00, 0x00));
		ExtTextOutW(dummyDC, 50 + abc.abcA, 50, ETO_OPAQUE, NULL, &tempchar, 1, NULL);

		// characters are expected to be full-height
		rectangle actbounds;
		actbounds.min_y = 50;
		actbounds.max_y = 50 + metrics.tmHeight - 1;

		// determine the actual left of the character
		for (actbounds.min_x = 0; actbounds.min_x < rowbytes; actbounds.min_x++)
		{
			BYTE *offs = bits + actbounds.min_x;
			UINT8 summary = 0;
			for (int y = 0; y < bmheight; y++)
				summary |= offs[y * rowbytes];
			if (summary != 0)
			{
				actbounds.min_x *= 8;
				if (!(summary & 0x80)) actbounds.min_x++;
				if (!(summary & 0xc0)) actbounds.min_x++;
				if (!(summary & 0xe0)) actbounds.min_x++;
				if (!(summary & 0xf0)) actbounds.min_x++;
				if (!(summary & 0xf8)) actbounds.min_x++;
				if (!(summary & 0xfc)) actbounds.min_x++;
				if (!(summary & 0xfe)) actbounds.min_x++;
				break;
			}
		}

		// determine the actual right of the character
		for (actbounds.max_x = rowbytes - 1; actbounds.max_x >= 0; actbounds.max_x--)
		{
			BYTE *offs = bits + actbounds.max_x;
			UINT8 summary = 0;
			for (int y = 0; y < bmheight; y++)
				summary |= offs[y * rowbytes];
			if (summary != 0)
			{
				actbounds.max_x *= 8;
				if (summary & 0x7f) actbounds.max_x++;
				if (summary & 0x3f) actbounds.max_x++;
				if (summary & 0x1f) actbounds.max_x++;
				if (summary & 0x0f) actbounds.max_x++;
				if (summary & 0x07) actbounds.max_x++;
				if (summary & 0x03) actbounds.max_x++;
				if (summary & 0x01) actbounds.max_x++;
				break;
			}
		}

		// allocate a new bitmap
		if (actbounds.max_x >= actbounds.min_x && actbounds.max_y >= actbounds.min_y)
		{
			bitmap.allocate(actbounds.max_x + 1 - actbounds.min_x, actbounds.max_y + 1 - actbounds.min_y);

			// copy the bits into it
			for (int y = 0; y < bitmap.height(); y++)
			{
				UINT32 *dstrow = &bitmap.pix32(y);
				UINT8 *srcrow = &bits[(y + actbounds.min_y) * rowbytes];
				for (int x = 0; x < bitmap.width(); x++)
				{
					int effx = x + actbounds.min_x;
					dstrow[x] = ((srcrow[effx / 8] << (effx % 8)) & 0x80) ? rgb_t(0xff, 0xff, 0xff, 0xff) : rgb_t(0x00, 0xff, 0xff, 0xff);
				}
			}

			// set the final offset values
			xoffs = actbounds.min_x - (50 + abc.abcA);
			yoffs = actbounds.max_y - (50 + metrics.tmAscent);
		}

		// de-select the font and release the DC
		SelectObject(dummyDC, oldbitmap);
		DeleteObject(dib);
	}

	SelectObject(dummyDC, oldfont);
	DeleteDC(dummyDC);
	return bitmap.valid();
}

class font_win : public osd_module, public font_module
{
public:
	font_win() : osd_module(OSD_FONT_PROVIDER, "win"), font_module()
	{
	}

	virtual int init(const osd_options &options) { return 0; }

	osd_font *font_alloc()
	{
		return global_alloc(osd_font_windows);
	}

};
#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(font_win, OSD_FONT_PROVIDER, "win")
#endif

MODULE_DEFINITION(FONT_WINDOWS, font_win)
