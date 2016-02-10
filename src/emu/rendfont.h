// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    rendfont.h

    Rendering system font management.

***************************************************************************/

#ifndef __RENDFONT_H__
#define __RENDFONT_H__

#include "render.h"

// forward instead of include
class osd_font;

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> render_font

// a render_font describes and provides an interface to a font
class render_font
{
	friend class render_manager;
	friend resource_pool_object<render_font>::~resource_pool_object();

	// construction/destruction
	render_font(render_manager &manager, const char *filename);
	virtual ~render_font();

public:
	// getters
	render_manager &manager() const { return m_manager; }

	// size queries
	INT32 pixel_height() const { return m_height; }
	float char_width(float height, float aspect, unicode_char ch);
	float string_width(float height, float aspect, const char *string);
	float utf8string_width(float height, float aspect, const char *utf8string);

	// texture/bitmap queries
	render_texture *get_char_texture_and_bounds(float height, float aspect, unicode_char ch, render_bounds &bounds);
	void get_scaled_bitmap_and_bounds(bitmap_argb32 &dest, float height, float aspect, unicode_char chnum, rectangle &bounds);

private:
	// a glyph describes a single glyph
	class glyph
	{
	public:
		glyph()
			: width(0),
				xoffs(0), yoffs(0),
				bmwidth(0), bmheight(0),
				rawdata(nullptr),
				texture(nullptr) { }

		INT32               width;              // width from this character to the next
		INT32               xoffs, yoffs;       // X and Y offset from baseline to top,left of bitmap
		INT32               bmwidth, bmheight;  // width and height of bitmap
		const char *        rawdata;            // pointer to the raw data for this one
		render_texture *    texture;            // pointer to a texture for rendering and sizing
		bitmap_argb32       bitmap;             // pointer to the bitmap containing the raw data

		rgb_t               color;

	};

	// internal format
	enum format
	{
		FF_UNKNOWN,
		FF_TEXT,
		FF_CACHED,
		FF_OSD
	};

	// helpers
	glyph &get_char(unicode_char chnum);
	void char_expand(unicode_char chnum, glyph &ch);
	bool load_cached_bdf(const char *filename);
	bool load_bdf();
	bool load_cached(emu_file &file, UINT32 hash);
	bool load_cached_cmd(emu_file &file, UINT32 hash);
	bool save_cached(const char *filename, UINT32 hash);

	void render_font_command_glyph();

	// internal state
	render_manager &    m_manager;
	format              m_format;           // format of font data
	int                 m_height;           // height of the font, from ascent to descent
	int                 m_yoffs;            // y offset from baseline to descent
	float               m_scale;            // 1 / height precomputed
	glyph               *m_glyphs[256];      // array of glyph subtables
	std::vector<char>   m_rawdata;          // pointer to the raw data for the font
	UINT64              m_rawsize;          // size of the raw font data
	osd_font            *m_osdfont;          // handle to the OSD font

	int                 m_height_cmd;       // height of the font, from ascent to descent
	int                 m_yoffs_cmd;        // y offset from baseline to descent
	glyph               *m_glyphs_cmd[256];  // array of glyph subtables
	std::vector<char>   m_rawdata_cmd;      // pointer to the raw data for the font

	// constants
	static const int CACHED_CHAR_SIZE       = 12;
	static const int CACHED_HEADER_SIZE     = 16;
	static const int CACHED_BDF_HASH_SIZE   = 1024;
};

void convert_command_glyph(std::string &s);

#endif  /* __RENDFONT_H__ */
