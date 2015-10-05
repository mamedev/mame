// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/m72.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

inline void m72_state::get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum)
{
	int code,attr,color,pri;
	
	// word 0               word 1
	// fftt tttt tttt tttt  ---- ---- zz-? pppp

	// f = flips, t = tilenum, z = pri, p = palette
	// ? = possible more priority

	tile_index *= 2;

	code  = vram[tile_index] & 0xff;
	attr  = vram[tile_index] >> 8;
	color = vram[tile_index+1] & 0xff;

	if (color & 0x80) pri = 2;
	else if (color & 0x40) pri = 1;
	else pri = 0;
/* color & 0x10 is used in bchopper and hharry, more priority? */

	SET_TILE_INFO_MEMBER(gfxnum,
			code + ((attr & 0x3f) << 8),
			color & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
	tileinfo.group = pri;
}

inline void m72_state::rtype2_get_tile_info(tile_data &tileinfo,int tile_index,const UINT16 *vram,int gfxnum)
{
	int code,attr,color,pri;

	// word 0               word 1
	// tttt tttt tttt tttt  ---- ---z zff- pppp

	// f = flips, t = tilenum, z = pri, p = palette


	tile_index *= 2;

	code  = vram[tile_index];
	color = vram[tile_index+1] & 0xff;
	attr  = vram[tile_index+1] >> 8;

	if (attr & 0x01) pri = 2;
	else if (color & 0x80) pri = 1;
	else pri = 0;

/* (vram[tile_index+2] & 0x10) is used by majtitle on the green, but it's not clear for what */
/* (vram[tile_index+3] & 0xfe) are used as well */

	SET_TILE_INFO_MEMBER(gfxnum,
			code,
			color & 0x0f,
			TILE_FLIPYX((color & 0x60) >> 5));
	tileinfo.group = pri;
}


TILE_GET_INFO_MEMBER(m72_state::get_bg_tile_info)
{
	get_tile_info(tileinfo,tile_index,m_videoram2,2);
}

TILE_GET_INFO_MEMBER(m72_state::get_fg_tile_info)
{
	get_tile_info(tileinfo,tile_index,m_videoram1,1);
}

TILE_GET_INFO_MEMBER(m72_state::rtype2_get_bg_tile_info)
{
	rtype2_get_tile_info(tileinfo,tile_index,m_videoram2,1);
}

TILE_GET_INFO_MEMBER(m72_state::rtype2_get_fg_tile_info)
{
	rtype2_get_tile_info(tileinfo,tile_index,m_videoram1,1);
}




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void m72_state::register_savestate()
{
	save_item(NAME(m_raster_irq_position));
	save_item(NAME(m_video_off));
	save_item(NAME(m_scrollx1));
	save_item(NAME(m_scrolly1));
	save_item(NAME(m_scrollx2));
	save_item(NAME(m_scrolly2));
	save_pointer(NAME(m_buffered_spriteram), m_spriteram.bytes()/2);
}


VIDEO_START_MEMBER(m72_state,m72)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_buffered_spriteram = auto_alloc_array(machine(), UINT16, m_spriteram.bytes()/2);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);

	//m_bg_tilemap->set_transmask(2,0x0001,0xfffe);
	m_bg_tilemap->set_transmask(2,0x0007,0xfff8); // needed for lohtj japan warning to look correct
	//m_bg_tilemap->set_transmask(2,0x001f,0xffe0); // needed for nspiritj japan warning to look correct
	// not sure what is needed to be able to see the imgfghto warning message

	memset(m_buffered_spriteram,0,m_spriteram.bytes());

	m_fg_tilemap->set_scrolldx(0,0);
	m_fg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap->set_scrolldx(0,0);
	m_bg_tilemap->set_scrolldy(-128,-128);

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




VIDEO_START_MEMBER(m72_state,rtype2)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_buffered_spriteram = auto_alloc_array(machine(), UINT16, m_spriteram.bytes()/2);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);
	m_bg_tilemap->set_transmask(2,0x0001,0xfffe);

	memset(m_buffered_spriteram,0,m_spriteram.bytes());

	m_fg_tilemap->set_scrolldx(4,0);
	m_fg_tilemap->set_scrolldy(-128,16);

	m_bg_tilemap->set_scrolldx(4,0);
	m_bg_tilemap->set_scrolldy(-128,16);

	register_savestate();
}

VIDEO_START_MEMBER(m72_state,poundfor)
{
	VIDEO_START_CALL_MEMBER(rtype2);

	m_fg_tilemap->set_scrolldx(6,0);
	m_bg_tilemap->set_scrolldx(6,0);
	m_fg_tilemap->set_scrolldy(-128,-128);
	m_bg_tilemap->set_scrolldy(-128,-128);

	save_item(NAME(m_prev));
	save_item(NAME(m_diff));
}

