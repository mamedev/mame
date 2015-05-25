// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Vas Crabb
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/goldstar.h"




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/


WRITE8_MEMBER(goldstar_state::goldstar_fg_vidram_w)
{
	m_fg_vidram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(goldstar_state::goldstar_fg_atrram_w)
{
	m_fg_atrram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(goldstar_state::get_goldstar_fg_tile_info)
{
	int const code = m_fg_vidram[tile_index];
	int const attr = m_fg_atrram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code | (attr & 0xf0) << 4,
			attr & 0x0f,
			0);
}


// colour / high tile bits are swapped around
TILE_GET_INFO_MEMBER(goldstar_state::get_cherrym_fg_tile_info)
{
	int const code = m_fg_vidram[tile_index];
	int const attr = m_fg_atrram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code | (attr & 0x0f) << 8,
			(attr & 0xf0) >> 4,
			0);
}



WRITE8_MEMBER(goldstar_state::goldstar_reel1_ram_w)
{
	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goldstar_state::get_goldstar_reel1_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
			m_reel1_ram[tile_index],
			m_bgcolor,
			0);
}


WRITE8_MEMBER(goldstar_state::goldstar_reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goldstar_state::get_goldstar_reel2_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
			m_reel2_ram[tile_index],
			m_bgcolor,
			0);
}

WRITE8_MEMBER(goldstar_state::goldstar_reel3_ram_w)
{
	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goldstar_state::get_goldstar_reel3_tile_info)
{
	SET_TILE_INFO_MEMBER(1,
			m_reel3_ram[tile_index],
			m_bgcolor,
			0);
}

VIDEO_START_MEMBER(goldstar_state, goldstar)
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	m_cm_enable_reg = 0x0b;
}

VIDEO_START_MEMBER(goldstar_state, cherrym)
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_goldstar_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

	m_cmaster_girl_num = 0;
	m_cmaster_girl_pal = 0;

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goldstar_state::get_cherrym_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_cm_enable_reg = 0x0b;
}


WRITE8_MEMBER(goldstar_state::goldstar_fa00_w)
{
	/* bit 1 toggles continuously - might be irq enable or watchdog reset */

	/* bit 2 selects background gfx color (I think) */
	m_bgcolor = (data & 0x04) >> 2;
	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();
}



UINT32 goldstar_state::screen_update_goldstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		for (int i = 0; i < 64; i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
			m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
			m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
		}

		// are these hardcoded, or registers?
		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+7)*8-1);
		const rectangle visible2(0*8, (14+48)*8-1, 12*8, (12+7)*8-1);
		const rectangle visible3(0*8, (14+48)*8-1, 20*8, (20+7)*8-1);

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	if (m_cm_enable_reg & 0x04)
	{
		if (memregion("user1")->base())
		{
			gfx_element *gfx = m_gfxdecode->gfx(2);
			int const girlyscroll = (INT8)((m_cm_girl_scroll & 0xf0));
			int const girlxscroll = (INT8)((m_cm_girl_scroll & 0x0f) << 4);

			gfx->zoom_transpen(bitmap,cliprect,m_cmaster_girl_num,m_cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (m_cm_enable_reg & 0x02)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


UINT32 goldstar_state::screen_update_cmast91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		for (int i = 0; i < 64; i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
			m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
			m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
		}

		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+7)*8-1);   /* same start for reel1 */
		const rectangle visible2(0*8, (14+48)*8-1, 11*8, (12+7)*8-1);   /* 4 pixels less for reel2 */
		const rectangle visible3(0*8, (14+48)*8-1, 19*8, (19+7)*8-1);   /* 8 pixels less for reel3 */

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	if (m_cm_enable_reg & 0x02)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



WRITE8_MEMBER(cmaster_state::outport0_w)
{
	m_cm_enable_reg = data;
	/*
	    ---- ---x  (global enable or irq enable?)
	    ---- --x-  (fg enable)
	    ---- -x--  (girl enable?)
	    ---- x---  (reels enable)

	    xxxx ----  unused?

	*/
	//popmessage("%02x",data);
}

WRITE8_MEMBER(cmaster_state::girl_scroll_w)
{
	m_cm_girl_scroll = data;
	/*
	    xxxx ----  yscroll
	    ---- xxxx  xscroll

	    this isn't very fine scrolling, but i see no other registers.
	    1000 1000 is the center of the screen.
	*/
}

WRITE8_MEMBER(cmaster_state::background_col_w)
{
	//printf("cm_background_col_w %02x\n",data);

	/* cherry master writes

	so it's probably

	0ggg cc00

	where g is which girl to display and c is the colour palette

	(note, this doesn't apply to the amcoe games which have no girls, I'm unsure how the priority/positioning works)


	*/
	m_cmaster_girl_num = (data >> 4) & 0x7;
	m_cmaster_girl_pal = (data >> 2) & 0x3;

	//bgcolor = (data & 0x03) >> 0;

	// apparently some boards have this colour scheme?
	// i'm not convinced it isn't just a different prom on them
	#if 0
	m_bgcolor = 0;
	m_bgcolor |= (data & 0x01) << 1;
	m_bgcolor |= (data & 0x02) >> 1;
	#else
	m_bgcolor = (data & 0x03) >> 0;
	#endif

	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();
}


UINT32 cmaster_state::screen_update_amcoe1a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		for (int i= 0;i < 64;i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
			m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
			m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
		}

		rectangle const visible1(0*8, (14+48)*8-1,  4*8,  (4+6)*8-1);
		rectangle const visible2(0*8, (14+48)*8-1, 10*8, (10+6)*8-1);
		rectangle const visible3(0*8, (14+48)*8-1, 16*8, (16+6)*8-1);

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	if (m_cm_enable_reg & 0x04)
	{
		// no girls
	}

	if (m_cm_enable_reg & 0x02)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



TILE_GET_INFO_MEMBER(wingco_state::get_magical_fg_tile_info)
{
	int const code = m_fg_vidram[tile_index];
	int const attr = m_fg_atrram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			(code | (attr & 0xf0)<<4) + (m_tile_bank * 0x1000),
			attr & 0x0f,
			0);
}


