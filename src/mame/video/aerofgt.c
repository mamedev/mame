#include "emu.h"
#include "includes/aerofgt.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pspikes_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg1videoram[tile_index];
	int bank = (code & 0x1000) >> 12;
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (state->m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + 8 * state->m_charpalettebank,
			0);
}

static TILE_GET_INFO( karatblz_bg1_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (state->m_gfxbank[0] << 13),
			(code & 0xe000) >> 13,
			0);
}

/* also spinlbrk */
static TILE_GET_INFO( karatblz_bg2_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg2videoram[tile_index];
	SET_TILE_INFO(
			1,
			(code & 0x1fff) + (state->m_gfxbank[1] << 13),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( spinlbrk_bg1_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg1videoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (state->m_gfxbank[0] << 12),
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg1videoram[tile_index];
	int bank = (code & 0x1800) >> 11;
	SET_TILE_INFO(
			0,
			(code & 0x07ff) + (state->m_gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	UINT16 code = state->m_bg2videoram[tile_index];
	int bank = 4 + ((code & 0x1800) >> 11);
	SET_TILE_INFO(
			1,
			(code & 0x07ff) + (state->m_gfxbank[bank] << 11),
			(code & 0xe000) >> 13,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void aerofgt_register_state_globals( running_machine &machine )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	state->save_item(NAME(state->m_gfxbank));
	state->save_item(NAME(state->m_bank));
	state->save_item(NAME(state->m_bg1scrollx));
	state->save_item(NAME(state->m_bg1scrolly));
	state->save_item(NAME(state->m_bg2scrollx));
	state->save_item(NAME(state->m_bg2scrolly));
	state->save_item(NAME(state->m_charpalettebank));
	state->save_item(NAME(state->m_spritepalettebank));
}

VIDEO_START( pspikes )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	state->m_bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info,tilemap_scan_rows,8,8,64,32);
	/* no bg2 in this game */

	state->m_sprite_gfx = 1;

	aerofgt_register_state_globals(machine);
	state->save_item(NAME(state->m_spikes91_lookup));
}


VIDEO_START( karatblz )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	state->m_bg1_tilemap = tilemap_create(machine, karatblz_bg1_tile_info,tilemap_scan_rows,     8,8,64,64);
	state->m_bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info,tilemap_scan_rows,8,8,64,64);

	state->m_bg2_tilemap->set_transparent_pen(15);
	state->m_spritepalettebank = 0;
	state->m_sprite_gfx = 2;

	aerofgt_register_state_globals(machine);
}

VIDEO_START( spinlbrk )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int i;

	state->m_bg1_tilemap = tilemap_create(machine, spinlbrk_bg1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_bg2_tilemap = tilemap_create(machine, karatblz_bg2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	state->m_bg2_tilemap->set_transparent_pen(15);

	state->m_spritepalettebank = 0;
	state->m_sprite_gfx = 2;

	/* sprite maps are hardcoded in this game */

	/* enemy sprites use ROM instead of RAM */
	state->m_spriteram2 = (UINT16 *)machine.region("gfx5")->base();
	state->m_spriteram2_size = 0x20000;

	/* front sprites are direct maps */
	state->m_spriteram1 = state->m_spriteram2 + state->m_spriteram2_size / 2;
	state->m_spriteram1_size = 0x4000;

	for (i = 0; i < state->m_spriteram1_size / 2; i++)
	{
		state->m_spriteram1[i] = i;
	}

	aerofgt_register_state_globals(machine);
}

VIDEO_START( turbofrc )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	state->m_bg1_tilemap = tilemap_create(machine, get_bg1_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->m_bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	state->m_bg2_tilemap->set_transparent_pen(15);

	state->m_spritepalettebank = 0;
	state->m_sprite_gfx = 2;

	aerofgt_register_state_globals(machine);
}

VIDEO_START( wbbc97 )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	state->m_bg1_tilemap = tilemap_create(machine, get_pspikes_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	/* no bg2 in this game */

	state->m_bg1_tilemap->set_transparent_pen(15);

	state->m_sprite_gfx = 1;

	aerofgt_register_state_globals(machine);

	state->save_item(NAME(state->m_wbbc97_bitmap_enable));
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(aerofgt_state::aerofgt_bg1videoram_w)
{
	COMBINE_DATA(&m_bg1videoram[offset]);
	m_bg1_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_bg2videoram_w)
{
	COMBINE_DATA(&m_bg2videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset);
}


static void setbank( running_machine &machine, tilemap_t *tmap, int num, int bank )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	if (state->m_gfxbank[num] != bank)
	{
		state->m_gfxbank[num] = bank;
		tmap->mark_all_dirty();
	}
}

WRITE16_MEMBER(aerofgt_state::pspikes_gfxbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(machine(), m_bg1_tilemap, 0, (data & 0xf0) >> 4);
		setbank(machine(), m_bg1_tilemap, 1, data & 0x0f);
	}
}

WRITE16_MEMBER(aerofgt_state::pspikesb_gfxbank_w)
{
	COMBINE_DATA(&m_rasterram[0x200 / 2]);

	setbank(machine(), m_bg1_tilemap, 0, (data & 0xf000) >> 12);
	setbank(machine(), m_bg1_tilemap, 1, (data & 0x0f00) >> 8);
}

WRITE16_MEMBER(aerofgt_state::spikes91_lookup_w)
{
	m_spikes91_lookup = data & 1;
}

WRITE16_MEMBER(aerofgt_state::karatblz_gfxbank_w)
{
	if (ACCESSING_BITS_8_15)
	{
		setbank(machine(), m_bg1_tilemap, 0, (data & 0x0100) >> 8);
		setbank(machine(), m_bg2_tilemap, 1, (data & 0x0800) >> 11);
	}
}

WRITE16_MEMBER(aerofgt_state::spinlbrk_gfxbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(machine(), m_bg1_tilemap, 0, (data & 0x07));
		setbank(machine(), m_bg2_tilemap, 1, (data & 0x38) >> 3);
	}
}

