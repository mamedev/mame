/***************************************************************************

Functions to emulate the video hardware of the machine.

***************************************************************************/


#include "emu.h"
#include "includes/srmp2.h"
#include "video/seta001.h"

PALETTE_INIT( srmp2 )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int col;

		col = (color_prom[i] << 8) + color_prom[i + machine.total_colors()];
		palette_set_color_rgb(machine,i ^ 0x0f,pal5bit(col >> 10),pal5bit(col >> 5),pal5bit(col >> 0));
	}
}


PALETTE_INIT( srmp3 )
{
	int i;

	for (i = 0; i < machine.total_colors(); i++)
	{
		int col;

		col = (color_prom[i] << 8) + color_prom[i + machine.total_colors()];
		palette_set_color_rgb(machine,i,pal5bit(col >> 10),pal5bit(col >> 5),pal5bit(col >> 0));
	}
}


SCREEN_UPDATE( srmp2 )
{
	srmp2_state *state = screen->machine().driver_data<srmp2_state>();
	bitmap_fill(bitmap, cliprect, 0x1ff);
	screen->machine().device<seta001_device>("spritegen")->srmp2_draw_sprites(screen->machine(), bitmap, cliprect, state->m_color_bank); 
	return 0;
}

SCREEN_UPDATE( srmp3 )
{
	srmp2_state *state = screen->machine().driver_data<srmp2_state>();
	bitmap_fill(bitmap, cliprect, 0x1f0);
	screen->machine().device<seta001_device>("spritegen")->srmp3_draw_sprites(screen->machine(), bitmap, cliprect, state->m_gfx_bank);
	return 0;
}

SCREEN_UPDATE( mjyuugi )
{
	srmp2_state *state = screen->machine().driver_data<srmp2_state>();
	bitmap_fill(bitmap, cliprect, 0x1f0);
	screen->machine().device<seta001_device>("spritegen")->mjyuugi_draw_sprites(screen->machine(), bitmap, cliprect, state->m_gfx_bank);
	return 0;
}