VIDEO_START_MEMBER(m72_state,hharryu)
{
	VIDEO_START_CALL_MEMBER(rtype2);

	m_fg_tilemap->set_scrolldx(4,3);
	m_bg_tilemap->set_scrolldx(6,0);
	m_fg_tilemap->set_scrolldy(-128,-128);
	m_bg_tilemap->set_scrolldy(-128,-128);
}

VIDEO_START_MEMBER(m72_state, m82_common)
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	m_buffered_spriteram = auto_alloc_array(machine(), UINT16, m_spriteram.bytes()/2);

	m_fg_tilemap->set_transmask(0,0xffff,0x0001);
	m_fg_tilemap->set_transmask(1,0x00ff,0xff01);
	m_fg_tilemap->set_transmask(2,0x0001,0xffff);

	m_bg_tilemap->set_transmask(0,0xffff,0x0000);
	m_bg_tilemap->set_transmask(1,0x00ff,0xff00);
	m_bg_tilemap->set_transmask(2,0x0001,0xfffe);

	memset(m_buffered_spriteram,0,m_spriteram.bytes());



	register_savestate();
	save_item(NAME(m_majtitle_rowscroll));
}

/* Major Title has a larger background RAM, and rowscroll */
// the Air Duel conversion on the same PCB does not, is it jumper selectable, or a register, or a different RAM chip?
TILEMAP_MAPPER_MEMBER(m72_state::majtitle_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return row*256 + col;
}

VIDEO_START_MEMBER(m72_state,m82_large)
{
// The tilemap can be 256x64, but seems to be used at 128x64 (scroll wraparound).
// The layout ramains 256x64, the right half is just not displayed.
//  m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,256,64);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_bg_tile_info),this),tilemap_mapper_delegate(FUNC(m72_state::majtitle_scan_rows),this),8,8,128,64);

	VIDEO_START_CALL_MEMBER(m82_common);

	m_fg_tilemap->set_scrolldx(4,0);
	m_fg_tilemap->set_scrolldy(-128,-128);
	
	m_bg_tilemap->set_scrolldx(4,0);
	m_bg_tilemap->set_scrolldy(-128,-128);
}

VIDEO_START_MEMBER(m72_state, m82_small)
{
//  Air Duel expects the regular tilemap size
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m72_state::rtype2_get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);

	VIDEO_START_CALL_MEMBER(m82_common);

// why do the offsets also differ?
	m_fg_tilemap->set_scrolldx(4,3);
	m_fg_tilemap->set_scrolldy(-128,-128);

	m_bg_tilemap->set_scrolldx(6-256,0);
	m_bg_tilemap->set_scrolldy(-128,-128);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_MEMBER(m72_state::palette1_r)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	return m_generic_paletteram_16[offset] | 0xffe0;    /* only D0-D4 are connected */
}

READ16_MEMBER(m72_state::palette2_r)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	return m_generic_paletteram2_16[offset] | 0xffe0;   /* only D0-D4 are connected */
}

inline void m72_state::changecolor(int color,int r,int g,int b)
{
	m_palette->set_pen_color(color,pal5bit(r),pal5bit(g),pal5bit(b));
}

WRITE16_MEMBER(m72_state::palette1_w)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	offset &= 0x0ff;
	changecolor(offset,
			m_generic_paletteram_16[offset + 0x000],
			m_generic_paletteram_16[offset + 0x200],
			m_generic_paletteram_16[offset + 0x400]);
}

WRITE16_MEMBER(m72_state::palette2_w)
{
	/* A9 isn't connected, so 0x200-0x3ff mirrors 0x000-0x1ff etc. */
	offset &= ~0x100;

	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	offset &= 0x0ff;
	changecolor(offset + 256,
			m_generic_paletteram2_16[offset + 0x000],
			m_generic_paletteram2_16[offset + 0x200],
			m_generic_paletteram2_16[offset + 0x400]);
}