WRITE16_MEMBER(aerofgt_state::turbofrc_gfxbank_w)
{
	tilemap_t *tmap = (offset == 0) ? m_bg1_tilemap : m_bg2_tilemap;

	data = COMBINE_DATA(&m_bank[offset]);

	setbank(machine(), tmap, 4 * offset + 0, (data >> 0) & 0x0f);
	setbank(machine(), tmap, 4 * offset + 1, (data >> 4) & 0x0f);
	setbank(machine(), tmap, 4 * offset + 2, (data >> 8) & 0x0f);
	setbank(machine(), tmap, 4 * offset + 3, (data >> 12) & 0x0f);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_gfxbank_w)
{
	tilemap_t *tmap = (offset < 2) ? m_bg1_tilemap : m_bg2_tilemap;

	data = COMBINE_DATA(&m_bank[offset]);

	setbank(machine(), tmap, 2 * offset + 0, (data >> 8) & 0xff);
	setbank(machine(), tmap, 2 * offset + 1, (data >> 0) & 0xff);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_bg1scrollx_w)
{
	COMBINE_DATA(&m_bg1scrollx);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_bg1scrolly_w)
{
	COMBINE_DATA(&m_bg1scrolly);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_bg2scrollx_w)
{
	COMBINE_DATA(&m_bg2scrollx);
}

WRITE16_MEMBER(aerofgt_state::aerofgt_bg2scrolly_w)
{
	COMBINE_DATA(&m_bg2scrolly);
}

WRITE16_MEMBER(aerofgt_state::pspikes_palette_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_spritepalettebank = data & 0x03;
		if (m_charpalettebank != (data & 0x1c) >> 2)
		{
			m_charpalettebank = (data & 0x1c) >> 2;
			m_bg1_tilemap->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(aerofgt_state::wbbc97_bitmap_enable_w)
{
	COMBINE_DATA(&m_wbbc97_bitmap_enable);
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void aerofgt_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int offs;
	priority <<= 12;

	offs = 0;
	while (offs < 0x0400 && (state->m_spriteram3[offs] & 0x8000) == 0)
	{
		int attr_start = 4 * (state->m_spriteram3[offs] & 0x03ff);

		/* is the way I handle priority correct? Or should I just check bit 13? */
		if ((state->m_spriteram3[attr_start + 2] & 0x3000) == priority)
		{
			int map_start;
			int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

			ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
			xsize = (state->m_spriteram3[attr_start + 1] & 0x0e00) >> 9;
			zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
			oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
			ysize = (state->m_spriteram3[attr_start + 0] & 0x0e00) >> 9;
			zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
			flipx = state->m_spriteram3[attr_start + 2] & 0x4000;
			flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
			color = (state->m_spriteram3[attr_start + 2] & 0x0f00) >> 8;
			map_start = state->m_spriteram3[attr_start + 3] & 0x3fff;

			ox += (xsize * zoomx + 2) / 4;
			oy += (ysize * zoomy + 2) / 4;

			zoomx = 32 - zoomx;
			zoomy = 32 - zoomy;

			for (y = 0; y <= ysize; y++)
			{
				int sx, sy;

				if (flipy)
					sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
				else
					sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

				for (x = 0; x <= xsize; x++)
				{
					int code;

					if (flipx)
						sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
					else
						sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

					if (map_start < 0x2000)
						code = state->m_spriteram1[map_start & 0x1fff] & 0x1fff;
					else
						code = state->m_spriteram2[map_start & 0x1fff] & 0x1fff;

					drawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + (map_start >= 0x2000 ? 1 : 0)],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11, zoomy << 11,15);
					map_start++;
				}
			}
		}
		offs++;
	}
}

