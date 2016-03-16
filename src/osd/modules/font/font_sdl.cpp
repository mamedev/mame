// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*
 * font_sdl.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_UNIX) && (!defined(SDLMAME_MACOSX)) && (!defined(SDLMAME_SOLARIS)) && (!defined(SDLMAME_HAIKU)) && (!defined(SDLMAME_EMSCRIPTEN))

#include <SDL2/SDL_ttf.h>
#ifndef SDLMAME_HAIKU
#include <fontconfig/fontconfig.h>
#endif


#include "corestr.h"
#include "corealloc.h"
#include "fileio.h"


//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_sdl : public osd_font
{
public:
	osd_font_sdl() : m_font(nullptr) { }
	osd_font_sdl(osd_font_sdl &&obj) : m_font(obj.m_font) { obj.m_font = nullptr; }
	virtual ~osd_font_sdl() { close(); }

	virtual bool open(std::string const &font_path, std::string const &name, int &height);
	virtual void close();
	virtual bool get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs);

	osd_font_sdl & operator=(osd_font_sdl &&obj)
	{
		using std::swap;
		swap(m_font, obj.m_font);
		return *this;
	}

private:
	osd_font_sdl(osd_font_sdl const &) = delete;
	osd_font_sdl & operator=(osd_font_sdl const &) = delete;

	static constexpr double POINT_SIZE = 144.0;

#ifndef SDLMAME_HAIKU
	TTF_Font *search_font_config(std::string const &name, bool bold, bool italic, bool underline, bool &bakedstyles);
#endif
	bool BDF_Check_Magic(std::string const &name);
	TTF_Font * TTF_OpenFont_Magic(std::string const &name, int fsize);

	TTF_Font *m_font;
};

bool osd_font_sdl::open(std::string const &font_path, std::string const &_name, int &height)
{
	TTF_Font *font = nullptr;
	bool bakedstyles = false;
	int style = 0;

	// accept qualifiers from the name
	std::string name(_name);
	if (name.compare("default") == 0)
	{
		name = "Liberation Sans";
	}

	bool const bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	bool const italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);
	bool const underline = (strreplace(name, "[U]", "") + strreplace(name, "[u]", "") > 0);
	bool const strike = (strreplace(name, "[S]", "") + strreplace(name, "[s]", "") > 0);

	// first up, try it as a filename
	font = TTF_OpenFont_Magic(name, POINT_SIZE);

	// if no success, try the font path

	if (!font)
	{
		osd_printf_verbose("Searching font %s in -%s\n", name.c_str(), OPTION_FONTPATH);
		//emu_file file(options().font_path(), OPEN_FLAG_READ);
		emu_file file(font_path.c_str(), OPEN_FLAG_READ);
		if (file.open(name.c_str()) == osd_file::error::NONE)
		{
			std::string full_name = file.fullpath();
			font = TTF_OpenFont_Magic(full_name, POINT_SIZE);
			if (font)
				osd_printf_verbose("Found font %s\n", full_name.c_str());
		}
	}

	// if that didn't work, crank up the FontConfig database
#ifndef SDLMAME_HAIKU
	if (!font)
	{
		font = search_font_config(name, bold, italic, underline, bakedstyles);
	}
#endif

	if (!font)
	{
		if (!BDF_Check_Magic(name))
		{
			osd_printf_verbose("font %s is not TrueType or BDF, using MAME default\n", name.c_str());
		}
		return false;
	}

	// apply styles
	if (!bakedstyles)
	{
		style |= bold ? TTF_STYLE_BOLD : 0;
		style |= italic ? TTF_STYLE_ITALIC : 0;
	}
	style |= underline ? TTF_STYLE_UNDERLINE : 0;
	// SDL_ttf 2.0.9 and earlier does not define TTF_STYLE_STRIKETHROUGH
#if SDL_VERSIONNUM(TTF_MAJOR_VERSION, TTF_MINOR_VERSION, TTF_PATCHLEVEL) > SDL_VERSIONNUM(2,0,9)
	style |= strike ? TTF_STYLE_STRIKETHROUGH : 0;
