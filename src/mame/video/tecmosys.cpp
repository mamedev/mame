// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

 tecmosys video driver

***************************************************************************/

#include "emu.h"
#include "includes/tecmosys.h"


template<int Layer>
TILE_GET_INFO_MEMBER(tecmosys_state::get_tile_info)
{
	SET_TILE_INFO_MEMBER(Layer,
			m_vram[Layer][2*tile_index+1],
			(m_vram[Layer][2*tile_index]&0x3f),
			TILE_FLIPYX((m_vram[Layer][2*tile_index]&0xc0)>>6));
}


inline void tecmosys_state::set_color_555(pen_t color, int rshift, int gshift, int bshift, uint16_t data)
{
	m_palette->set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

WRITE16_MEMBER(tecmosys_state::tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w)
{
	COMBINE_DATA(&m_tilemap_paletteram16[offset]);
	set_color_555(offset+0x4000, 5, 10, 0, m_tilemap_paletteram16[offset]);
}

void tecmosys_state::render_sprites_to_bitmap(bitmap_rgb32 &bitmap, uint16_t extrax, uint16_t extray )
{
	int i;

	/* render sprites (with priority information) to temp bitmap */
	m_sprite_bitmap.fill(0x0000);
	/* there are multiple spritelists in here, to allow for buffering */
	for (i=(m_spritelist*0x4000)/2;i<((m_spritelist+1)*0x4000)/2;i+=8)
	{
		int xcnt,ycnt;
		int drawx, drawy;
		uint16_t* dstptr;

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
					uint8_t data;

					dstptr = &m_sprite_bitmap.pix16(drawy, drawx);


					data =  (m_sprite_region[address]);


					if(data) dstptr[0] = (data + (colour*0x100)) | (priority << 14);
				}

				address++;
			}
		}
	}
}

void tecmosys_state::tilemap_copy_to_compose(uint16_t pri, const rectangle &cliprect)
{
	int y,x;
	uint16_t *srcptr;
	uint16_t *dstptr;
	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcptr = &m_tmp_tilemap_renderbitmap.pix16(y);
		dstptr = &m_tmp_tilemap_composebitmap.pix16(y);
		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			if ((srcptr[x]&0xf)!=0x0)
				dstptr[x] =  (srcptr[x]&0x7ff) | pri;
		}
	}
}


void tecmosys_state::do_final_mix(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	for (int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		uint16_t const *const srcptr = &m_tmp_tilemap_composebitmap.pix16(y);
		uint16_t const *const srcptr2 = &m_sprite_bitmap.pix16(y);

		uint32_t *const dstptr = &bitmap.pix32(y);
		for (int x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			uint16_t const pri = srcptr[x] & 0xc000;
			uint16_t const pri2 = srcptr2[x] & 0xc000;

			uint16_t const penvalue = m_tilemap_paletteram16[srcptr[x]&0x7ff];
			uint32_t const colour = paldata[(srcptr[x]&0x7ff) | 0x4000];

			uint16_t penvalue2;
			uint32_t colour2;
			uint8_t mask = 0;
			bool is_transparent = false;
			if (srcptr2[x]&0xff)
			{
				penvalue2 = m_palette->basemem().read(srcptr2[x] & 0x3fff);
				colour2 = paldata[srcptr2[x]&0x3fff];
				mask = srcptr2[x]&0xff;
			}
			else if (srcptr[x]&0xf)
			{
				penvalue2 = m_tilemap_paletteram16[srcptr[x]&0x7ff];
				colour2 =   paldata[(srcptr[x]&0x7ff) | 0x4000];
				mask = srcptr[x]&0xf;
			}
			else
			{
				penvalue2 = m_palette->basemem().read(0x3f00);
				colour2 =   paldata[0x3f00];
				is_transparent = true;
			}

			auto const draw_blended =
					[] (uint32_t colour, uint32_t colour2, uint32_t &dst)
					{
						int b = (colour & 0x000000ff) >> 0;
						int g = (colour & 0x0000ff00) >> 8;
						int r = (colour & 0x00ff0000) >> 16;

						int const b2 = (colour2 & 0x000000ff) >> 0;
						int const g2 = (colour2 & 0x0000ff00) >> 8;
						int const r2 = (colour2 & 0x00ff0000) >> 16;

						r = (r + r2) >> 1;
						g = (g + g2) >> 1;
						b = (b + b2) >> 1;

						dst = b | (g<<8) | (r<<16);
					};
			auto const draw_bg =
					[this] (uint32_t colour, uint32_t src, uint32_t &dst)
					{
						if (src&0xf)
							dst = colour;
						else
							dst = m_palette->pen(0x3f00);
					};

			if (is_transparent)
			{
				if (((penvalue & 0x8000) && (srcptr[x]&0xf)) && (penvalue2 & 0x8000)) // blend
					draw_blended(colour, colour2, dstptr[x]);
				else
					draw_bg(colour, srcptr[x], dstptr[x]);
			}
			else
			{
				if (((penvalue & 0x8000) && (srcptr[x]&0xf)) && ((penvalue2 & 0x8000) && (mask))) // blend
					draw_blended(colour, colour2, dstptr[x]);
				else if ((pri2 >= pri) && (mask))
					dstptr[x] = colour2;
				else
					draw_bg(colour, srcptr[x], dstptr[x]);
			}
		}
	}
}


