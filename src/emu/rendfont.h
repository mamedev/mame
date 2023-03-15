// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    rendfont.h

    Rendering system font management.

***************************************************************************/

#ifndef MAME_EMU_RENDFONT_H
#define MAME_EMU_RENDFONT_H

#include "render.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> render_font

// a render_font describes and provides an interface to a font
class render_font
{
	friend class render_manager;

	// construction/destruction
	render_font(render_manager &manager, const char *filename);

public:
	virtual ~render_font();

	// getters
	render_manager &manager() const { return m_manager; }

	// size queries
	s32 pixel_height() const { return m_height; }
	float char_width(float height, float aspect, char32_t ch);
	float string_width(float height, float aspect, std::string_view string);
	float utf8string_width(float height, float aspect, std::string_view utf8string);

	// texture/bitmap queries
	render_texture *get_char_texture_and_bounds(float height, float aspect, char32_t ch, render_bounds &bounds);
	void get_scaled_bitmap_and_bounds(bitmap_argb32 &dest, float height, float aspect, char32_t chnum, rectangle &bounds);

private:
	// a glyph describes a single glyph
	class glyph
	{
	public:
		glyph()
			: width(-1)
			, xoffs(-1), yoffs(-1)
			, bmwidth(0), bmheight(0)
			, rawdata(nullptr)
			, texture(nullptr)
			, bitmap()
			, color()
		{
		}

		s32                 width;              // width from this character to the next
		s32                 xoffs, yoffs;       // X and Y offset from baseline to top,left of bitmap
		s32                 bmwidth, bmheight;  // width and height of bitmap
		const char *        rawdata;            // pointer to the raw data for this one
		render_texture *    texture;            // pointer to a texture for rendering and sizing
		bitmap_argb32       bitmap;             // pointer to the bitmap containing the raw data

		rgb_t               color;
	};

	// internal format
	enum class format
	{
		UNKNOWN,
		TEXT,
		CACHED,
		OSD
	};

	// helpers
	glyph &get_char(char32_t chnum);
	void char_expand(char32_t chnum, glyph &ch);
	bool load_cached_bdf(std::string_view filename);
	bool load_bdf();
	bool load_cached(util::random_read &file, u64 length, u32 hash);
	bool save_cached(util::random_write &file, u64 length, u32 hash);

	void render_font_command_glyph();

	// internal state
	render_manager &    m_manager;
	format              m_format;           // format of font data
	int                 m_height;           // height of the font, from ascent to descent
	int                 m_yoffs;            // y offset from baseline to descent
	int                 m_defchar;          // default substitute character
	float               m_scale;            // 1 / height precomputed
	glyph               *m_glyphs[17*256];  // array of glyph subtables
	std::vector<char>   m_rawdata;          // pointer to the raw data for the font
	u64                 m_rawsize;          // size of the raw font data
	std::unique_ptr<osd_font> m_osdfont;    // handle to the OSD font

	int                 m_height_cmd;       // height of the font, from ascent to descent
	int                 m_yoffs_cmd;        // y offset from baseline to descent
	EQUIVALENT_ARRAY(m_glyphs, glyph *) m_glyphs_cmd; // array of glyph subtables
	std::vector<char>   m_rawdata_cmd;      // pointer to the raw data for the font

	// constants
	static const u64 CACHED_BDF_HASH_SIZE   = 1024;
};

std::string convert_command_glyph(std::string_view str);

#endif  /* MAME_EMU_RENDFONT_H */
