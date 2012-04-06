/*************************************************************************

   Run and Gun
   (c) 1993 Konami

   Video hardware emulation.

   Driver by R. Belmont

*************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/rungun.h"

/* TTL text plane stuff */
static TILE_GET_INFO( ttl_get_tile_info )
{
	rungun_state *state = machine.driver_data<rungun_state>();
	UINT8 *lvram = (UINT8 *)state->m_ttl_vram;
	int attr, code;

	attr = (lvram[BYTE_XOR_LE(tile_index<<2)] & 0xf0) >> 4;
	code = ((lvram[BYTE_XOR_LE(tile_index<<2)] & 0x0f) << 8) | (lvram[BYTE_XOR_LE((tile_index<<2)+2)]);

	SET_TILE_INFO(state->m_ttl_gfx_index, code, attr, 0);
}

void rng_sprite_callback( running_machine &machine, int *code, int *color, int *priority_mask )
{
	rungun_state *state = machine.driver_data<rungun_state>();
	*color = state->m_sprite_colorbase | (*color & 0x001f);
}

READ16_MEMBER(rungun_state::rng_ttl_ram_r)
{
	return m_ttl_vram[offset];
}

WRITE16_MEMBER(rungun_state::rng_ttl_ram_w)
{
	COMBINE_DATA(&m_ttl_vram[offset]);
}

/* 53936 (PSAC2) rotation/zoom plane */
WRITE16_MEMBER(rungun_state::rng_936_videoram_w)
{
	COMBINE_DATA(&m_936_videoram[offset]);
	m_936_tilemap->mark_tile_dirty(offset / 2);
}

static TILE_GET_INFO( get_rng_936_tile_info )
{
	rungun_state *state = machine.driver_data<rungun_state>();
	int tileno, colour, flipx;

	tileno = state->m_936_videoram[tile_index * 2 + 1] & 0x3fff;
	flipx = (state->m_936_videoram[tile_index * 2 + 1] & 0xc000) >> 14;
	colour = 0x10 + (state->m_936_videoram[tile_index * 2] & 0x000f);

	SET_TILE_INFO(0, tileno, colour, TILE_FLIPYX(flipx));
}


VIDEO_START( rng )
{
	static const gfx_layout charlayout =
	{
		8, 8,	// 8x8
		4096,	// # of tiles
		4,		// 4bpp
		{ 0, 1, 2, 3 },	// plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	// X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	// Y offsets
		8*8*4
	};

	rungun_state *state = machine.driver_data<rungun_state>();
	int gfx_index;

	state->m_936_tilemap = tilemap_create(machine, get_rng_936_tile_info, tilemap_scan_rows, 16, 16, 128, 128);
	state->m_936_tilemap->set_transparent_pen(0);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine.gfx[gfx_index] == 0)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	machine.gfx[gfx_index] = gfx_element_alloc(machine, &charlayout, machine.region("gfx3")->base(), machine.total_colors() / 16, 0);
	state->m_ttl_gfx_index = gfx_index;

	// create the tilemap
	state->m_ttl_tilemap = tilemap_create(machine, ttl_get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	state->m_ttl_tilemap->set_transparent_pen(0);

	state->m_sprite_colorbase = 0x20;
}

SCREEN_UPDATE_IND16(rng)
{
	rungun_state *state = screen.machine().driver_data<rungun_state>();

	bitmap.fill(get_black_pen(screen.machine()), cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	k053936_zoom_draw(state->m_k053936, bitmap, cliprect, state->m_936_tilemap, 0, 0, 1);

	k053247_sprites_draw(state->m_k055673, bitmap, cliprect);

	state->m_ttl_tilemap->mark_all_dirty();
	state->m_ttl_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
