// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
/*
 * font_sdl.c
 *
 */

#include "font_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_UNIX) && !defined(SDLMAME_MACOSX) && !defined(SDLMAME_HAIKU) && !defined(SDLMAME_ANDROID)

#include "corestr.h"
#include "corealloc.h"
#include "fileio.h"
#include "unicode.h"

#ifdef SDLMAME_EMSCRIPTEN
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif
#if !defined(SDLMAME_HAIKU) && !defined(SDLMAME_EMSCRIPTEN)
#include <fontconfig/fontconfig.h>
#endif


//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

class osd_font_sdl : public osd_font
{
public:
	osd_font_sdl() : m_font(nullptr, &TTF_CloseFont) { }
	osd_font_sdl(osd_font_sdl &&obj) : m_font(std::move(obj.m_font)) { }
	virtual ~osd_font_sdl() { close(); }

	virtual bool open(std::string const &font_path, std::string const &name, int &height);
	virtual void close();
	virtual bool get_bitmap(char32_t chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs);

	osd_font_sdl & operator=(osd_font_sdl &&obj)
	{
		using std::swap;
		swap(m_font, obj.m_font);
		return *this;
	}

private:
	typedef std::unique_ptr<TTF_Font, void (*)(TTF_Font *)> TTF_Font_ptr;

	osd_font_sdl(osd_font_sdl const &) = delete;
	osd_font_sdl & operator=(osd_font_sdl const &) = delete;

	static constexpr double POINT_SIZE = 144.0;

#if !defined(SDLMAME_HAIKU) && !defined(SDLMAME_EMSCRIPTEN)
	TTF_Font_ptr search_font_config(std::string const &family, std::string const &style, bool &bakedstyles);
#endif
	bool BDF_Check_Magic(std::string const &name);
	TTF_Font_ptr TTF_OpenFont_Magic(std::string const &name, int fsize, long index);

	TTF_Font_ptr m_font;
};

bool osd_font_sdl::open(std::string const &font_path, std::string const &_name, int &height)
{
	bool bakedstyles = false;

	std::string name(_name);
	if (name.compare("default") == 0)
	{
		name = "Liberation Sans|Regular";
	}

	// accept qualifiers from the name
	bool const underline = (strreplace(name, "[U]", "") + strreplace(name, "[u]", "") > 0);
	bool const strike = (strreplace(name, "[S]", "") + strreplace(name, "[s]", "") > 0);
	std::string::size_type const separator = name.rfind('|');
	std::string const family(name.substr(0, separator));
	std::string const style((std::string::npos != separator) ? name.substr(separator + 1) : std::string());

	// first up, try it as a filename
	TTF_Font_ptr font = TTF_OpenFont_Magic(family, POINT_SIZE, 0);

	// if no success, try the font path
	if (!font)
	{
		osd_printf_verbose("Searching font %s in -%s path/s\n", family, font_path);
		//emu_file file(options().font_path(), OPEN_FLAG_READ);
		emu_file file(font_path.c_str(), OPEN_FLAG_READ);
		if (file.open(family.c_str()) == osd_file::error::NONE)
		{
			std::string full_name = file.fullpath();
			font = TTF_OpenFont_Magic(full_name, POINT_SIZE, 0);
			if (font)
				osd_printf_verbose("Found font %s\n", full_name);
		}
	}

	// if that didn't work, crank up the FontConfig database
#if !defined(SDLMAME_HAIKU) && !defined(SDLMAME_EMSCRIPTEN)
	if (!font)
	{
		font = search_font_config(family, style, bakedstyles);
	}
#endif

	if (!font)
	{
		if (!BDF_Check_Magic(name))
		{
			osd_printf_verbose("font %s is not TrueType or BDF, using MAME default\n", name);
		}
		return false;
	}

	// apply styles
	int styleflags = 0;
	if (!bakedstyles)
	{
		if ((style.find("Bold") != std::string::npos) || (style.find("Black") != std::string::npos)) styleflags |= TTF_STYLE_BOLD;
		if ((style.find("Italic") != std::string::npos) || (style.find("Oblique") != std::string::npos)) styleflags |= TTF_STYLE_ITALIC;
	}
	styleflags |= underline ? TTF_STYLE_UNDERLINE : 0;
	// SDL_ttf 2.0.9 and earlier does not define TTF_STYLE_STRIKETHROUGH
#if SDL_VERSIONNUM(TTF_MAJOR_VERSION, TTF_MINOR_VERSION, TTF_PATCHLEVEL) > SDL_VERSIONNUM(2,0,9)
	styleflags |= strike ? TTF_STYLE_STRIKETHROUGH : 0;
#else
	if (strike)
		osd_printf_warning("Ignoring strikethrough for SDL_TTF older than 2.0.10\n");
#endif // PATCHLEVEL
	TTF_SetFontStyle(font.get(), styleflags);

	height = TTF_FontLineSkip(font.get());

	m_font = std::move(font);
	return true;
}