#else
	if (strike)
		osd_printf_warning("Ignoring strikethrough for SDL_TTF older than 2.0.10\n");
#endif // PATCHLEVEL
	TTF_SetFontStyle(font, style);

	height = TTF_FontLineSkip(font);

	close();
	m_font = font;
	return true;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_sdl::close()
{
	if (m_font)
		TTF_CloseFont(m_font);
	m_font = nullptr;
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_sdl::get_bitmap(unicode_char chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs)
{
	TTF_Font *ttffont;
	SDL_Surface *drawsurf;
	SDL_Color fcol = { 0xff, 0xff, 0xff };
	std::uint16_t ustr[16];

	ttffont = m_font;

	memset(ustr,0,sizeof(ustr));
	ustr[0] = (std::uint16_t)chnum;
	drawsurf = TTF_RenderUNICODE_Solid(ttffont, ustr, fcol);

	// was nothing returned?
	if (drawsurf)
	{
		// allocate a MAME destination bitmap
		bitmap.allocate(drawsurf->w, drawsurf->h);

		// copy the rendered character image into it
		for (int y = 0; y < bitmap.height(); y++)
		{
			std::uint32_t *dstrow = &bitmap.pix32(y);
			std::uint8_t *srcrow = (std::uint8_t *)drawsurf->pixels;

			srcrow += (y * drawsurf->pitch);

			for (int x = 0; x < drawsurf->w; x++)
			{
				dstrow[x] = srcrow[x] ? rgb_t(0xff,0xff,0xff,0xff) : rgb_t(0x00,0xff,0xff,0xff);
			}
		}

		// what are these?
		xoffs = yoffs = 0;
		width = drawsurf->w;

		SDL_FreeSurface(drawsurf);
	}

	return bitmap.valid();
}

TTF_Font * osd_font_sdl::TTF_OpenFont_Magic(std::string const &name, int fsize)
{
	emu_file file(OPEN_FLAG_READ);
	if (file.open(name.c_str()) == osd_file::error::NONE)
	{
		unsigned char buffer[5] = { 0xff, 0xff, 0xff, 0xff, 0xff };
		unsigned char magic[5] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
		file.read(buffer,5);
		if (memcmp(buffer, magic, 5))
			return nullptr;
	}
	return TTF_OpenFont(name.c_str(), POINT_SIZE);
}

bool osd_font_sdl::BDF_Check_Magic(std::string const &name)
{
	emu_file file(OPEN_FLAG_READ);
	if (file.open(name.c_str()) == osd_file::error::NONE)
	{
		unsigned char buffer[9];
		unsigned char magic[9] = { 'S', 'T', 'A', 'R', 'T', 'F', 'O', 'N', 'T' };
		file.read(buffer, 9);
		file.close();
		if (!memcmp(buffer, magic, 9))
			return true;
	}

	return false;
}

#ifndef SDLMAME_HAIKU
TTF_Font *osd_font_sdl::search_font_config(std::string const &name, bool bold, bool italic, bool underline, bool &bakedstyles)
{
	TTF_Font *font = nullptr;
	FcValue val;

	FcConfig *const config = FcConfigGetCurrent();
	std::unique_ptr<FcPattern, void (*)(FcPattern *)> pat(FcPatternCreate(), &FcPatternDestroy);
	std::unique_ptr<FcObjectSet, void (*)(FcObjectSet *)> os(FcObjectSetCreate(), &FcObjectSetDestroy);
	FcPatternAddString(pat.get(), FC_FAMILY, (const FcChar8 *)name.c_str());

	// try and get a font with the requested styles baked-in
	if (bold)
	{
		if (italic)
			FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)"Bold Italic");
		else
			FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)"Bold");
	}
	else if (italic)
	{
		FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)"Italic");
	}
	else
	{
		FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)"Regular");
	}

	FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");

	FcObjectSetAdd(os.get(), FC_FILE);
	std::unique_ptr<FcFontSet, void (*)(FcFontSet *)> fontset(FcFontList(config, pat.get(), os.get()), &FcFontSetDestroy);

	for (int i = 0; i < fontset->nfont; i++)
	{
		if ((FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) != FcResultMatch) ||
			(val.type != FcTypeString))
		{
			continue;
		}

		osd_printf_verbose("Matching font: %s\n", val.u.s);
		{
			std::string match_name((const char*)val.u.s);
			font = TTF_OpenFont_Magic(match_name, POINT_SIZE);
		}

		if (font)
		{
			bakedstyles = true;
			break;
		}
	}

	// didn't get a font above?  try again with no baked-in styles
	if (!font)
	{
		pat.reset(FcPatternCreate());
		FcPatternAddString(pat.get(), FC_FAMILY, (const FcChar8 *)name.c_str());
		FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)"Regular");
		FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");
		fontset.reset(FcFontList(config, pat.get(), os.get()));

		for (int i = 0; (i < fontset->nfont) && !font; i++)
		{
			if ((FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) == FcResultMatch) &&
				(val.type == FcTypeString))
			{
				osd_printf_verbose("Matching unstyled font: %s\n", val.u.s);

				std::string const match_name((const char *)val.u.s);
				font = TTF_OpenFont_Magic(match_name, POINT_SIZE);
			}
		}
	}

	return font;
}
#endif


