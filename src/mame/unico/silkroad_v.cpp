// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
#include "emu.h"
#include "silkroad.h"
#include "screen.h"

/* Sprites probably need to be delayed */
/* Some scroll layers may need to be offset slightly? */
/* Check Sprite Colours after redump */
/* Clean Up */
/* is theres a bg colour register? */

void silkroad_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint32_t *source = m_sprram;
	uint32_t *finish = source + 0x1000/4;

	while( source < finish )
	{
		int xpos = (source[0] & 0x01ff0000) >> 16;
		int ypos = (source[0] & 0x0000ffff);
		int tileno = (source[1] & 0xffff0000) >> 16;
		int attr = (source[1] & 0x0000ffff);
		int flipx = (attr & 0x0080);
		int width = ((attr & 0x0f00) >> 8) + 1;
		int wcount;
		int color = (attr & 0x003f) ;
		int pri      =  ((attr & 0x1000)>>12);  // Priority (1 = Low)
		int pri_mask =  ~((1 << (pri+1)) - 1);  // Above the first "pri" levels

		// attr & 0x2000 -> another priority bit?

		if ( (source[1] & 0xff00) == 0xff00 ) break;

		if ( (attr & 0x8000) == 0x8000 ) tileno+=0x10000;

		if (!flipx)
		{
			for (wcount=0;wcount<width;wcount++)
			{
				gfx->prio_transpen(bitmap,cliprect,tileno+wcount,color,0,0,xpos+wcount*16+8,ypos,screen.priority(),pri_mask,0);
			}
		}
		else
		{
			for (wcount=width;wcount>0;wcount--)
			{
				gfx->prio_transpen(bitmap,cliprect,tileno+(width-wcount),color,1,0,xpos+wcount*16-16+8,ypos,screen.priority(),pri_mask,0);
			}
		}

		source += 2;
	}
}

template<int Layer>
TILE_GET_INFO_MEMBER(silkroad_state::get_tile_info)
{
	int code = ((m_vram[Layer][tile_index] & 0xffff0000) >> 16 );
	int color = ((m_vram[Layer][tile_index] & 0x000001f));
	int flipx =  ((m_vram[Layer][tile_index] & 0x0000080) >> 7);

	code += 0x18000;

	tileinfo.set(0,
			code,
			color,
			TILE_FLIPYX(flipx));
}

void silkroad_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(silkroad_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(silkroad_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(silkroad_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
}

uint32_t silkroad_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(0x7c0, cliprect);

	m_tilemap[0]->set_scrollx(0, ((m_regs[0] & 0xffff0000) >> 16) );
	m_tilemap[0]->set_scrolly(0, (m_regs[0] & 0x0000ffff) >> 0 );

	m_tilemap[2]->set_scrolly(0, (m_regs[1] & 0xffff0000) >> 16 );
	m_tilemap[2]->set_scrollx(0, (m_regs[2] & 0xffff0000) >> 16 );

	m_tilemap[1]->set_scrolly(0, ((m_regs[5] & 0xffff0000) >> 16));
	m_tilemap[1]->set_scrollx(0, (m_regs[2] & 0x0000ffff) >> 0 );

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0,1);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0,2);
	draw_sprites(screen,bitmap,cliprect);

	if (0)
	{
		popmessage ("Regs %08x %08x %08x %08x %08x",
		m_regs[0],
		m_regs[1],
		m_regs[2],
		m_regs[4],
		m_regs[5]);
	}

	return 0;
}