WRITE16_MEMBER(m72_state::videoram1_w)
{
	COMBINE_DATA(&m_videoram1[offset]);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(m72_state::videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(m72_state::irq_line_w)
{
	COMBINE_DATA(&m_raster_irq_position);
}

WRITE16_MEMBER(m72_state::scrollx1_w)
{
	COMBINE_DATA(&m_scrollx1);
}

WRITE16_MEMBER(m72_state::scrollx2_w)
{
	COMBINE_DATA(&m_scrollx2);
}

WRITE16_MEMBER(m72_state::scrolly1_w)
{
	COMBINE_DATA(&m_scrolly1);
}

WRITE16_MEMBER(m72_state::scrolly2_w)
{
	COMBINE_DATA(&m_scrolly2);
}

WRITE16_MEMBER(m72_state::dmaon_w)
{
	if (ACCESSING_BITS_0_7)
		memcpy(m_buffered_spriteram, m_spriteram, m_spriteram.bytes());
}


WRITE16_MEMBER(m72_state::port02_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(machine(), 0,data & 0x01);
		coin_counter_w(machine(), 1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(((data & 0x04) >> 2) ^ ((~ioport("DSW")->read() >> 8) & 1));

		/* bit 3 is display disable */
		m_video_off = data & 0x08;

		/* bit 4 resets sound CPU (active low) */
		if (data & 0x10)
			m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		else
			m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

		/* bit 5 = "bank"? */
	}
}

WRITE16_MEMBER(m72_state::rtype2_port02_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data & 0xe0) logerror("write %02x to port 02\n",data);

		/* bits 0/1 are coin counters */
		coin_counter_w(machine(), 0,data & 0x01);
		coin_counter_w(machine(), 1,data & 0x02);

		/* bit 2 is flip screen (handled both by software and hardware) */
		flip_screen_set(((data & 0x04) >> 2) ^ ((~ioport("DSW")->read() >> 8) & 1));

		/* bit 3 is display disable */
		m_video_off = data & 0x08;

		/* other bits unknown */
	}
}



/* the following is mostly a kludge. This register seems to be used for something else */
WRITE16_MEMBER(m72_state::majtitle_gfx_ctrl_w)
{
	if (ACCESSING_BITS_8_15)
	{
		if (data & 0xff00) m_majtitle_rowscroll = 1;
		else m_majtitle_rowscroll = 0;
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

void m72_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	UINT16 *spriteram = m_buffered_spriteram;
	int offs;

	offs = 0;
	while (offs < m_spriteram.bytes()/2)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = spriteram[offs+1];
		color = spriteram[offs+2] & 0x0f;
		sx = -256+(spriteram[offs+3] & 0x3ff);
		sy = 384-(spriteram[offs+0] & 0x1ff);
		flipx = spriteram[offs+2] & 0x0800;
		flipy = spriteram[offs+2] & 0x0400;

		w = 1 << ((spriteram[offs+2] & 0xc000) >> 14);
		h = 1 << ((spriteram[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen())
		{
			sx = 512 - 16*w - sx;
			sy = 284 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (x = 0;x < w;x++)
		{
			for (y = 0;y < h;y++)
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
	UINT16 *spriteram16_2 = m_spriteram2;
	int offs;

	for (offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		int code,color,sx,sy,flipx,flipy,w,h,x,y;


		code = spriteram16_2[offs+1];
		color = spriteram16_2[offs+2] & 0x0f;
		sx = -256+(spriteram16_2[offs+3] & 0x3ff);
		sy = 384-(spriteram16_2[offs+0] & 0x1ff);
		flipx = spriteram16_2[offs+2] & 0x0800;
		flipy = spriteram16_2[offs+2] & 0x0400;

		w = 1;// << ((spriteram16_2[offs+2] & 0xc000) >> 14);
		h = 1 << ((spriteram16_2[offs+2] & 0x3000) >> 12);
		sy -= 16 * h;

		if (flip_screen())
		{
			sx = 512 - 16*w - sx;
			sy = 256 - 16*h - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		for (x = 0;x < w;x++)
		{
			for (y = 0;y < h;y++)
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

UINT32 m72_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_video_off)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_fg_tilemap->set_scrollx(0,m_scrollx1);
	m_fg_tilemap->set_scrolly(0,m_scrolly1);

	m_bg_tilemap->set_scrollx(0,m_scrollx2);
	m_bg_tilemap->set_scrolly(0,m_scrolly2);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}

UINT32 m72_state::screen_update_majtitle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;


	if (m_video_off)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_fg_tilemap->set_scrollx(0,m_scrollx1);
	m_fg_tilemap->set_scrolly(0,m_scrolly1);

	if (m_majtitle_rowscroll)
	{
		m_bg_tilemap->set_scroll_rows(512);
		for (i = 0;i < 512;i++)
			m_bg_tilemap->set_scrollx((i+m_scrolly2)&0x1ff,
					256 + m_majtitle_rowscrollram[i]);
	}
	else
	{
		m_bg_tilemap->set_scroll_rows(1);
		m_bg_tilemap->set_scrollx(0,256 + m_scrollx2);
	}
	m_bg_tilemap->set_scrolly(0,m_scrolly2);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1,0);
	majtitle_draw_sprites(bitmap,cliprect);
	draw_sprites(bitmap,cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0,0);
	return 0;
}
