// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

 tecmosys video driver

***************************************************************************/

#include "emu.h"
#include "tecmosys.h"


template<int Layer>
TILE_GET_INFO_MEMBER(tecmosys_state::get_tile_info)
{
	tileinfo.set(Layer,
			m_vram[Layer][2*tile_index+1],
			(m_vram[Layer][2*tile_index]&0x3f),
			TILE_FLIPYX((m_vram[Layer][2*tile_index]&0xc0)>>6));
}


inline void tecmosys_state::set_color_555(pen_t color, int rshift, int gshift, int bshift, u16 data)
{
	m_palette->set_pen_color(color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}

void tecmosys_state::tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tilemap_paletteram16[offset]);
	set_color_555(offset+0x4000, 5, 10, 0, m_tilemap_paletteram16[offset]);
}

void tecmosys_state::render_sprites_to_bitmap(const rectangle &cliprect, u16 extrax, u16 extray )
{
	const rectangle scaled_cliprect((cliprect.min_x << 8), ((cliprect.max_x + 1) << 8), (cliprect.min_y << 8), ((cliprect.max_y + 1) << 8));

	/* render sprites (with priority information) to temp bitmap */
	m_sprite_bitmap.fill(0x0000, cliprect);
	/* there are multiple spritelists in here, to allow for buffering */
	for (int i = (m_spritelist * 0x4000) / 2; i < ((m_spritelist + 1) * 0x4000) / 2; i += 8)
	{
		/*
		    sprite format (16 bytes per each sprite)
		        fedcba98 76543210
		    00  ------xx xxxxxxxx X position (10 bit signed)
		    02  -------x xxxxxxxx Y position (9 bit signed)
		    04  ----iiii ffffffff Zoom X (add destination X position per each X counter, 4.8(8.8?) fixed point)*
		    06  ----iiii ffffffff Zoom Y (add destination Y position per each Y counter, 4.8(8.8?) fixed point)*
		    08  x------- -------- Disable this sprite
		        --xxxxxx -------- Palette select (256 color each)
		        -------- x------- Flip Y
		        -------- -x------ Flip X
		        -------- --xx---- Priority
		        -------- ----xxxx ROM offset MSB
		    0a  xxxxxxxx xxxxxxxx ROM offset LSB (256 byte each)
		    0c  xxxxxxxx -------- Source Width (16 pixel each)
		        -------- xxxxxxxx Source Height (16 pixel each)
		    0e  -------- -------- Unused

		    * : i = integer value, f = fraction value
		*/

		if (m_spriteram[i+4] & 0x8000)
			continue;

		int x = m_spriteram[i+0]+386;
		int y = (m_spriteram[i+1]+1);

		x -= extrax;
		y -= extray;

		y &= 0x1ff;
		x &= 0x3ff;

		if (x & 0x200) x -= 0x400;
		if (y & 0x100) y -= 0x200;

		offs_t address =  m_spriteram[i+5] | ((m_spriteram[i+4] & 0x000f) << 16);

		address <<= 8;

		bool const flipx   =  (m_spriteram[i+4] & 0x0040) >> 6;
		bool const flipy   =  (m_spriteram[i+4] & 0x0080) >> 7; // used by some move effects in tkdensho

		u16 const zoomx    =  (m_spriteram[i+2] & 0x0fff) >> 0; // zoom?
		u16 const zoomy    =  (m_spriteram[i+3] & 0x0fff) >> 0; // zoom?

		u16 const ysize    =  ((m_spriteram[i+6] & 0x00ff)) * 16;
		u16 const xsize    = (((m_spriteram[i+6] & 0xff00) >> 8)) * 16;

		u16 const colour   =  (m_spriteram[i+4] & 0x3f00);

		u16 const priority = ((m_spriteram[i+4] & 0x0030)) << 10;

		if (zoomx == 0x100 && zoomy == 0x100) // non-zoomed
		{
			int drawx_base = x;
			int srcx = 0;
			if (drawx_base < cliprect.min_x)
			{
				const int remains = cliprect.min_x - drawx_base;
				drawx_base += remains;
				srcx += remains;
			}
			if (srcx >= xsize)
				continue;

			if (drawx_base > cliprect.max_x)
				continue;

			int drawy = y;
			int srcy = 0;
			if (drawy < cliprect.min_y)
			{
				const int remains = cliprect.min_y - drawy;
				drawy += remains;
				srcy += remains;
			}
			if (srcy >= ysize)
				continue;

			if (drawy > cliprect.max_y)
				continue;

			for (int ycnt = srcy; (drawy <= cliprect.max_y) && (ycnt < ysize); ycnt++, drawy++)
			{
				int ressy;
				if (flipy) ressy = (ysize - 1) - ycnt;
				else ressy = ycnt;

				const u32 srcoffs = address + (ressy * xsize);
				u16 *const dstptr = &m_sprite_bitmap.pix(drawy);

				for (int drawx = drawx_base, xcnt = srcx; (drawx <= cliprect.max_x) && (xcnt < xsize); xcnt++, drawx++)
				{
					int ressx;
					if (flipx) ressx = (xsize - 1) - xcnt;
					else ressx = xcnt;

					const u8 data = (m_sprite_gfx[(srcoffs + ressx) & m_sprite_gfx_mask]);

					if (data)
						dstptr[drawx] = (data + colour) | priority;
				}
			}
		}
		else if (zoomx > 0 && zoomy > 0) // zoomed
		{
			int drawx_base = x << 8;
			int srcx = 0;
			while (drawx_base < scaled_cliprect.min_x)
			{
				drawx_base += zoomx;
				srcx++;
				if (srcx >= xsize)
					break;
			}
			if (srcx >= xsize)
				continue;

			if (drawx_base >= scaled_cliprect.max_x)
				continue;

			int drawy = y << 8;
			int srcy = 0;
			while (drawy < scaled_cliprect.min_y)
			{
				drawy += zoomy;
				srcy++;
				if (srcy >= ysize)
					break;
			}
			if (srcy >= ysize)
				continue;

			if (drawy >= scaled_cliprect.max_y)
				continue;

			for (int ycnt = srcy; (drawy < scaled_cliprect.max_y) && (ycnt < ysize); ycnt++, drawy += zoomy)
			{
				int ressy;
				if (flipy) ressy = (ysize - 1) - ycnt;
				else ressy = ycnt;

				const u32 srcoffs = address + (ressy * xsize);
				u16 *const dstptr = &m_sprite_bitmap.pix(drawy >> 8);

				for (int drawx = drawx_base, xcnt = srcx; (drawx < scaled_cliprect.max_x) && (xcnt < xsize); xcnt++, drawx += zoomx)
				{
					int ressx;
					if (flipx) ressx = (xsize - 1) - xcnt;
					else ressx = xcnt;

					const u8 data = (m_sprite_gfx[(srcoffs + ressx) & m_sprite_gfx_mask]);

					if (data)
						dstptr[drawx >> 8] = (data + colour) | priority;
				}
			}
		}
	}
}

