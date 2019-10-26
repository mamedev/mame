// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/m72.h"
#include "cpu/nec/v25.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

inline void m72_state::m72_m81_get_tile_info(tile_data &tileinfo,int tile_index,const u16 *vram,int gfxnum)
{
	int pri;

	// word 0               word 1
	// fftt tttt tttt tttt  ---- ---- zz-? pppp

	// f = flips, t = tilenum, z = pri, p = palette
	// ? = possible more priority

	tile_index *= 2;

	const u16 code = vram[tile_index];
	const u16 attr = vram[tile_index+1];

	if (attr & 0x0080) pri = 2;
	else if (attr & 0x0040) pri = 1;
	else pri = 0;
/* attr & 0x0010 is used in bchopper and hharry, more priority? */

	SET_TILE_INFO_MEMBER(gfxnum,
			code & 0x3fff,
			attr & 0x000f,
			TILE_FLIPYX((code & 0xc000) >> 14));
	tileinfo.group = pri;
}


TILE_GET_INFO_MEMBER(m72_state::get_bg_tile_info)
{
	m72_m81_get_tile_info(tileinfo,tile_index,m_videoram[1],m_bg_source);
}

TILE_GET_INFO_MEMBER(m72_state::get_fg_tile_info)
{
	m72_m81_get_tile_info(tileinfo,tile_index,m_videoram[0],m_fg_source);
}

template<unsigned N>
TILE_GET_INFO_MEMBER(m72_state::rtype2_get_tile_info)
{
	int pri;

	// word 0               word 1
	// tttt tttt tttt tttt  ---- ---z zff- pppp

	// f = flips, t = tilenum, z = pri, p = palette

	tile_index *= 2;

	const u16 code = m_videoram[N][tile_index];
	const u16 attr = m_videoram[N][tile_index+1];

	if (attr & 0x0100) pri = 2;
	else if (attr & 0x0080) pri = 1;
	else pri = 0;

/* (m_videoram[N][tile_index+2] & 0x10) is used by majtitle on the green, but it's not clear for what */
/* (m_videoram[N][tile_index+3] & 0xfe) are used as well */

	SET_TILE_INFO_MEMBER(1,
			code,
			attr & 0x000f,
			TILE_FLIPYX((attr & 0x0060) >> 5));
	tileinfo.group = pri;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void m72_state::register_savestate()
{
	save_item(NAME(m_raster_irq_position));
	save_item(NAME(m_video_off));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}


VIDEO_START_MEMBER(m72_state,m72)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,64);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);

	//m_bg_tilemap->set_transmask(2,0x0001,0xfffe);
	m_bg_tilemap->set_transmask(2,0x0007,0xfff8); // needed for lohtj japan warning to look correct

	memset(m_spriteram->buffer(),0,m_spriteram->bytes());

	m_fg_tilemap->set_scrolldx(0,0);
	m_fg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap->set_scrolldx(0,0);
	m_bg_tilemap->set_scrolldy(-128,-128);

	// on M72 the FG data always comes from the Ax roms and the BG data always comes from the Bx roms
	m_fg_source = 1;
	m_bg_source = 2;

	register_savestate();
}

VIDEO_START_MEMBER(m72_state,xmultipl)
{
	VIDEO_START_CALL_MEMBER(m72);

	m_fg_tilemap->set_scrolldx(4,64);
	m_fg_tilemap->set_scrolldy(-128, 0);

	m_bg_tilemap->set_scrolldx(6,0);
	m_bg_tilemap->set_scrolldy(-128,-128);

}


VIDEO_START_MEMBER(m72_state,hharry)
{
	VIDEO_START_CALL_MEMBER(m72);

	m_bg_tilemap->set_transmask(2,0x0001,0xfffe); // ? maybe the standard logic is ok.

	m_fg_tilemap->set_scrolldx(4,3);
	m_fg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap->set_scrolldx(6,0);
	m_bg_tilemap->set_scrolldy(-128,16);
}

VIDEO_START_MEMBER(m72_state,imgfightj)
{
	VIDEO_START_CALL_MEMBER(m72);
	m_bg_tilemap->set_transmask(2,0xff00,0x00ff); // for japan message
}

VIDEO_START_MEMBER(m72_state,nspiritj)
{
	VIDEO_START_CALL_MEMBER(m72);
	m_bg_tilemap->set_transmask(2,0x001f,0xffe0); // for japan message
}

VIDEO_START_MEMBER(m72_state,mrheli)
{
	VIDEO_START_CALL_MEMBER(m72);
	m_bg_tilemap->set_transmask(2,0x00ff,0xff00); // for japan message
}


