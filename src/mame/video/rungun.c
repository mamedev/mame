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
TILE_GET_INFO_MEMBER(rungun_state::ttl_get_tile_info)
{
	UINT8 *lvram = (UINT8 *)m_ttl_vram;
	int attr, code;

	attr = (lvram[BYTE_XOR_LE(tile_index<<2)] & 0xf0) >> 4;
	code = ((lvram[BYTE_XOR_LE(tile_index<<2)] & 0x0f) << 8) | (lvram[BYTE_XOR_LE((tile_index<<2)+2)]);

	SET_TILE_INFO_MEMBER(m_ttl_gfx_index, code, attr, 0);
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

TILE_GET_INFO_MEMBER(rungun_state::get_rng_936_tile_info)
{
	int tileno, colour, flipx;

	tileno = m_936_videoram[tile_index * 2 + 1] & 0x3fff;
	flipx = (m_936_videoram[tile_index * 2 + 1] & 0xc000) >> 14;
	colour = 0x10 + (m_936_videoram[tile_index * 2] & 0x000f);

	SET_TILE_INFO_MEMBER(0, tileno, colour, TILE_FLIPYX(flipx));
}


void rungun_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	int gfx_index;

	m_936_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(rungun_state::get_rng_936_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
	m_936_tilemap->set_transparent_pen(0);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine().gfx[gfx_index] == 0)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	machine().gfx[gfx_index] = auto_alloc(machine(), gfx_element(machine(), charlayout, memregion("gfx3")->base(), machine().total_colors() / 16, 0));
	m_ttl_gfx_index = gfx_index;

	// create the tilemap
	m_ttl_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(rungun_state::ttl_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_ttl_tilemap->set_transparent_pen(0);

	m_sprite_colorbase = 0x20;
}

UINT32 rungun_state::screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	bitmap.fill(get_black_pen(machine()), cliprect);
	machine().priority_bitmap.fill(0, cliprect);

	k053936_zoom_draw(m_k053936, bitmap, cliprect, m_936_tilemap, 0, 0, 1);

	k053247_sprites_draw(m_k055673, bitmap, cliprect);

	m_ttl_tilemap->mark_all_dirty();
	m_ttl_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
