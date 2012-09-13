#include "emu.h"
#include "includes/pcktgal.h"
#include "video/decbac06.h"

void pcktgal_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0;i < machine().total_colors();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + machine().total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + machine().total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + machine().total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + machine().total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine(),i,MAKE_RGB(r,g,b));
	}
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pcktgal_state *state = machine.driver_data<pcktgal_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram.bytes();offs += 4)
	{
		if (spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - spriteram[offs+2];
			sy = 240 - spriteram[offs];

			flipx = spriteram[offs+1] & 0x04;
			flipy = spriteram[offs+1] & 0x02;
			if (state->flip_screen()) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					spriteram[offs+3] + ((spriteram[offs+1] & 1) << 8),
					(spriteram[offs+1] & 0x70) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

SCREEN_UPDATE_IND16( pcktgal )
{
//  state->flip_screen_set(screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( pcktgalb )
{
	// the bootleg doesn't properly set the tilemap registers, because it's on non-original hardware, which probably doesn't have the flexible tilemaps.
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw_bootleg(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0, 2);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;}
