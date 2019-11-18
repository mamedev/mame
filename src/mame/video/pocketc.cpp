// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "includes/pocketc.h"

// TODO: Convert to SVG rendering or internal layout

const rgb_t pocketc_state::indirect_palette[] =
{
	{  99, 107,  99 },
	{  94, 111, 103 },
	{ 255, 255, 255 },
	{ 255, 255, 255 },
	{  60,  66,  60 },
	{   0,   0,   0 }
};

const int pocketc_state::colortable[8][2] =
{
	{ 5, 4 },
	{ 5, 0 },
	{ 5, 2 },
	{ 4, 5 },
	{ 1, 4 },
	{ 0, 5 },
	{ 1, 5 },
	{ 3, 5 }
};

void pocketc_state::pocketc_palette(palette_device &palette) const
{
	for (int i = 0; i < ARRAY_LENGTH(indirect_palette); i++)
		palette.set_indirect_color(i, indirect_palette[i]);

	for (int i = 0; i < ARRAY_LENGTH(colortable); i++)
	{
		palette.set_pen_indirect(i*2,   colortable[i][0]);
		palette.set_pen_indirect(i*2+1, colortable[i][1]);
	}
}

void pocketc_state::pocketc_draw_special(bitmap_ind16 &bitmap, int x, int y, const char* const *fig, int color)
{
	for (int i = 0; i < 5; i++, y++)
		for (int j = 0; fig[i][j]; j++)
			if (fig[i][j] != ' ')
				bitmap.pix16(y, x + j) = color;
}