VIDEO_START_MEMBER(wingco_state, bingowng)
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_goldstar_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_goldstar_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	m_cm_enable_reg = 0x0b;
}

VIDEO_START_MEMBER(wingco_state, magical)
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_goldstar_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_goldstar_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_goldstar_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(32);
	m_reel2_tilemap->set_scroll_cols(32);
	m_reel3_tilemap->set_scroll_cols(32);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wingco_state::get_magical_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	// is there an enable reg for this game?
	m_cm_enable_reg = 0x0b;
}


UINT32 wingco_state::screen_update_bingowng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		for (int i = 0; i < 64; i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
		}

		rectangle const visible1(0*8, (14+48)*8-1,  3*8,  (4+7)*8-1);
		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
	}

	if (m_cm_enable_reg & 0x04)
	{
		if (memregion("user1")->base())
		{
			gfx_element *gfx = m_gfxdecode->gfx(2);
			int const girlyscroll = (INT8)((m_cm_girl_scroll & 0xf0));
			int const girlxscroll = (INT8)((m_cm_girl_scroll & 0x0f)<<4);

			gfx->zoom_transpen(bitmap,cliprect,m_cmaster_girl_num,m_cmaster_girl_pal,0,0,-(girlxscroll*2),-(girlyscroll), 0x20000, 0x10000,0);
		}
	}

	if (m_cm_enable_reg & 0x02)
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}

	return 0;
}

UINT32 wingco_state::screen_update_magical(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		// guess, could be wrong, but different screens clearly need different reel layouts
		if (m_vidreg & 2)
		{
			for (int i = 0; i < 32; i++)
			{
				m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i*2]);
				m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i*2]);
			//  m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i*2]);
			}

			rectangle const visible1alt(0*8, (16+48)*8-1,  4*8,  16*8-1);
			rectangle const visible2alt(0*8, (16+48)*8-1, 16*8,  28*8-1);

			m_reel1_tilemap->draw(screen, bitmap, visible1alt, 0, 0);
			m_reel2_tilemap->draw(screen, bitmap, visible2alt, 0, 0);
			//m_reel3_tilemap->draw(screen, bitmap, &magical_visible3, 0, 0);
		}
		else
		{
			for (int i = 0; i < 32; i++)
			{
				m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i*2]);
				m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i*2]);
				m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i*2]);
			}

			rectangle const visible1(0*8, (14+48)*8-1,  4*8,  (4+8)*8-1);
			rectangle const visible2(0*8, (14+48)*8-1, 12*8, (12+8)*8-1);
			rectangle const visible3(0*8, (14+48)*8-1, 20*8, (20+8)*8-1);

			m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
			m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
			m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
		}
	}

	if (m_cm_enable_reg & 0x02)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



