/*******************************************************************************

    Karnov - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/karnov.h"
#include "video/deckarn.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Karnov has two 1024x8 palette PROM.
  I don't know the exact values of the resistors between the RAM and the
  RGB output. I assumed these values (the same as Commando)

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

void karnov_state::palette_init()
{
	const UINT8 *color_prom = machine().root_device().memregion("proms")->base();
	int i;

	for (i = 0; i < machine().total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine().total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine().total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine().total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine().total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine(), i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

void karnov_flipscreen_w( running_machine &machine, int data )
{
	karnov_state *state = machine.driver_data<karnov_state>();
	state->m_flipscreen = data;
	machine.tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

static void draw_background( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	karnov_state *state = machine.driver_data<karnov_state>();
	int my, mx, offs, color, tile, fx, fy;
	int scrollx = state->m_scroll[0];
	int scrolly = state->m_scroll[1];

	if (state->m_flipscreen)
		fx = fy = 1;
	else
		fx = fy = 0;

	mx = -1;
	my = 0;

	for (offs = 0; offs < 0x400; offs ++)
	{
		mx++;
		if (mx == 32)
		{
			mx=0;
			my++;
		}

		tile = state->m_pf_data[offs];
		color = tile >> 12;
		tile = tile & 0x7ff;
		if (state->m_flipscreen)
			drawgfx_opaque(*state->m_bitmap_f, state->m_bitmap_f->cliprect(), machine.gfx[1],tile,
				color, fx, fy, 496-16*mx,496-16*my);
		else
			drawgfx_opaque(*state->m_bitmap_f, state->m_bitmap_f->cliprect(), machine.gfx[1],tile,
				color, fx, fy, 16*mx,16*my);
	}

	if (!state->m_flipscreen)
	{
		scrolly = -scrolly;
		scrollx = -scrollx;
	}
	else
	{
		scrolly = scrolly + 256;
		scrollx = scrollx + 256;
	}

	copyscrollbitmap(bitmap, *state->m_bitmap_f, 1, &scrollx, 1, &scrolly, cliprect);
}

/******************************************************************************/

UINT32 karnov_state::screen_update_karnov(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(screen.machine(), bitmap, cliprect);
	screen.machine().device<deco_karnovsprites_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, m_spriteram->buffer(), 0x800, 0);
	m_fix_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(karnov_state::get_fix_tile_info)
{
	int tile = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(
			0,
			tile&0xfff,
			tile>>14,
			0);
}

WRITE16_MEMBER(karnov_state::karnov_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(karnov_state::karnov_playfield_swap_w)
{
	offset = ((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5);
	COMBINE_DATA(&m_pf_data[offset]);
}

/******************************************************************************/

VIDEO_START_MEMBER(karnov_state,karnov)
{

	/* Allocate bitmap & tilemap */
	m_bitmap_f = auto_bitmap_ind16_alloc(machine(), 512, 512);
	m_fix_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(karnov_state,wndrplnt)
{

	/* Allocate bitmap & tilemap */
	m_bitmap_f = auto_bitmap_ind16_alloc(machine(), 512, 512);
	m_fix_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(karnov_state::get_fix_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	save_item(NAME(*m_bitmap_f));

	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
