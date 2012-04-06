#include "emu.h"
#include "video/konicdev.h"
#include "includes/f1gp.h"


#define TOTAL_CHARS 0x800


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( f1gp_get_roz_tile_info )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	int code = state->m_rozvideoram[tile_index];

	SET_TILE_INFO(3, code & 0x7ff, code >> 12, 0);
}

static TILE_GET_INFO( f1gp2_get_roz_tile_info )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	int code = state->m_rozvideoram[tile_index];

	SET_TILE_INFO(2, (code & 0x7ff) + (state->m_roz_bank << 11), code >> 12, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	int code = state->m_fgvideoram[tile_index];

	SET_TILE_INFO(0, code & 0x7fff, 0, (code & 0x8000) ? TILE_FLIPY : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( f1gp )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();

	state->m_roz_tilemap = tilemap_create(machine, f1gp_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0xff);

	state->m_zoomdata = (UINT16 *)machine.region("gfx4")->base();
	gfx_element_set_source(machine.gfx[3], (UINT8 *)state->m_zoomdata);

//  state->save_pointer(NAME(state->m_zoomdata), machine.region("gfx4")->bytes());
}


VIDEO_START( f1gpb )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();

	state->m_roz_tilemap = tilemap_create(machine, f1gp_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0xff);

	state->m_zoomdata = (UINT16 *)machine.region("gfx4")->base();
	gfx_element_set_source(machine.gfx[3], (UINT8 *)state->m_zoomdata);

//  state->save_pointer(NAME(state->m_zoomdata), machine.region("gfx4")->bytes());
}

VIDEO_START( f1gp2 )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();

	state->m_roz_tilemap = tilemap_create(machine, f1gp2_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_fg_tilemap->set_transparent_pen(0xff);
	state->m_roz_tilemap->set_transparent_pen(0x0f);

	state->m_fg_tilemap->set_scrolldx(-80, 0);
	state->m_fg_tilemap->set_scrolldy(-26, 0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(f1gp_state::f1gp_zoomdata_r)
{
	return m_zoomdata[offset];
}

WRITE16_MEMBER(f1gp_state::f1gp_zoomdata_w)
{
	COMBINE_DATA(&m_zoomdata[offset]);
	gfx_element_mark_dirty(machine().gfx[3], offset / 64);
}

READ16_MEMBER(f1gp_state::f1gp_rozvideoram_r)
{
	return m_rozvideoram[offset];
}

WRITE16_MEMBER(f1gp_state::f1gp_rozvideoram_w)
{
	COMBINE_DATA(&m_rozvideoram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(f1gp_state::f1gp_fgvideoram_w)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(f1gp_state::f1gp_fgscroll_w)
{
	COMBINE_DATA(&m_scroll[offset]);

	m_fg_tilemap->set_scrollx(0, m_scroll[0]);
	m_fg_tilemap->set_scrolly(0, m_scroll[1]);
}

WRITE16_MEMBER(f1gp_state::f1gp_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_flipscreen = data & 0x20;
		m_gfxctrl = data & 0xdf;
	}
}

WRITE16_MEMBER(f1gp_state::f1gp2_gfxctrl_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_flipscreen = data & 0x20;

		/* bit 0/1 = fg/sprite/roz priority */
		/* bit 2 = blank screen */

		m_gfxctrl = data & 0xdf;
	}

	if (ACCESSING_BITS_8_15)
	{
		if (m_roz_bank != (data >> 8))
		{
			m_roz_bank = (data >> 8);
			m_roz_tilemap->mark_all_dirty();
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void f1gp_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int primask )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	int attr_start, first;
	UINT16 *spram = chip ? state->m_spr2vram : state->m_spr1vram;

	first = 4 * spram[0x1fe];

	for (attr_start = 0x0200 - 8; attr_start >= first; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color/*, pri*/;
		/* table hand made by looking at the ship explosion in attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spram[attr_start + 2] & 0x0080)) continue;

		ox = spram[attr_start + 1] & 0x01ff;
		xsize = (spram[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spram[attr_start + 1] & 0xf000) >> 12;
		oy = spram[attr_start + 0] & 0x01ff;
		ysize = (spram[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spram[attr_start + 0] & 0xf000) >> 12;
		flipx = spram[attr_start + 2] & 0x0800;
		flipy = spram[attr_start + 2] & 0x8000;
		color = (spram[attr_start + 2] & 0x000f);// + 16 * spritepalettebank;
		//pri = spram[attr_start + 2] & 0x0010;
		map_start = spram[attr_start + 3];

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->m_spr1cgram[map_start % (state->m_spr1cgram_size / 2)];
				else
					code = state->m_spr2cgram[map_start % (state->m_spr2cgram_size / 2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1 + chip],
						code,
						color,
						flipx,flipy,
						sx,sy,
						0x1000 * zoomx,0x1000 * zoomy,
						machine.priority_bitmap,
//                      pri ? 0 : 0x2);
						primask,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}


SCREEN_UPDATE_IND16( f1gp )
{
	f1gp_state *state = screen.machine().driver_data<f1gp_state>();

	screen.machine().priority_bitmap.fill(0, cliprect);

	k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_roz_tilemap, 0, 0, 1);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 1);

	/* quick kludge for "continue" screen priority */
	if (state->m_gfxctrl == 0x00)
	{
		f1gp_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0x02);
		f1gp_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0x02);
	}
	else
	{
		f1gp_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0x00);
		f1gp_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0x02);
	}
	return 0;
}