uint32_t tecmosys_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// TODO : Verify 0xc00000-0xc00003 scroll both flip screen and normal screen case, Bit 4 of 0xc00004 is unknown
	// TODO : is tilemap flip bits is correct?
	m_tilemap[0]->set_flip(((m_scroll[0][2] & 1) ? TILEMAP_FLIPX : 0) | ((m_scroll[0][2] & 2) ? TILEMAP_FLIPY : 0) );

	// TODO : Scroll is wrong/unverified when flip screen case
	m_tilemap[1]->set_flip(((m_scroll[1][2] & 1) ? TILEMAP_FLIPX : 0) | ((m_scroll[1][2] & 2) ? TILEMAP_FLIPY : 0) );
	m_tilemap[1]->set_scrolly(0, m_scroll[1][1]+16);
	m_tilemap[1]->set_scrollx(0, m_scroll[1][0]+104);

	m_tilemap[2]->set_flip(((m_scroll[2][2] & 1) ? TILEMAP_FLIPX : 0) | ((m_scroll[2][2] & 2) ? TILEMAP_FLIPY : 0) );
	m_tilemap[2]->set_scrolly(0, m_scroll[2][1]+17);
	m_tilemap[2]->set_scrollx(0, m_scroll[2][0]+106);

	m_tilemap[3]->set_flip(((m_scroll[3][2] & 1) ? TILEMAP_FLIPX : 0) | ((m_scroll[3][2] & 2) ? TILEMAP_FLIPY : 0) );
	m_tilemap[3]->set_scrolly(0, m_scroll[3][1]+17);
	m_tilemap[3]->set_scrollx(0, m_scroll[3][0]+106);

	m_tmp_tilemap_composebitmap.fill(0, cliprect);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_tilemap[1]->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x0000, cliprect);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_tilemap[2]->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x4000, cliprect);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_tilemap[3]->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0x8000, cliprect);

	m_tmp_tilemap_renderbitmap.fill(0, cliprect);
	m_tilemap[0]->draw(screen, m_tmp_tilemap_renderbitmap, cliprect, 0,0);
	tilemap_copy_to_compose(0xc000, cliprect);

	do_final_mix(bitmap, cliprect);

	// prepare sprites for NEXT frame - causes 1 frame palette errors, but prevents sprite lag in tkdensho, which is correct?
	render_sprites_to_bitmap(bitmap, m_880000regs[0x0], m_880000regs[0x1]);

	return 0;
}

void tecmosys_state::video_start()
{
	m_screen->register_screen_bitmap(m_sprite_bitmap);
	m_sprite_bitmap.fill(0x4000);

	m_screen->register_screen_bitmap(m_tmp_tilemap_composebitmap);
	m_screen->register_screen_bitmap(m_tmp_tilemap_renderbitmap);
	m_tmp_tilemap_composebitmap.fill(0x0000);
	m_tmp_tilemap_renderbitmap.fill(0x0000);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmosys_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8, 32*2,32*2);
	m_tilemap[0]->set_transparent_pen(0);

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmosys_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmosys_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_tilemap[2]->set_transparent_pen(0);

	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmosys_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16,16, 32,32);
	m_tilemap[3]->set_transparent_pen(0);

	save_item(NAME(m_spritelist));
}
