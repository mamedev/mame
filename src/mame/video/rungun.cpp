// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************

   Run and Gun
   (c) 1993 Konami

   Video hardware emulation.

   Driver by R. Belmont

*************************************************************************/

#include "emu.h"

#include "includes/rungun.h"

/* TTL text plane stuff */
TILE_GET_INFO_MEMBER(rungun_state::ttl_get_tile_info)
{
	UINT32 base_addr = (FPTR)tilemap.user_data();
	UINT8 *lvram = (UINT8 *)m_ttl_vram + base_addr;
	int attr, code;
	
	attr = (lvram[BYTE_XOR_LE(tile_index<<2)] & 0xf0) >> 4;
	code = ((lvram[BYTE_XOR_LE(tile_index<<2)] & 0x0f) << 8) | (lvram[BYTE_XOR_LE((tile_index<<2)+2)]);

	SET_TILE_INFO_MEMBER(m_ttl_gfx_index, code, attr, 0);
}

K055673_CB_MEMBER(rungun_state::sprite_callback)
{
	*color = m_sprite_colorbase | (*color & 0x001f);
}

READ16_MEMBER(rungun_state::rng_ttl_ram_r)
{
	return m_ttl_vram[offset+(m_video_mux_bank*0x1000)];
}

WRITE16_MEMBER(rungun_state::rng_ttl_ram_w)
{
	COMBINE_DATA(&m_ttl_vram[offset+(m_video_mux_bank*0x1000)]);
	m_ttl_tilemap[m_video_mux_bank]->mark_tile_dirty(offset / 2);
}

/* 53936 (PSAC2) rotation/zoom plane */
READ16_MEMBER(rungun_state::rng_psac2_videoram_r)
{
	return m_psac2_vram[offset+(m_video_mux_bank*0x80000)];
}

WRITE16_MEMBER(rungun_state::rng_psac2_videoram_w)
{
	COMBINE_DATA(&m_psac2_vram[offset+(m_video_mux_bank*0x80000)]);
	m_936_tilemap[m_video_mux_bank]->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(rungun_state::get_rng_936_tile_info)
{
	UINT32 base_addr = (FPTR)tilemap.user_data();
	int tileno, colour, flipx;
	
	tileno = m_psac2_vram[tile_index * 2 + 1 + base_addr] & 0x3fff;
	flipx = (m_psac2_vram[tile_index * 2 + 1 + base_addr] & 0xc000) >> 14;
	colour = 0x10 + (m_psac2_vram[tile_index * 2 + base_addr] & 0x000f);

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

	m_ttl_vram = auto_alloc_array(machine(), UINT16, 0x1000*2);
	m_psac2_vram = auto_alloc_array(machine(), UINT16, 0x80000*2);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (m_gfxdecode->gfx(gfx_index) == nullptr)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	m_gfxdecode->set_gfx(gfx_index, global_alloc(gfx_element(m_palette, charlayout, memregion("gfx3")->base(), 0, m_palette->entries() / 16, 0)));
	m_ttl_gfx_index = gfx_index;

	// create the tilemaps
	for(UINT32 screen_num = 0;screen_num < 2;screen_num++)
	{
		m_ttl_tilemap[screen_num] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rungun_state::ttl_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
		m_ttl_tilemap[screen_num]->set_user_data((void *)(FPTR)(screen_num * 0x2000));
		m_ttl_tilemap[screen_num]->set_transparent_pen(0);

		m_936_tilemap[screen_num] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rungun_state::get_rng_936_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 128, 128);
		m_936_tilemap[screen_num]->set_user_data((void *)(FPTR)(screen_num * 0x80000));
		m_936_tilemap[screen_num]->set_transparent_pen(0);
		
	}
	m_sprite_colorbase = 0x20;

	m_screen->register_screen_bitmap(m_rng_dual_demultiplex_left_temp);
	m_screen->register_screen_bitmap(m_rng_dual_demultiplex_right_temp);
}

UINT32 rungun_state::screen_update_rng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	screen.priority().fill(0, cliprect);
	m_current_display_bank = machine().first_screen()->frame_number() & 1;
	if(m_single_screen_mode == true)
		m_current_display_bank = 0;
		
	if(m_video_priority_mode == false)
	{
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_936_tilemap[m_current_display_bank], 0, 0, 1);
		m_k055673->k053247_sprites_draw(bitmap, cliprect);
	}
	else
	{
		m_k055673->k053247_sprites_draw(bitmap, cliprect);
		m_k053936->zoom_draw(screen, bitmap, cliprect, m_936_tilemap[m_current_display_bank], 0, 0, 1);
	}
	
	m_ttl_tilemap[m_current_display_bank]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


// the 60hz signal gets split between 2 screens
UINT32 rungun_state::screen_update_rng_dual_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int m_current_display_bank = machine().first_screen()->frame_number() & 1;

	if (!m_current_display_bank)
		screen_update_rng(screen, m_rng_dual_demultiplex_left_temp, cliprect);
	else
		screen_update_rng(screen, m_rng_dual_demultiplex_right_temp, cliprect);

	copybitmap( bitmap, m_rng_dual_demultiplex_left_temp, 0, 0, 0, 0, cliprect);
	return 0;
}

// this depends upon the fisrt screen being updated, and the bitmap being copied to the temp bitmap
UINT32 rungun_state::screen_update_rng_dual_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap( bitmap, m_rng_dual_demultiplex_right_temp, 0, 0, 0, 0, cliprect);
	return 0;
}

void rungun_state::sprite_dma_trigger(void)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT32 src_address;

	if(m_single_screen_mode == true)
		src_address = 1*0x2000;
	else
		src_address = m_current_display_bank*0x2000;

	// TODO: size could be programmable somehow.
	for(int i=0;i<0x1000;i+=2)
		m_k055673->k053247_word_w(space,i/2,m_banked_ram[(i + src_address) /2],0xffff);	
}