class font_sdl : public osd_module, public font_module
{
public:
	font_sdl() : osd_module(OSD_FONT_PROVIDER, "sdl"), font_module()
	{
	}

	osd_font::ptr font_alloc()
	{
		return std::make_unique<osd_font_sdl>();
	}

	virtual int init(const osd_options &options)
	{
		if (TTF_Init() == -1)
		{
			osd_printf_error("SDL_ttf failed: %s\n", TTF_GetError());
			return -1;
		}
		return 0;
	}

	virtual void exit()
	{
		TTF_Quit();
	}

	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) override;
};

bool font_sdl::get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result)
{
	result.clear();

	// TODO: enumerate TTF files in font path, since we can load them, too

	FcConfig *const config = FcConfigGetCurrent();
	std::unique_ptr<FcPattern, void (*)(FcPattern *)> pat(FcPatternCreate(), &FcPatternDestroy);
	FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");

	std::unique_ptr<FcObjectSet, void (*)(FcObjectSet *)> os(FcObjectSetCreate(), &FcObjectSetDestroy);
	FcObjectSetAdd(os.get(), FC_FAMILY);
	FcObjectSetAdd(os.get(), FC_FILE);

	std::unique_ptr<FcFontSet, void (*)(FcFontSet *)> fontset(FcFontList(config, pat.get(), os.get()), &FcFontSetDestroy);
	for (int i = 0; (i < fontset->nfont); i++)
	{
		FcValue val;
		if ((FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) == FcResultMatch) &&
			(val.type == FcTypeString) &&
			(FcPatternGet(fontset->fonts[i], FC_FAMILY, 0, &val) == FcResultMatch) &&
			(val.type == FcTypeString))
		{
			auto const compare_fonts = [](std::pair<std::string, std::string> const &a, std::pair<std::string, std::string> const &b) -> bool
			{
				int const first = core_stricmp(a.first.c_str(), b.first.c_str());
				if (first < 0) return true;
				else if (first > 0) return false;
				else return core_stricmp(b.second.c_str(), b.second.c_str()) < 0;
			};
			std::pair<std::string, std::string> font((const char *)val.u.s, (const char *)val.u.s);
			auto const pos = std::lower_bound(result.begin(), result.end(), font, compare_fonts);
			if ((result.end() == pos) || (pos->first != font.first)) result.emplace(pos, std::move(font));
		}
	}

	return true;
}

#else /* SDLMAME_UNIX */

MODULE_NOT_SUPPORTED(font_sdl, OSD_FONT_PROVIDER, "sdl")

#endif

MODULE_DEFINITION(FONT_SDL, font_sdl)
