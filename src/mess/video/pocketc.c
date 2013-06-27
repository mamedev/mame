#include "emu.h"
#include "includes/pocketc.h"

/* PC126x
   24x2 5x7 space between char
   2000 .. 203b, 2800 .. 283b
   2040 .. 207b, 2840 .. 287b
  203d: 0 BUSY, 1 PRINT, 3 JAPAN, 4 SMALL, 5 SHIFT, 6 DEF
  207c: 1 DEF 1 RAD 2 GRAD 5 ERROR 6 FLAG */

static const UINT8 pocketc_palette[] =
{
	99,107,99,
	94,111,103,
	255,255,255,
	255,255,255,
	60, 66, 60,
	0, 0, 0
};

const unsigned short pocketc_colortable[8][2] = {
	{ 5, 4 },
	{ 5, 0 },
	{ 5, 2 },
	{ 4, 5 },
	{ 1, 4 },
	{ 0, 5 },
	{ 1, 5 },
	{ 3, 5 }
};

PALETTE_INIT( pocketc )
{
	UINT8 i=0, r, b, g, color_count = 6;

	machine.colortable = colortable_alloc(machine, color_count);

	while (color_count--)
	{
		r = pocketc_palette[i++]; g = pocketc_palette[i++]; b = pocketc_palette[i++];
		colortable_palette_set_color(machine.colortable, 5 - color_count, MAKE_RGB(r, g, b));
	}

	for( i = 0; i < 8; i++ )
	{
		colortable_entry_set_value(machine.colortable, i*2, pocketc_colortable[i][0]);
		colortable_entry_set_value(machine.colortable, i*2+1, pocketc_colortable[i][1]);
	}
}


/* Draw an indicator (DEG, SHIFT, etc) */
void pocketc_draw_special(bitmap_ind16 &bitmap, int x, int y, const POCKETC_FIGURE fig, int color)
{
	int i,j;
	for (i=0; fig[i]; i++, y++)
	{
		for (j=0; fig[i][j]!=0; j++)
		{
			switch(fig[i][j])
			{
			case '1':
				bitmap.pix16(y, x+j) = color;
				break;
			case 'e':
				return;
			}
		}
	}
}
