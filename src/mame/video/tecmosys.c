// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

 tecmosys video driver

***************************************************************************/

#include "emu.h"
#include "includes/tecmosys.h"


TILE_GET_INFO_MEMBER(tecmosys_state::get_bg0tile_info)
{
	SET_TILE_INFO_MEMBER(1,
			m_bg0tilemap_ram[2*tile_index+1],
			(m_bg0tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((m_bg0tilemap_ram[2*tile_index]&0xc0)>>6));
}

TILE_GET_INFO_MEMBER(tecmosys_state::get_bg1tile_info)
{
	SET_TILE_INFO_MEMBER(2,
			m_bg1tilemap_ram[2*tile_index+1],
			(m_bg1tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((m_bg1tilemap_ram[2*tile_index]&0xc0)>>6));
}

TILE_GET_INFO_MEMBER(tecmosys_state::get_bg2tile_info)
{
	SET_TILE_INFO_MEMBER(3,
			m_bg2tilemap_ram[2*tile_index+1],
			(m_bg2tilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((m_bg2tilemap_ram[2*tile_index]&0xc0)>>6));
}

TILE_GET_INFO_MEMBER(tecmosys_state::get_fg_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_fgtilemap_ram[2*tile_index+1],
			(m_fgtilemap_ram[2*tile_index]&0x3f),
			TILE_FLIPYX((m_fgtilemap_ram[2*tile_index]&0xc0)>>6));
}


WRITE16_MEMBER(tecmosys_state::bg0_tilemap_w)
{
	COMBINE_DATA(&m_bg0tilemap_ram[offset]);
	m_bg0tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(tecmosys_state::bg1_tilemap_w)
{
	COMBINE_DATA(&m_bg1tilemap_ram[offset]);
	m_bg1tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(tecmosys_state::bg2_tilemap_w)
{
	COMBINE_DATA(&m_bg2tilemap_ram[offset]);
	m_bg2tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(tecmosys_state::fg_tilemap_w)
{
	COMBINE_DATA(&m_fgtilemap_ram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset/2);
}


inline void tecmosys_state::set_color_555(pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	m_palette->set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_MEMBER(tecmosys_state::tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w)
{
	COMBINE_DATA(&m_tilemap_paletteram16[offset]);
	set_color_555(offset+0x4000, 5, 10, 0, m_tilemap_paletteram16[offset]);
}

WRITE16_MEMBER(tecmosys_state::bg0_tilemap_lineram_w)
{
	COMBINE_DATA(&m_bg0tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg0 lineram %04x %04x",offset,data);
}

WRITE16_MEMBER(tecmosys_state::bg1_tilemap_lineram_w)
{
	COMBINE_DATA(&m_bg1tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg1 lineram %04x %04x",offset,data);
}

WRITE16_MEMBER(tecmosys_state::bg2_tilemap_lineram_w)
{
	COMBINE_DATA(&m_bg2tilemap_lineram[offset]);
	if (data!=0x0000) popmessage("non 0 write to bg2 lineram %04x %04x",offset,data);
}



void tecmosys_state::render_sprites_to_bitmap(bitmap_rgb32 &bitmap, UINT16 extrax, UINT16 extray )
{
	UINT8 *gfxsrc    = memregion       ( "gfx1" )->base();
	int i;

	/* render sprites (with priority information) to temp bitmap */
	m_sprite_bitmap.fill(0x0000);
	/* there are multiple spritelists in here, to allow for buffering */
	for (i=(m_spritelist*0x4000)/2;i<((m_spritelist+1)*0x4000)/2;i+=8)
	{
		int xcnt,ycnt;
		int drawx, drawy;
		UINT16* dstptr;

		int x, y;
		int address;
		int xsize = 16;
		int ysize = 16;
		int colour;
		int flipx, flipy;
		int priority;
		int zoomx, zoomy;

		x = m_spriteram[i+0]+386;
		y = (m_spriteram[i+1]+1);

		x-= extrax;
		y-= extray;

		y&=0x1ff;
		x&=0x3ff;

		if (x&0x200) x-=0x400;
		if (y&0x100) y-=0x200;

		address =  m_spriteram[i+5]| ((m_spriteram[i+4]&0x000f)<<16);

		address<<=8;

		flipx = (m_spriteram[i+4]&0x0040)>>6;
		flipy = (m_spriteram[i+4]&0x0080)>>7; // used by some move effects in tkdensho

		zoomx = (m_spriteram[i+2] & 0x0fff)>>0; // zoom?
		zoomy = (m_spriteram[i+3] & 0x0fff)>>0; // zoom?

		if ((!zoomx) || (!zoomy)) continue;

		ysize =  ((m_spriteram[i+6] & 0x00ff))*16;
		xsize =  (((m_spriteram[i+6] & 0xff00)>>8))*16;

		colour =  ((m_spriteram[i+4] & 0x3f00))>>8;

		priority = ((m_spriteram[i+4] & 0x0030))>>4;

		if (m_spriteram[i+4] & 0x8000) continue;

		for (ycnt = 0; ycnt < ysize; ycnt++)
		{
			int actualycnt = (ycnt * zoomy) >> 8;
			int actualysize = (ysize * zoomy) >> 8;

			if (flipy) drawy = y + (actualysize-1) - actualycnt;
			else drawy = y + actualycnt;

			for (xcnt = 0; xcnt < xsize; xcnt++)
			{
				int actualxcnt = (xcnt * zoomx) >> 8;
				int actualxsize = (xsize *zoomx) >> 8;

				if (flipx) drawx = x + (actualxsize-1) - actualxcnt;
				else drawx = x + actualxcnt;

				if ((drawx>=0 && drawx<320) && (drawy>=0 && drawy<240))
				{
					UINT8 data;

					dstptr = &m_sprite_bitmap.pix16(drawy, drawx);


					data =  (gfxsrc[address]);


					if(data) dstptr[0] = (data + (colour*0x100)) | (priority << 14);
				}

				address++;
			}
		}
	}
}

void tecmosys_state::tilemap_copy_to_compose(UINT16 pri)
{
	int y,x;
	UINT16 *srcptr;
	UINT16 *dstptr;
	for (y=0;y<240;y++)
	{
		srcptr = &m_tmp_tilemap_renderbitmap.pix16(y);
		dstptr = &m_tmp_tilemap_composebitmap.pix16(y);
		for (x=0;x<320;x++)
		{
			if ((srcptr[x]&0xf)!=0x0)
				dstptr[x] =  (srcptr[x]&0x7ff) | pri;
		}
	}
}

void tecmosys_state::do_final_mix(bitmap_rgb32 &bitmap)
{
	const pen_t *paldata = m_palette->pens();
	int y,x;
	UINT16 *srcptr;
	UINT16 *srcptr2;
	UINT32 *dstptr;

	for (y=0;y<240;y++)
	{
		srcptr = &m_tmp_tilemap_composebitmap.pix16(y);
		srcptr2 = &m_sprite_bitmap.pix16(y);

		dstptr = &bitmap.pix32(y);
		for (x=0;x<320;x++)
		{
			UINT16 pri, pri2;
			UINT16 penvalue;
			UINT16 penvalue2;
			UINT32 colour;
			UINT32 colour2;

			pri = srcptr[x] & 0xc000;
			pri2 = srcptr2[x] & 0xc000;

			penvalue = m_tilemap_paletteram16[srcptr[x]&0x7ff];
			colour =   paldata[(srcptr[x]&0x7ff) | 0x4000];

			if (srcptr2[x]&0x3fff)
			{
				penvalue2 = m_palette->basemem().read(srcptr2[x] & 0x3fff);
				colour2 = paldata[srcptr2[x]&0x3fff];
			}
			else
			{
				penvalue2 = m_tilemap_paletteram16[srcptr[x]&0x7ff];
				colour2 =   paldata[(srcptr[x]&0x7ff) | 0x4000];
			}

			if ((penvalue & 0x8000) && (penvalue2 & 0x8000)) // blend
			{
				int r,g,b;
				int r2,g2,b2;
				b = (colour & 0x000000ff) >> 0;
				g = (colour & 0x0000ff00) >> 8;
				r = (colour & 0x00ff0000) >> 16;

				b2 = (colour2 & 0x000000ff) >> 0;
				g2 = (colour2 & 0x0000ff00) >> 8;
				r2 = (colour2 & 0x00ff0000) >> 16;

				r = (r + r2) >> 1;
				g = (g + g2) >> 1;
				b = (b + b2) >> 1;

				dstptr[x] = b | (g<<8) | (r<<16);
			}
			else if (pri2 >= pri)
			{
				dstptr[x] = colour2;
			}
			else
			{
				dstptr[x] = colour;
			}
		}
	}
}


UINT32 tecmosys_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0x4000), cliprect);


	m_bg0tilemap->set_scrolly(0, m_c80000regs[1]+16);
	m_bg0tilemap->set_scrollx(0, m_c80000regs[0]+104);

	m_bg1tilemap->set_scrolly(0, m_a80000regs[1]+17);
	m_bg1tilemap->set_scrollx(0, m_a80000regs[0]+106);

	m_bg2tilemap->set_scrolly(0, m_b00000regs[1]+17);
	m_bg2tilemap->set_scrollx(0, m_b00000regs[0]+106);

	m_tmp_tilemap_composebitmap.fill(0, cliprect);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_bg0tilemap->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x0000);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_bg1tilemap->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x4000);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_bg2tilemap->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x8000);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_txt_tilemap->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0xc000);


	do_final_mix(bitmap);

	// prepare sprites for NEXT frame - causes 1 frame palette errors, but prevents sprite lag in tkdensho, which is correct?
	render_sprites_to_bitmap(bitmap, m_880000regs[0x0], m_880000regs[0x1]);

	return 0;
}

void tecmosys_state::video_start()
{
	m_sprite_bitmap.allocate(320,240);
	m_sprite_bitmap.fill(0x4000);

	m_tmp_tilemap_composebitmap.allocate(320,240);
	m_tmp_tilemap_renderbitmap.allocate(320,240);

	m_tmp_tilemap_composebitmap.fill(0x0000);
	m_tmp_tilemap_renderbitmap.fill(0x0000);


	m_txt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmosys_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32*2,32*2);
	m_txt_tilemap->set_transparent_pen(0);

	m_bg0tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmosys_state::get_bg0tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_bg0tilemap->set_transparent_pen(0);

	m_bg1tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmosys_state::get_bg1tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_bg1tilemap->set_transparent_pen(0);

	m_bg2tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmosys_state::get_bg2tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_bg2tilemap->set_transparent_pen(0);

	save_item(NAME(m_spritelist));
}
