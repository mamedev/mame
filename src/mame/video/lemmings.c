/***************************************************************************

    Lemmings video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    There are two sets of sprites, the combination of custom chips 52 & 71.
    There is a background pixel layer implemented with discrete logic
    rather than a custom chip and a foreground VRAM tilemap layer that the
    game mostly uses as a pixel layer (the vram format is arranged as
    sequential pixels, rather than sequential characters).

***************************************************************************/

#include "emu.h"
#include "includes/lemmings.h"
#include "video/decospr.h"

/******************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	lemmings_state *state = machine.driver_data<lemmings_state>();
	UINT16 tile = state->m_vram_data[tile_index];

	SET_TILE_INFO(
			2,
			tile&0x7ff,
			(tile>>12)&0xf,
			0);
}

VIDEO_START( lemmings )
{
	lemmings_state *state = machine.driver_data<lemmings_state>();
	state->m_vram_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols, 8, 8, 64, 32);

	state->m_vram_tilemap->set_transparent_pen(0);
	state->m_bitmap0.fill(0x100);

	gfx_element_set_source(machine.gfx[2], state->m_vram_buffer);

	machine.device<decospr_device>("spritegen")->alloc_sprite_bitmap();
	machine.device<decospr_device>("spritegen2")->alloc_sprite_bitmap();

	state->save_item(NAME(state->m_bitmap0));
	state->save_item(NAME(state->m_vram_buffer));
	state->save_item(NAME(state->m_sprite_triple_buffer_0));
	state->save_item(NAME(state->m_sprite_triple_buffer_1));
}

SCREEN_VBLANK( lemmings )
{
	// rising edge
	if (vblank_on)
	{
		lemmings_state *state = screen.machine().driver_data<lemmings_state>();
		memcpy(state->m_sprite_triple_buffer_0, state->m_spriteram->buffer(), 0x800);
		memcpy(state->m_sprite_triple_buffer_1, state->m_spriteram2->buffer(), 0x800);
	}
}

/******************************************************************************/

// RAM based
WRITE16_MEMBER(lemmings_state::lemmings_pixel_0_w)
{

	int sx, sy, src, old;

	old = m_pixel_0_data[offset];
	COMBINE_DATA(&m_pixel_0_data[offset]);
	src = m_pixel_0_data[offset];
	if (old == src)
		return;

	sy = (offset << 1) / 0x800;
	sx = (offset << 1) & 0x7ff;

	if (sx > 2047 || sy > 255)
		return;

	m_bitmap0.pix16(sy, sx + 0) = ((src >> 8) & 0xf) | 0x100;
	m_bitmap0.pix16(sy, sx + 1) = ((src >> 0) & 0xf) | 0x100;
}

// RAM based tiles for the FG tilemap
WRITE16_MEMBER(lemmings_state::lemmings_pixel_1_w)
{
	int sx, sy, src, tile;

	COMBINE_DATA(&m_pixel_1_data[offset]);
	src = m_pixel_1_data[offset];

	sy = ((offset << 1) / 0x200);
	sx = ((offset << 1) & 0x1ff);

	/* Copy pixel to buffer for easier decoding later */
	tile = ((sx / 8) * 32) + (sy / 8);
	gfx_element_mark_dirty(machine().gfx[2], tile);
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 8) & 0xf;

	sx += 1; /* Update both pixels in the word */
	m_vram_buffer[(tile * 64) + ((sx & 7)) + ((sy & 7) * 8)] = (src >> 0) & 0xf;
}

WRITE16_MEMBER(lemmings_state::lemmings_vram_w)
{
	COMBINE_DATA(&m_vram_data[offset]);
	m_vram_tilemap->mark_tile_dirty(offset);
}


void lemmings_copy_bitmap(running_machine &machine, bitmap_rgb32& bitmap, bitmap_ind16& srcbitmap, int* xscroll, int* yscroll, const rectangle& cliprect)
{
	int y,x;
	const pen_t *paldata = machine.pens;

	for (y=cliprect.min_y; y<cliprect.max_y;y++)
	{
		UINT32* dst = &bitmap.pix32(y,0);

		for (x=cliprect.min_x; x<cliprect.max_x;x++)
		{
			UINT16 src = srcbitmap.pix16((y-*yscroll)&0xff,(x-*xscroll)&0x7ff);

			if (src!=0x100)
				dst[x] = paldata[src];
		}
	}
}

SCREEN_UPDATE_RGB32( lemmings )
{
	lemmings_state *state = screen.machine().driver_data<lemmings_state>();
	int x1 = -state->m_control_data[0];
	int x0 = -state->m_control_data[2];
	int y = 0;
	rectangle rect;
	rect.max_y = cliprect.max_y;
	rect.min_y = cliprect.min_y;

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_sprite_triple_buffer_1, 0x400, true);
	screen.machine().device<decospr_device>("spritegen2")->draw_sprites(bitmap, cliprect, state->m_sprite_triple_buffer_0, 0x400, true);

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x300, 0xff);

	/* Pixel layer can be windowed in hardware (two player mode) */
	if ((state->m_control_data[6] & 2) == 0)
	{
		lemmings_copy_bitmap(screen.machine(), bitmap, state->m_bitmap0, &x1, &y, cliprect);
	}
	else
	{
		rect.max_x = 159;
		rect.min_x = 0;
		lemmings_copy_bitmap(screen.machine(), bitmap, state->m_bitmap0, &x0, &y, rect);

		rect.max_x = 319;
		rect.min_x = 160;
		lemmings_copy_bitmap(screen.machine(), bitmap, state->m_bitmap0, &x1, &y, rect);
	}

	screen.machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0xff);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x300, 0xff);
	state->m_vram_tilemap->draw(bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen2")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0xff);
	return 0;
}