template<class _BitmapClass>
static void turbofrc_draw_sprites( running_machine &machine, _BitmapClass &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * state->m_spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200 - 8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->m_spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->m_spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;

		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->m_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->m_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->m_spriteram3[attr_start + 2] & 0x0800;
		flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
		color = (state->m_spriteram3[attr_start + 2] & 0x000f) + 16 * state->m_spritepalettebank;

		map_start = state->m_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->m_spriteram1[map_start % (state->m_spriteram1_size/2)];
				else
					code = state->m_spriteram2[map_start % (state->m_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine.priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void spinlbrk_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int attr_start, base, first;
	base = chip * 0x0200;
	first = 4 * state->m_spriteram3[0x1fe + base];

	for (attr_start = base + 0x0200-8; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->m_spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->m_spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;
		if ((!chip_disabled_pri) & (pri >> 4))
			continue;

		ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->m_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->m_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->m_spriteram3[attr_start + 2] & 0x0800;
		flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
		color = (state->m_spriteram3[attr_start + 2] & 0x000f) + 16 * state->m_spritepalettebank;

		map_start = state->m_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->m_spriteram1[map_start % (state->m_spriteram1_size/2)];
				else
					code = state->m_spriteram2[map_start % (state->m_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine.priority_bitmap,pri ? 2 : 0,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void aerfboo2_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int attr_start, base, first;

	base = chip * 0x0200;
//  first = 4 * state->m_spriteram3[0x1fe + base];
	first = 0;

	for (attr_start = base + 0x0200 - 4; attr_start >= first + base; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color, pri;
// some other drivers still use this wrong table, they have to be upgraded
//      int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(state->m_spriteram3[attr_start + 2] & 0x0080))
			continue;

		pri = state->m_spriteram3[attr_start + 2] & 0x0010;

		if ( chip_disabled_pri & !pri)
			continue;
		if ((!chip_disabled_pri) & (pri >> 4))
			continue;
		ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
		xsize = (state->m_spriteram3[attr_start + 2] & 0x0700) >> 8;
		zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
		oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
		ysize = (state->m_spriteram3[attr_start + 2] & 0x7000) >> 12;
		zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
		flipx = state->m_spriteram3[attr_start + 2] & 0x0800;
		flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
		color = (state->m_spriteram3[attr_start + 2] & 0x000f) + 16 * state->m_spritepalettebank;

		map_start = state->m_spriteram3[attr_start + 3];

// aerofgt has this adjustment, but doing it here would break turbo force title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy)
				sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else
				sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx)
					sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->m_spriteram1[map_start % (state->m_spriteram1_size/2)];
				else
					code = state->m_spriteram2[map_start % (state->m_spriteram2_size/2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + chip],
							 code,
							 color,
							 flipx,flipy,
							 sx,sy,
							 zoomx << 11, zoomy << 11,
							 machine.priority_bitmap,pri ? 0 : 2,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}

static void pspikesb_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int i;

	for (i = 4; i < state->m_spriteram3_size / 2; i += 4)
	{
		int xpos, ypos, color, flipx, flipy, code;

		if (state->m_spriteram3[i + 3 - 4] & 0x8000)
			break;

		xpos = (state->m_spriteram3[i + 2] & 0x1ff) - 34;
		ypos = 256 - (state->m_spriteram3[i + 3 - 4] & 0x1ff) - 33;
		code = state->m_spriteram3[i + 0] & 0x1fff;
		flipy = 0;
		flipx = state->m_spriteram3[i + 1] & 0x0800;
		color = state->m_spriteram3[i + 1] & 0x000f;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx],
				code,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);

	}
}

static void spikes91_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int i;
	UINT8 *lookup;
	lookup = machine.region("user1")->base();
	state->m_spritepalettebank = 1;

	for (i = state->m_spriteram3_size / 2 - 4; i >= 4; i -= 4)
	{
		int xpos, ypos, color, flipx, flipy, code, realcode;

		code = state->m_spriteram3[i + 0] & 0x1fff;

		if (!code)
			continue;

		xpos = (state->m_spriteram3[i + 2] & 0x01ff) - 16;
		ypos = 256 - (state->m_spriteram3[i + 1] & 0x00ff) - 26;
		flipy = 0;
		flipx = state->m_spriteram3[i + 3] & 0x8000;
		color = ((state->m_spriteram3[i + 3] & 0x00f0) >> 4);

		code |= state->m_spikes91_lookup * 0x2000;

		realcode = (lookup[code] << 8) + lookup[0x10000 + code];

		drawgfx_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos,15);

		/* wrap around y */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx],
				realcode,
				color,
				flipx,flipy,
				xpos,ypos + 512,15);
	}
}

