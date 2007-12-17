/***************************************************************************

    rendfont.h

    Rendering system font management.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RENDFONT_H__
#define __RENDFONT_H__

#include "render.h"
#include "unicode.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

render_font *render_font_alloc(const char *filename);
void render_font_free(render_font *font);
INT32 render_font_get_pixel_height(render_font *font);
render_texture *render_font_get_char_texture_and_bounds(render_font *font, float height, float aspect, unicode_char ch, render_bounds *bounds);
void render_font_get_scaled_bitmap_and_bounds(render_font *font, mame_bitmap *dest, float height, float aspect, unicode_char chnum, rectangle *bounds);
float render_font_get_char_width(render_font *font, float height, float aspect, unicode_char ch);
float render_font_get_string_width(render_font *font, float height, float aspect, const char *string);
float render_font_get_utf8string_width(render_font *font, float height, float aspect, const char *utf8string);

#endif	/* __RENDFONT_H__ */
