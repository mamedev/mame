/***************************************************************************

    rendfont.h

    Rendering system font management.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef __RENDFONT_H__
#define __RENDFONT_H__

#include "render.h"


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
		INT32               width;              // width from this character to the next
		INT32               xoffs, yoffs;       // X and Y offset from baseline to top,left of bitmap
		INT32               bmwidth, bmheight;  // width and height of bitmap
		const char *        rawdata;            // pointer to the raw data for this one
		bitmap_argb32       bitmap;             // pointer to the bitmap containing the raw data
		render_texture *    texture;            // pointer to a texture for rendering and sizing
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
	bool save_cached(const char *filename, UINT32 hash);

	// internal state
	render_manager &    m_manager;
	format              m_format;           // format of font data
	int                 m_height;           // height of the font, from ascent to descent
	int                 m_yoffs;            // y offset from baseline to descent
	float               m_scale;            // 1 / height precomputed
	glyph *             m_glyphs[256];      // array of glyph subtables
	const char *        m_rawdata;          // pointer to the raw data for the font
	UINT64              m_rawsize;          // size of the raw font data
	osd_font            m_osdfont;          // handle to the OSD font

	// constants
	static const int CACHED_CHAR_SIZE       = 12;
	static const int CACHED_HEADER_SIZE     = 16;
	static const int CACHED_BDF_HASH_SIZE   = 1024;
};


#endif  /* __RENDFONT_H__ */