WRITE8_MEMBER(sanghopm_state::fg_vidram_w)
{
	m_fg_vidram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::fg_atrram_w)
{
	m_fg_atrram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::bg_vidram_w)
{
	m_bg_vidram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::bg_atrram_w)
{
	m_bg_atrram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::reel1_attrram_w)
{
	m_reel1_attrram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::reel2_attrram_w)
{
	m_reel2_attrram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sanghopm_state::reel3_attrram_w)
{
	m_reel3_attrram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(sanghopm_state::get_fg_tile_info)
{
	int const code = m_fg_vidram[tile_index];
	int const attr = m_fg_atrram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code | (attr & 0x0f)<<8,
			(attr & 0x70) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(sanghopm_state::get_bg_tile_info)
{
	int const code = m_bg_vidram[tile_index];
	int const attr = m_bg_atrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0x70) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(sanghopm_state::get_reel1_tile_info)
{
	int const code = m_reel1_ram[tile_index];
	int const attr = m_reel1_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0x70) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(sanghopm_state::get_reel2_tile_info)
{
	int const code = m_reel2_ram[tile_index];
	int const attr = m_reel2_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0x70)>>4,
			0);
}

TILE_GET_INFO_MEMBER(sanghopm_state::get_reel3_tile_info)
{
	int const code = m_reel3_ram[tile_index];
	int const attr = m_reel3_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0x70)>>4,
			0);
}


VIDEO_START_MEMBER(sanghopm_state, sangho)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanghopm_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanghopm_state::get_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanghopm_state::get_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanghopm_state::get_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanghopm_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}


UINT32 sanghopm_state::screen_update_sangho(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_tilemap->set_scrolly(0, -16);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* enable reels (render all the five layers) */
	if (!(m_enable_reg & 0x01))
	{
		for (int i = 0; i < 64;i++)
		{
			m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
			m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
			m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
		}

		// are these hardcoded, or registers?
		rectangle const visible1(0*8, (15+48)*8-1,  4*8,  (4+7)*8-1);
		rectangle const visible2(0*8, (15+48)*8-1, 12*8, (12+7)*8-1);
		rectangle const visible3(0*8, (15+48)*8-1, 20*8, (20+7)*8-1);

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



WRITE8_MEMBER(unkch_state::reel1_attrram_w)
{
	m_reel1_attrram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(unkch_state::reel2_attrram_w)
{
	m_reel2_attrram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(unkch_state::reel3_attrram_w)
{
	m_reel3_attrram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(unkch_state::get_reel1_tile_info)
{
	int const code = m_reel1_ram[tile_index];
	int const attr = m_reel1_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(unkch_state::get_reel2_tile_info)
{
	int const code = m_reel2_ram[tile_index];
	int const attr = m_reel2_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(unkch_state::get_reel3_tile_info)
{
	int const code = m_reel3_ram[tile_index];
	int const attr = m_reel3_attrram[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr & 0x0f)<<8,
			(attr & 0xf0) >> 4,
			0);
}


VIDEO_START_MEMBER(unkch_state, unkch)
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unkch_state::get_reel1_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unkch_state::get_reel2_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unkch_state::get_reel3_tile_info),this),TILEMAP_SCAN_ROWS,8,32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(32);
	m_reel2_tilemap->set_scroll_cols(32);
	m_reel3_tilemap->set_scroll_cols(32);

	m_cmaster_girl_num = 0;
	m_cmaster_girl_pal = 0;
	m_vidreg = 0x00;

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(unkch_state::get_cherrym_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_cm_enable_reg = 0x0b;
}

UINT32 unkch_state::screen_update_unkch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (!(m_cm_enable_reg & 0x01))
		return 0;

	if (m_cm_enable_reg & 0x08)
	{
		// guess, this could be something else completely!!
		// only draw the first 'reels' tilemap, but fullscreen, using alt registers? (or no scrolling at all? - doubtful, see girl)
		if (m_vidreg & 0x40)
		{
			for (int i = 0; i < 32; i++)
			{
				m_reel1_tilemap->set_scrolly(i, -0x08/*m_reel1_scroll[(i*2)+1]*/);
			//  m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[(i*2)+1]);
			//  m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[(i*2)+1]);
			}

			m_reel1_tilemap->draw(screen, bitmap, cliprect, 0, 0);

		}
		// or draw the reels normally?
		else
		{
			for (int i= 0; i < 32; i++)
			{
				m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i*2]);
				m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i*2]);
				m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i*2]);
			}

			const rectangle visible1(0*8, (14+48)*8-1,  3*8,  (3+7)*8-1);
			const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
			const rectangle visible3(0*8, (14+48)*8-1, 17*8, (17+7)*8-1);

			m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
			m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
			m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
		}
	}

	if (m_cm_enable_reg & 0x02)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