//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_font_sdl::close()
{
	m_font.reset();
}

//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_font_sdl::get_bitmap(char32_t chnum, bitmap_argb32 &bitmap, std::int32_t &width, std::int32_t &xoffs, std::int32_t &yoffs)
{
	SDL_Color const fcol = { 0xff, 0xff, 0xff };
	char ustr[16];
	ustr[utf8_from_uchar(ustr, ARRAY_LENGTH(ustr), chnum)] = '\0';
	std::unique_ptr<SDL_Surface, void (*)(SDL_Surface *)> const drawsurf(TTF_RenderUTF8_Solid(m_font.get(), ustr, fcol), &SDL_FreeSurface);

	// was nothing returned?
	if (drawsurf)
	{
		// allocate a MAME destination bitmap
		bitmap.allocate(drawsurf->w, drawsurf->h);

		// copy the rendered character image into it
		for (int y = 0; y < bitmap.height(); y++)
		{
			std::uint32_t *const dstrow = &bitmap.pix(y);
			std::uint8_t const *const srcrow = reinterpret_cast<std::uint8_t const *>(drawsurf->pixels) + (y * drawsurf->pitch);

			for (int x = 0; x < drawsurf->w; x++)
			{
				dstrow[x] = srcrow[x] ? rgb_t(0xff, 0xff, 0xff, 0xff) : rgb_t(0x00, 0xff, 0xff, 0xff);
			}
		}

		// what are these?
		xoffs = yoffs = 0;
		width = drawsurf->w;
	}

	return bitmap.valid();
}

osd_font_sdl::TTF_Font_ptr osd_font_sdl::TTF_OpenFont_Magic(std::string const &name, int fsize, long index)
{
	emu_file file(OPEN_FLAG_READ);
	if (file.open(name.c_str()) == osd_file::error::NONE)
	{
		unsigned char const ttf_magic[] = { 0x00, 0x01, 0x00, 0x00, 0x00 };
		unsigned char const ttc1_magic[] = { 0x74, 0x74, 0x63, 0x66, 0x00, 0x01, 0x00, 0x00 };
		unsigned char const ttc2_magic[] = { 0x74, 0x74, 0x63, 0x66, 0x00, 0x02, 0x00, 0x00 };
		auto buffer_size = std::max({ sizeof(ttf_magic), sizeof(ttc1_magic), sizeof(ttc2_magic) });
		unsigned char buffer[buffer_size];
		auto const bytes_read = file.read(buffer, buffer_size);
		file.close();

		if (((bytes_read >= sizeof(ttf_magic)) && !std::memcmp(buffer, ttf_magic, sizeof(ttf_magic))) ||
			((bytes_read >= sizeof(ttc1_magic)) && !std::memcmp(buffer, ttc1_magic, sizeof(ttc1_magic))) ||
			((bytes_read >= sizeof(ttc2_magic)) && !std::memcmp(buffer, ttc2_magic, sizeof(ttc2_magic))))
			return TTF_Font_ptr(TTF_OpenFontIndex(name.c_str(), POINT_SIZE, index), &TTF_CloseFont);
	}
	return TTF_Font_ptr(nullptr, &TTF_CloseFont);
}

bool osd_font_sdl::BDF_Check_Magic(std::string const &name)
{
	emu_file file(OPEN_FLAG_READ);
	if (file.open(name.c_str()) == osd_file::error::NONE)
	{
		unsigned char const magic[] = { 'S', 'T', 'A', 'R', 'T', 'F', 'O', 'N', 'T' };
		unsigned char buffer[sizeof(magic)];
		if ((sizeof(magic) != file.read(buffer, sizeof(magic))) || memcmp(buffer, magic, sizeof(magic)))
			return true;
	}
	return false;
}