static void f1gpb_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	UINT16 *spriteram = state->m_spriteram;
	int attr_start, start_offset = state->m_spriteram_size / 2 - 4;

	// find the "end of list" to draw the sprites in reverse order
	for (attr_start = 4; attr_start < state->m_spriteram_size / 2; attr_start += 4)
	{
		if (spriteram[attr_start + 3 - 4] == 0xffff) /* end of list marker */
		{
			start_offset = attr_start - 4;
			break;
		}
	}

	for (attr_start = start_offset;attr_start >= 4;attr_start -= 4)
	{
		int code, gfx;
		int x, y, flipx, flipy, color, pri;

		x = (spriteram[attr_start + 2] & 0x03ff) - 48;
		y = (256 - (spriteram[attr_start + 3 - 4] & 0x03ff)) - 15;
		flipx = spriteram[attr_start + 1] & 0x0800;
		flipy = spriteram[attr_start + 1] & 0x8000;
		color = spriteram[attr_start + 1] & 0x000f;
		code = spriteram[attr_start + 0] & 0x3fff;
		pri = 0; //?

		if((spriteram[attr_start + 1] & 0x00f0) && (spriteram[attr_start + 1] & 0x00f0) != 0xc0)
		{
			printf("attr %X\n",spriteram[attr_start + 1] & 0x00f0);
			code = machine.rand();
		}

/*
        if (spriteram[attr_start + 1] & ~0x88cf)
            printf("1 = %X\n", spriteram[attr_start + 1] & ~0x88cf);
*/
		if(code >= 0x2000)
		{
			gfx = 1;
			code -= 0x2000;
		}
		else
		{
			gfx = 0;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x,y,
			machine.priority_bitmap,
			pri ? 0 : 0x2,15);

		// wrap around x
		pdrawgfx_transpen(bitmap,cliprect,machine.gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x - 512,y,
			machine.priority_bitmap,
			pri ? 0 : 0x2,15);
	}
}

SCREEN_UPDATE_IND16( f1gpb )
{
	f1gp_state *state = screen.machine().driver_data<f1gp_state>();
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	incxy = (INT16)state->m_rozregs[1];
	incyx = -incxy;
	incxx = incyy = (INT16)state->m_rozregs[3];
	startx = state->m_rozregs[0] + 328;
	starty = state->m_rozregs[2];

	state->m_fg_tilemap->set_scrolly(0, state->m_fgregs[0] + 8);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_roz_tilemap->draw_roz(bitmap, cliprect,
		startx << 13, starty << 13,
		incxx << 5, incxy << 5, incyx << 5, incyy << 5,
		1, 0, 0);

	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 1);

	f1gpb_draw_sprites(screen.machine(), bitmap, cliprect);

	return 0;
}


static void f1gp2_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	f1gp_state *state = machine.driver_data<f1gp_state>();
	int offs;

	offs = 0;
	while (offs < 0x0400 && (state->m_spritelist[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

		attr_start = 4 * (state->m_spritelist[offs++] & 0x01ff);

		ox = state->m_spritelist[attr_start + 1] & 0x01ff;
		xsize = (state->m_spritelist[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (state->m_spritelist[attr_start + 1] & 0xf000) >> 12;
		oy = state->m_spritelist[attr_start + 0] & 0x01ff;
		ysize = (state->m_spritelist[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (state->m_spritelist[attr_start + 0] & 0xf000) >> 12;
		flipx = state->m_spritelist[attr_start + 2] & 0x4000;
		flipy = state->m_spritelist[attr_start + 2] & 0x8000;
		color = (state->m_spritelist[attr_start + 2] & 0x1f00) >> 8;
		map_start = state->m_spritelist[attr_start + 3] & 0x7fff;

// aerofgt has the following adjustment, but doing it here would break the title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (state->m_spritelist[attr_start + 2] & 0x20ff)
			color = machine.rand();

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = state->m_sprcgram[map_start & 0x3fff];
				map_start++;

				if (state->m_flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							zoomx << 11,zoomy << 11,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[1],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11,zoomy << 11,15);
			}
		}
	}
}


SCREEN_UPDATE_IND16( f1gp2 )
{
	f1gp_state *state = screen.machine().driver_data<f1gp_state>();

	if (state->m_gfxctrl & 4)	/* blank screen */
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
	else
	{
		switch (state->m_gfxctrl & 3)
		{
			case 0:
				k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				f1gp2_draw_sprites(screen.machine(), bitmap, cliprect);
				state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
				break;
			case 1:
				k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
				f1gp2_draw_sprites(screen.machine(), bitmap, cliprect);
				break;
			case 2:
				state->m_fg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
				k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_roz_tilemap, 0, 0, 1);
				f1gp2_draw_sprites(screen.machine(), bitmap, cliprect);
				break;
#ifdef MAME_DEBUG
			case 3:
				popmessage("unsupported priority 3\n");
#endif
		}
	}
	return 0;
}