/* Major Title has a larger background RAM, and rowscroll */
// the Air Duel conversion on the same PCB does not, is it jumper selectable, or a register, or a different RAM chip?
TILEMAP_MAPPER_MEMBER(m72_state::m82_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return row*256 + col;
}

VIDEO_START_MEMBER(m72_state,m82)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::rtype2_get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::rtype2_get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
// The tilemap can be 256x64, but seems to be used at 128x64 (scroll wraparound).
// The layout ramains 256x64, the right half is just not displayed.
	m_bg_tilemap_large = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::rtype2_get_tile_info<1>)), tilemap_mapper_delegate(*this, FUNC(m72_state::m82_scan_rows)), 8,8, 128,64);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);
	m_bg_tilemap->set_transmask(2,0x0001,0xfffe);

	m_bg_tilemap_large->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap_large->set_transmask(1,0x00ff,0xff00);
	m_bg_tilemap_large->set_transmask(2,0x0001,0xfffe);

	m_fg_tilemap->set_scrolldx(4,0);
	m_fg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap->set_scrolldx(6-256,0);
	m_bg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap_large->set_scrolldx(4,0);
	m_bg_tilemap_large->set_scrolldy(-128,-128);

	memset(m_spriteram->buffer(),0,m_spriteram->bytes());

	register_savestate();
	save_item(NAME(m_m82_rowscroll));
	save_item(NAME(m_m82_tmcontrol));
}


// M84
VIDEO_START_MEMBER(m72_state,rtype2)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::rtype2_get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m72_state::rtype2_get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 64,64);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);
	m_bg_tilemap->set_transmask(2,0x0001,0xfffe);

	memset(m_spriteram->buffer(),0,m_spriteram->bytes());

	m_fg_tilemap->set_scrolldx(4,0);
	m_fg_tilemap->set_scrolldy(-128,16);

	m_bg_tilemap->set_scrolldx(4,0);
	m_bg_tilemap->set_scrolldy(-128,16);

	register_savestate();
}


VIDEO_START_MEMBER(m72_state,hharryu)
{
	VIDEO_START_CALL_MEMBER(rtype2);

	m_fg_tilemap->set_scrolldx(4,3);
	m_bg_tilemap->set_scrolldx(6,0);
	m_fg_tilemap->set_scrolldy(-128,-128);
	m_bg_tilemap->set_scrolldy(-128,-128);
}

// m85
VIDEO_START_MEMBER(m72_state,poundfor)
{
	VIDEO_START_CALL_MEMBER(rtype2);

	m_fg_tilemap->set_scrolldx(6,0);
	m_bg_tilemap->set_scrolldx(6,0);
	m_fg_tilemap->set_scrolldy(-128,-128);
	m_bg_tilemap->set_scrolldy(-128,-128);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void m72_state::videoram1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[0][offset]);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

void m72_state::videoram2_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoram[1][offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);

	// m82 has selectable tilemap size
	if (m_bg_tilemap_large)
		m_bg_tilemap_large->mark_tile_dirty(offset/2);

}

void m72_state::irq_line_w(u16 data)
{
	// KNA70H015(11): ISET
	m_raster_irq_position = data & 0x1ff;
//  printf("m_raster_irq_position %04x\n", m_raster_irq_position);

	// bchopper title screen jumps around, as does ingame at times, if this isn't done here
	if (m_upd71059c.found())
		m_upd71059c->ir2_w(0);
	else
		m_maincpu->set_input_line(NEC_INPUT_LINE_INTP2, CLEAR_LINE);
}

void m72_state::dmaon_w(u8 data)
{
	m_spriteram->copy();
}


void m72_state::port02_w(u8 data)
{
	if (data & 0xe0) logerror("write %02x to port 02\n",data);

	/* bits 0/1 are coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 0x01);
	machine().bookkeeping().coin_counter_w(1,data & 0x02);

	/* bit 2 is flip screen (handled both by software and hardware) */
	flip_screen_set(((data & 0x04) >> 2) ^ ((~m_io_dsw->read() >> 8) & 1));

	/* bit 3 is display disable */
	m_video_off = data & 0x08;

	/* bit 4 resets sound CPU (active low) */
	if (data & 0x10)
		m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	/* bit 5 = "bank"? */
}

void m72_state::rtype2_port02_w(u8 data)
{
	if (data & 0xe0) logerror("write %02x to port 02\n",data);

	/* bits 0/1 are coin counters */
	machine().bookkeeping().coin_counter_w(0,data & 0x01);
	machine().bookkeeping().coin_counter_w(1,data & 0x02);

	/* bit 2 is flip screen (handled both by software and hardware) */
	flip_screen_set(((data & 0x04) >> 2) ^ ((~m_io_dsw->read() >> 8) & 1));

	/* bit 3 is display disable */
	m_video_off = data & 0x08;

	/* other bits unknown */
}