static void aerfboot_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int attr_start, last;

	last = ((state->m_rasterram[0x404 / 2] << 5) - 0x8000) / 2;

	for (attr_start = state->m_spriteram3_size / 2 - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
		oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
		flipx = state->m_spriteram3[attr_start + 2] & 0x0800;
		flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
		color = state->m_spriteram3[attr_start + 2] & 0x000f;

		zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = state->m_spriteram3[attr_start + 2] & 0x0010;
		code = state->m_spriteram3[attr_start + 3] & 0x1fff;

		if (!(state->m_spriteram3[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine.priority_bitmap,pri ? 0 : 2,15);

	}

	last = ((state->m_rasterram[0x402 / 2] << 5) - 0x8000) / 2;

	for (attr_start = ((state->m_spriteram3_size / 2) / 2) - 4; attr_start >= last; attr_start -= 4)
	{
		int code;
		int ox, oy, sx, sy, zoomx, zoomy, flipx, flipy, color, pri;

		ox = state->m_spriteram3[attr_start + 1] & 0x01ff;
		oy = state->m_spriteram3[attr_start + 0] & 0x01ff;
		flipx = state->m_spriteram3[attr_start + 2] & 0x0800;
		flipy = state->m_spriteram3[attr_start + 2] & 0x8000;
		color = state->m_spriteram3[attr_start + 2] & 0x000f;

		zoomx = (state->m_spriteram3[attr_start + 1] & 0xf000) >> 12;
		zoomy = (state->m_spriteram3[attr_start + 0] & 0xf000) >> 12;
		pri = state->m_spriteram3[attr_start + 2] & 0x0010;
		code = state->m_spriteram3[attr_start + 3] & 0x1fff;

		if (!(state->m_spriteram3[attr_start + 2] & 0x0040))
			code |= 0x2000;

		zoomx = 32 + zoomx;
		zoomy = 32 + zoomy;

		sy = ((oy + 16 - 1) & 0x1ff) - 16;

		sx = ((ox + 16 + 3) & 0x1ff) - 16;

		pdrawgfxzoom_transpen(bitmap,cliprect,machine.gfx[state->m_sprite_gfx + (code >= 0x1000 ? 0 : 1)],
				code,
				color,
				flipx,flipy,
				sx,sy,
				zoomx << 11,zoomy << 11,
				machine.priority_bitmap,pri ? 0 : 2,15);

	}
}

static void wbbc97_draw_bitmap( running_machine &machine, bitmap_rgb32 &bitmap )
{
	aerofgt_state *state = machine.driver_data<aerofgt_state>();
	int x, y, count;

	count = 16; // weird, the bitmap doesn't start at 0?
	for (y = 0; y < 256; y++)
		for (x = 0; x < 512; x++)
		{
			int color = state->m_bitmapram[count] >> 1;

			/* data is GRB; convert to RGB */
			rgb_t pen = MAKE_RGB(pal5bit((color & 0x3e0) >> 5), pal5bit((color & 0x7c00) >> 10), pal5bit(color & 0x1f));
			bitmap.pix32(y, (10 + x - state->m_rasterram[(y & 0x7f)]) & 0x1ff) = pen;

			count++;
			count &= 0x1ffff;
		}
}


SCREEN_UPDATE_IND16( pspikes )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(256);
	scrolly = state->m_bg1scrolly;
	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0xff, state->m_rasterram[i]);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( pspikesb )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(256);
	scrolly = state->m_bg1scrolly;
	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0xff, state->m_rasterram[i] + 22);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	pspikesb_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( spikes91 )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;
	int y, x;
	int count;
	const gfx_element *gfx = screen.machine().gfx[0];

	state->m_bg1_tilemap->set_scroll_rows(256);
	scrolly = state->m_bg1scrolly;

	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0xff, state->m_rasterram[i + 0x01f0 / 2] + 0x96 + 0x16);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	spikes91_draw_sprites(screen.machine(), bitmap, cliprect);

	/* we could use a tilemap, but it's easier to just do it here */
	count = 0;
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			UINT16 tileno = state->m_tx_tilemap_ram[count] & 0x1fff;
			UINT16 colour = state->m_tx_tilemap_ram[count] & 0xe000;
			drawgfx_transpen(bitmap,cliprect,gfx,
					tileno,
					colour>>13,
					0,0,
					(x*8)+24,(y*8)+8,15);

			count++;

		}

	}

	return 0;
}

