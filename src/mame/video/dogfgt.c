#include "emu.h"
#include "includes/dogfgt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Dog-Fight has both palette RAM and PROMs. The PROMs are used for tiles &
  pixmap, RAM for sprites.

***************************************************************************/

PALETTE_INIT( dogfgt )
{
	int i;

	/* first 16 colors are RAM */
	for (i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 16, MAKE_RGB(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	dogfgt_state *state = machine.driver_data<dogfgt_state>();
	SET_TILE_INFO(
			0,
			state->m_bgvideoram[tile_index],
			state->m_bgvideoram[tile_index + 0x400] & 0x03,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( dogfgt )
{
	dogfgt_state *state = machine.driver_data<dogfgt_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->m_bitmapram = auto_alloc_array(machine, UINT8, BITMAPRAM_SIZE);
	state->save_pointer(NAME(state->m_bitmapram), BITMAPRAM_SIZE);

	machine.primary_screen->register_screen_bitmap(state->m_pixbitmap);
	state->save_item(NAME(state->m_pixbitmap));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(dogfgt_state::dogfgt_plane_select_w)
{

	m_bm_plane = data;
}

READ8_MEMBER(dogfgt_state::dogfgt_bitmapram_r)
{

	if (m_bm_plane > 2)
	{
		popmessage("bitmapram_r offs %04x plane %d\n", offset, m_bm_plane);
		return 0;
	}

	return m_bitmapram[offset + BITMAPRAM_SIZE / 3 * m_bm_plane];
}

WRITE8_MEMBER(dogfgt_state::internal_bitmapram_w)
{
	int x, y, subx;

	m_bitmapram[offset] = data;

	offset &= (BITMAPRAM_SIZE / 3 - 1);
	x = 8 * (offset / 256);
	y = offset % 256;

	for (subx = 0; subx < 8; subx++)
	{
		int i, color = 0;

		for (i = 0; i < 3; i++)
			color |= ((m_bitmapram[offset + BITMAPRAM_SIZE / 3 * i] >> subx) & 1) << i;

		if (flip_screen_get(machine()))
			m_pixbitmap.pix16(y ^ 0xff, (x + subx) ^ 0xff) = PIXMAP_COLOR_BASE + 8 * m_pixcolor + color;
		else
			m_pixbitmap.pix16(y, x + subx) = PIXMAP_COLOR_BASE + 8 * m_pixcolor + color;
	}
}

WRITE8_MEMBER(dogfgt_state::dogfgt_bitmapram_w)
{

	if (m_bm_plane > 2)
	{
		popmessage("bitmapram_w offs %04x plane %d\n", offset, m_bm_plane);
		return;
	}

	internal_bitmapram_w(space, offset + BITMAPRAM_SIZE / 3 * m_bm_plane, data);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_bgvideoram_w)
{

	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_scroll_w)
{

	m_scroll[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll[0] + 256 * m_scroll[1] + 256);
	m_bg_tilemap->set_scrolly(0, m_scroll[2] + 256 * m_scroll[3]);
}

WRITE8_MEMBER(dogfgt_state::dogfgt_1800_w)
{

	/* bits 0 and 1 are probably text color (not verified because PROM is missing) */
	m_pixcolor = ((data & 0x01) << 1) | ((data & 0x02) >> 1);

	/* bits 4 and 5 are coin counters */
	coin_counter_w(machine(), 0, data & 0x10);
	coin_counter_w(machine(), 1, data & 0x20);

	/* bit 7 is screen flip */
	flip_screen_set(machine(), data & 0x80);

	/* other bits unused? */
	logerror("PC %04x: 1800 = %02x\n", cpu_get_pc(&space.device()), data);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect )
{
	dogfgt_state *state = machine.driver_data<dogfgt_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		if (state->m_spriteram[offs] & 0x01)
		{
			int sx, sy, flipx, flipy;

			sx = state->m_spriteram[offs + 3];
			sy = (240 - state->m_spriteram[offs + 2]) & 0xff;
			flipx = state->m_spriteram[offs] & 0x04;
			flipy = state->m_spriteram[offs] & 0x02;
			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					state->m_spriteram[offs + 1] + ((state->m_spriteram[offs] & 0x30) << 4),
					(state->m_spriteram[offs] & 0x08) >> 3,
					flipx,flipy,
					sx,sy,0);
		}
	}
}


SCREEN_UPDATE_IND16( dogfgt )
{
	dogfgt_state *state = screen.machine().driver_data<dogfgt_state>();
	int offs;

	if (state->m_lastflip != flip_screen_get(screen.machine()) || state->m_lastpixcolor != state->m_pixcolor)
	{
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);

		state->m_lastflip = flip_screen_get(screen.machine());
		state->m_lastpixcolor = state->m_pixcolor;

		for (offs = 0; offs < BITMAPRAM_SIZE; offs++)
			state->internal_bitmapram_w(*space, offs, state->m_bitmapram[offs]);
	}


	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);

	draw_sprites(screen.machine(), bitmap, cliprect);

	copybitmap_trans(bitmap, state->m_pixbitmap, 0, 0, 0, 0, cliprect, PIXMAP_COLOR_BASE + 8 * state->m_pixcolor);
	return 0;
}