#if !defined(SDLMAME_HAIKU) && !defined(SDLMAME_EMSCRIPTEN)
osd_font_sdl::TTF_Font_ptr osd_font_sdl::search_font_config(std::string const &family, std::string const &style, bool &bakedstyles)
{
	TTF_Font_ptr font(nullptr, &TTF_CloseFont);

	FcConfig *const config = FcConfigGetCurrent();
	std::unique_ptr<FcPattern, void (*)(FcPattern *)> pat(FcPatternCreate(), &FcPatternDestroy);
	std::unique_ptr<FcObjectSet, void (*)(FcObjectSet *)> os(FcObjectSetCreate(), &FcObjectSetDestroy);
	FcPatternAddString(pat.get(), FC_FAMILY, (const FcChar8 *)family.c_str());

	// try and get a font with the requested styles baked-in
	if (!style.empty())
		FcPatternAddString(pat.get(), FC_STYLE, (const FcChar8 *)style.c_str());

	FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");

	FcObjectSetAdd(os.get(), FC_FILE);
	FcObjectSetAdd(os.get(), FC_INDEX);
	std::unique_ptr<FcFontSet, void (*)(FcFontSet *)> fontset(FcFontList(config, pat.get(), os.get()), &FcFontSetDestroy);

	for (int i = 0; (i < fontset->nfont) && !font; i++)
	{
		FcValue val;
		if ((FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) == FcResultMatch) && (val.type == FcTypeString))
		{
			osd_printf_verbose("Matching font: %s\n", val.u.s);

			std::string const match_name((const char*)val.u.s);
			long const index = ((FcPatternGet(fontset->fonts[i], FC_INDEX, 0, &val) == FcResultMatch) && (val.type == FcTypeInteger)) ? val.u.i : 0;
			font = TTF_OpenFont_Magic(match_name, POINT_SIZE, index);

			if (font)
				bakedstyles = true;
		}
	}

	// didn't get a font above?  try again with no baked-in styles
	// note that this simply returns the first match for the family name, which could be regular if you're lucky, but it could be bold oblique or something
	if (!font && !style.empty())
	{
		pat.reset(FcPatternCreate());
		FcPatternAddString(pat.get(), FC_FAMILY, (const FcChar8 *)family.c_str());
		FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");
		fontset.reset(FcFontList(config, pat.get(), os.get()));

		for (int i = 0; (i < fontset->nfont) && !font; i++)
		{
			FcValue val;
			if ((FcPatternGet(fontset->fonts[i], FC_FILE, 0, &val) == FcResultMatch) && (val.type == FcTypeString))
			{
				osd_printf_verbose("Matching unstyled font: %s\n", val.u.s);

				std::string const match_name((const char*)val.u.s);
				long const index = ((FcPatternGet(fontset->fonts[i], FC_INDEX, 0, &val) == FcResultMatch) && (val.type == FcTypeInteger)) ? val.u.i : 0;
				font = TTF_OpenFont_Magic(match_name, POINT_SIZE, index);

				if (font)
					bakedstyles = false;
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

	osd_font::ptr font_alloc() override
	{
		return std::make_unique<osd_font_sdl>();
	}

	virtual int init(const osd_options &options) override
	{
		if (TTF_Init() == -1)
		{
			osd_printf_error("SDL_ttf failed: %s\n", TTF_GetError());
			return -1;
		}
		return 0;
	}

	virtual void exit() override
	{
		TTF_Quit();
	}

	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) override;
};


bool font_sdl::get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result)
{
	result.clear();

	// TODO: enumerate TTF files in font path, since we can load them, too

#if !defined(SDLMAME_HAIKU) && !defined(SDLMAME_EMSCRIPTEN)
	FcConfig *const config = FcConfigGetCurrent();
	std::unique_ptr<FcPattern, void (*)(FcPattern *)> pat(FcPatternCreate(), &FcPatternDestroy);
	FcPatternAddString(pat.get(), FC_FONTFORMAT, (const FcChar8 *)"TrueType");

	std::unique_ptr<FcObjectSet, void (*)(FcObjectSet *)> os(FcObjectSetCreate(), &FcObjectSetDestroy);
	FcObjectSetAdd(os.get(), FC_FAMILY);
	FcObjectSetAdd(os.get(), FC_FILE);
	FcObjectSetAdd(os.get(), FC_STYLE);

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
				int const second = core_stricmp(a.second.c_str(), b.second.c_str());
				if (second < 0) return true;
				else if (second > 0) return false;
				else return core_stricmp(b.first.c_str(), b.first.c_str()) < 0;
			};
			std::string config((const char *)val.u.s);
			std::string display(config);
			if ((FcPatternGet(fontset->fonts[i], FC_STYLE, 0, &val) == FcResultMatch) && (val.type == FcTypeString))
			{
				config.push_back('|');
				config.append((const char *)val.u.s);
				display.push_back(' ');
				display.append((const char *)val.u.s);
			}
			std::pair<std::string, std::string> font(std::move(config), std::move(display));
			auto const pos = std::lower_bound(result.begin(), result.end(), font, compare_fonts);
			if ((result.end() == pos) || (pos->first != font.first)) result.emplace(pos, std::move(font));
		}
	}

	return true;
#else
	return false;
#endif
}

#else /* SDLMAME_UNIX */

MODULE_NOT_SUPPORTED(font_sdl, OSD_FONT_PROVIDER, "sdl")

#endif

MODULE_DEFINITION(FONT_SDL, font_sdl)