SCREEN_UPDATE_IND16( karatblz )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	state->m_bg1_tilemap->set_scrollx(0, state->m_bg1scrollx - 8);
	state->m_bg1_tilemap->set_scrolly(0, state->m_bg1scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_bg2scrollx - 4);
	state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 1, -1);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0);
	return 0;
}

SCREEN_UPDATE_IND16( spinlbrk )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(512);
	scrolly = 0;
	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[i] - 8);
//  state->m_bg1_tilemap->set_scrolly(0, state->m_bg1scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_bg2scrollx - 4);
//  state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	spinlbrk_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0);
	spinlbrk_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1);
	spinlbrk_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0);
	spinlbrk_draw_sprites(screen.machine(), bitmap, cliprect, 1, -1);
	return 0;
}

SCREEN_UPDATE_IND16( turbofrc )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(512);
	scrolly = state->m_bg1scrolly + 2;
	for (i = 0; i < 256; i++)
//      state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[i] - 11);
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[7] - 11);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_bg2scrollx - 7);
	state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly + 2);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 1, -1); //ship
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0); //intro
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1); //enemy
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0); //enemy
	return 0;
}

SCREEN_UPDATE_IND16( aerofgt )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	state->m_bg1_tilemap->set_scrollx(0, state->m_rasterram[0x0000] - 18);
	state->m_bg1_tilemap->set_scrolly(0, state->m_bg1scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_rasterram[0x0200] - 20);
	state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);

	aerofgt_draw_sprites(screen.machine(), bitmap, cliprect, 0);
	aerofgt_draw_sprites(screen.machine(), bitmap, cliprect, 1);

	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 0);

	aerofgt_draw_sprites(screen.machine(), bitmap, cliprect, 2);
	aerofgt_draw_sprites(screen.machine(), bitmap, cliprect, 3);
	return 0;
}


SCREEN_UPDATE_IND16( aerfboot )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(512);
	scrolly = state->m_bg1scrolly + 2;
	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[7] + 174);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_bg2scrollx + 172);
	state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly + 2);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboot_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( aerfboo2 )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(512);
	scrolly = state->m_bg1scrolly + 2;
	for (i = 0; i < 256; i++)
//      state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[i] - 11);
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0x1ff, state->m_rasterram[7] - 11);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);
	state->m_bg2_tilemap->set_scrollx(0, state->m_bg2scrollx - 7);
	state->m_bg2_tilemap->set_scrolly(0, state->m_bg2scrolly + 2);

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_bg2_tilemap->draw(bitmap, cliprect, 0, 1);

	/* we use the priority buffer so sprites are drawn front to back */
	aerfboo2_draw_sprites(screen.machine(), bitmap, cliprect, 1, -1); //ship
	aerfboo2_draw_sprites(screen.machine(), bitmap, cliprect, 1, 0); //intro
	aerfboo2_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1); //enemy
	aerfboo2_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0); //enemy
	return 0;
}

SCREEN_UPDATE_RGB32( wbbc97 )
{
	aerofgt_state *state = screen.machine().driver_data<aerofgt_state>();
	int i, scrolly;

	state->m_bg1_tilemap->set_scroll_rows(256);
	scrolly = state->m_bg1scrolly;
	for (i = 0; i < 256; i++)
		state->m_bg1_tilemap->set_scrollx((i + scrolly) & 0xff, state->m_rasterram[i]);
	state->m_bg1_tilemap->set_scrolly(0, scrolly);

	screen.machine().priority_bitmap.fill(0, cliprect);

	if (state->m_wbbc97_bitmap_enable)
	{
		wbbc97_draw_bitmap(screen.machine(), bitmap);
		state->m_bg1_tilemap->draw(bitmap, cliprect, 0, 0);
	}
	else
	{
		state->m_bg1_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	}

	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, -1);
	turbofrc_draw_sprites(screen.machine(), bitmap, cliprect, 0, 0);
	return 0;
}