void m72_state::poundfor_port02_w(u8 data)
{
	// bit 5 resets both uPD4701A?
	m_upd4701[0]->resetx_w(BIT(data, 5));
	m_upd4701[0]->resety_w(BIT(data, 5));
	m_upd4701[1]->resetx_w(BIT(data, 5));
	m_upd4701[1]->resety_w(BIT(data, 5));

	rtype2_port02_w(data & 0xbf);
}



/* the following is mostly a kludge. This register seems to be used for something else */
void m72_state::m82_gfx_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00) m_m82_rowscroll = 1;
		else m_m82_rowscroll = 0;
	}
//  printf("m82_gfx_ctrl_w %04x\n", data);

}

void m72_state::m82_tm_ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_m82_tmcontrol);
//  printf("tmcontrol %04x\n", m_m82_tmcontrol);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void m72_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	u16 *spriteram = m_spriteram->buffer();
	int offs = 0;

	while (offs < m_spriteram->bytes()/2)
	{
		const int code = spriteram[offs+1];
		const u32 color = spriteram[offs+2] & 0x0f;
		int sx = -256+(spriteram[offs+3] & 0x3ff);
		int sy = 384-(spriteram[offs+0] & 0x1ff);
		int flipx = spriteram[offs+2] & 0x0800;
		int flipy = spriteram[offs+2] & 0x0400;

		const int w = 1 << ((spriteram[offs+2] & 0xc000) >> 14);
		const int h = 1 << ((spriteram[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen())
		{
			sx = 512 - 16*w - sx;
			sy = 284 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int c = code;

				if (flipx) c += 8*(w-1-x);
				else c += 8*x;
				if (flipy) c += h-1-y;
				else c += y;

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,0);
			}
		}

		offs += w*4;
	}
}

void m72_state::majtitle_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	u16 *spriteram16_2 = m_spriteram2;

	for (int offs = 0; offs < m_spriteram2.bytes(); offs += 4)
	{
		const int code = spriteram16_2[offs+1];
		const u32 color = spriteram16_2[offs+2] & 0x0f;
		int sx = -256+(spriteram16_2[offs+3] & 0x3ff);
		int sy = 384-(spriteram16_2[offs+0] & 0x1ff);
		int flipx = spriteram16_2[offs+2] & 0x0800;
		int flipy = spriteram16_2[offs+2] & 0x0400;

		const int w = 1;// << ((spriteram16_2[offs+2] & 0xc000) >> 14);
		const int h = 1 << ((spriteram16_2[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen())
		{
			sx = 512 - 16*w - sx;
			sy = 256 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int c = code;

				if (flipx) c += 8*(w-1-x);
				else c += 8*x;
				if (flipy) c += h-1-y;
				else c += y;

				m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
						c,
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,0);
			}
		}
	}
}

u32 m72_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_off)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_fg_tilemap->set_scrollx(0,m_scrollx[0]);
	m_fg_tilemap->set_scrolly(0,m_scrolly[0]);

	m_bg_tilemap->set_scrollx(0,m_scrollx[1]);
	m_bg_tilemap->set_scrolly(0,m_scrolly[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}

u32 m72_state::screen_update_m81(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// on M81 the FG data always comes from the Ax roms
	// the source of the BG data however depends on Jumper J3
	const int J3 = m_m81_b_b_j3->read();
	if (J3 == 0) m_bg_source = 1;
	else m_bg_source = 2;

	return screen_update(screen, bitmap, cliprect);
}


u32 m72_state::screen_update_m82(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tilemap_t* tm;

	if (m_m82_tmcontrol & 0x40) tm = m_bg_tilemap_large;
	else  tm = m_bg_tilemap;

	if (m_video_off)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_fg_tilemap->set_scrollx(0,m_scrollx[0]);
	m_fg_tilemap->set_scrolly(0,m_scrolly[0]);

	if (m_m82_rowscroll)
	{
		tm->set_scroll_rows(512);
		for (int i = 0; i < 512; i++)
			tm->set_scrollx((i+m_scrolly[1])&0x1ff,
					256 + m_m82_rowscrollram[i]);
	}
	else
	{
		tm->set_scroll_rows(1);
		tm->set_scrollx(0,256 + m_scrollx[1]);
	}

	tm->set_scrolly(0,m_scrolly[1]);
	tm->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	majtitle_draw_sprites(bitmap,cliprect);
	draw_sprites(bitmap,cliprect);
	tm->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}