void tecmosys_state::tilemap_copy_to_compose(u16 pri, const rectangle &cliprect)
{
	for (int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		u16 const *const srcptr = &m_tmp_tilemap_renderbitmap.pix(y);
		u16 *const dstptr = &m_tmp_tilemap_composebitmap.pix(y);
		for (int x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			if ((srcptr[x]&0xf)!=0x0)
				dstptr[x] = (srcptr[x]&0x7ff) | pri;
		}
	}
}


void tecmosys_state::do_final_mix(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	for (int y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		u16 const *const srcptr = &m_tmp_tilemap_composebitmap.pix(y);
		u16 const *const srcptr2 = &m_sprite_bitmap.pix(y);

		u32 *const dstptr = &bitmap.pix(y);
		for (int x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			u16 const pri = srcptr[x] & 0xc000;
			u16 const pri2 = srcptr2[x] & 0xc000;

			u16 const penvalue = m_tilemap_paletteram16[srcptr[x]&0x7ff];
			u32 const colour = paldata[(srcptr[x]&0x7ff) | 0x4000];

			u16 penvalue2;
			u32 colour2;
			u8 mask = 0;
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
					[] (u32 colour, u32 colour2, u32 &dst)
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
					[this] (u32 colour, u32 src, u32 &dst)
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


u32 tecmosys_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	render_sprites_to_bitmap(cliprect, m_880000regs[0x0], m_880000regs[0x1]);

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

	m_spritelist = 0;
	save_item(NAME(m_spritelist));
}
