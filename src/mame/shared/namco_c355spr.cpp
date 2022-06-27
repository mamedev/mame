// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Bryan McPhail

/*
    Namco 186/187 Zooming Sprites
    Namco C355 Zooming sprites

    used by
    namcofl.cpp (all games)
    namconb1.cpp (all games)
    gal3.cpp (all games)
    namcos21.cpp (Driver's Eyes, Solvalou, Starblade, Air Combat, Cyber Sled) (everything except Winning Run series)
    namcos2.cpp (Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours, Suzuka 8 Hours 2)
    deco32.cpp (Dragon Gun, Lock 'n' Loaded)

    earlier titles use the 186/187 pair (eg. Steel Gunner, Dragon Gun) while later boards appear to implement the
    same functionality in a single C355 custom (NB1 hardware, Final Lap R etc.)  It is not known if there are any
    differences in capability.

    TODO: verify which boards use which chips


    dragongun does a masking trick on the dragon during the attract intro, it should not be visible but rather
    cause the fire to be invisible in the shape of the dragon
    dragongun 'waterfall' prior to one of the bosses also needs correct priority


    relative to the start of the sprite area these offets are typically used
    it is not clear if this is implemented in a single RAM chip, or multiple on some boards

     * 0x00000 sprite attr (page0) (solvalou service mode)
     * 0x02000 sprite list (page0) (solvalou service mode)
     *
     * 0x02400 window attributes
     * 0x04000 format
     * 0x08000 tile
     * 0x10000 sprite attr (page1)
     * 0x14000 sprite list (page1)

     TODO: solvalou service mode wants lists / attributes at 0x02000/2 & 0x00000/2
     Drawing this is what causes the bad tile in vshoot

     Do any games need 2 lists at the same time or should 1 list be configurable?

     It seems unlikely there are really 2 lists

     Maybe the list written by solvalou's service mode is where data gets copied for
     rendering by the sprite DMA trigger, but solvalou is choosing to write it there
     directly rather than trigger the DMA?

*/

#include "emu.h"
#include "namco_c355spr.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device, "namco_c355spr", "Namco 186/187 or C355 (Sprites)")

namco_c355spr_device::namco_c355spr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gfx_interface(mconfig, *this, nullptr),
	device_video_interface(mconfig, *this),
	m_pri_cb(*this, DEVICE_SELF, FUNC(namco_c355spr_device::default_priority)),
	m_read_spritetile(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritetile)),
	m_read_spriteformat(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spriteformat)),
	m_read_spritetable(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritetable)),
	m_read_cliptable(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_cliptable)),
	m_read_spritelist(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritelist)),
	m_palxor(0),
	m_buffer(0),
	m_external_prifill(false),
	m_gfx_region(*this, DEVICE_SELF),
	m_colbase(0),
	m_colors(16),
	m_granularity(256),
	m_draw_2_lists(true),
	m_device_allocates_spriteram_and_bitmaps(true)
{
	std::fill(std::begin(m_position), std::end(m_position), 0);
	std::fill(std::begin(m_scrolloffs), std::end(m_scrolloffs), 0);
	for (int i = 0; i < 2; i++)
		m_spriteram[i] = nullptr;
}

namco_c355spr_device::namco_c355spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	namco_c355spr_device(mconfig, NAMCO_C355SPR, tag, owner, clock)
{
}

/**************************************************************************************/

template<class BitmapClass>
void namco_c355spr_device::zdrawgfxzoom(
	BitmapClass *dest_bmp, const rectangle &clip, gfx_element *gfx,
	u32 code, u32 color, bool flipx, bool flipy, int hpos, int vpos,
	int hsize, int vsize, u8 prival,

	bitmap_ind8 *pri_buffer,
	int sprite_screen_width, int  sprite_screen_height,
	bitmap_ind8 *pri_bitmap)
{
	if (!hsize || !vsize) return;
	if (!gfx) return;

	const u32 pal = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
	const pen_t *palpen = &gfx->palette().pen(pal);

	const u8 *source_base = gfx->get_data(code % gfx->elements());
	if (sprite_screen_width && sprite_screen_height)
	{
		/* compute sprite increment per screen pixel */
		int dx = (gfx->width() << 16) / sprite_screen_width;
		int dy = (gfx->height() << 16) / sprite_screen_height;

		int ex = hpos + sprite_screen_width;
		int ey = vpos + sprite_screen_height;

		int x_index_base;
		int y_index;

		if (flipx)
		{
			x_index_base = (sprite_screen_width - 1) * dx;
			dx = -dx;
		}
		else
		{
			x_index_base = 0;
		}

		if (flipy)
		{
			y_index = (sprite_screen_height - 1) * dy;
			dy = -dy;
		}
		else
		{
			y_index = 0;
		}

		if (hpos < clip.min_x)
		{ /* clip left */
			int pixels = clip.min_x - hpos;
			hpos += pixels;
			x_index_base += pixels * dx;
		}
		if (vpos < clip.min_y)
		{ /* clip top */
			int pixels = clip.min_y - vpos;
			vpos += pixels;
			y_index += pixels * dy;
		}
		if (ex > clip.max_x + 1)
		{ /* clip right */
			int pixels = ex - clip.max_x - 1;
			ex -= pixels;
		}
		if (ey > clip.max_y + 1)
		{ /* clip bottom */
			int pixels = ey - clip.max_y - 1;
			ey -= pixels;
		}

		if (ex > hpos)
		{ /* skip if inner loop doesn't draw anything */
			for (int y = vpos; y < ey; y++)
			{
				u8 const *const source = source_base + (y_index >> 16) * gfx->rowbytes();
				auto *const dest = &dest_bmp->pix(y);
				int x_index = x_index_base;
				u8 *pri = nullptr;

				if (dest_bmp->bpp() == 32)
					pri = &pri_bitmap->pix(y);

				for (int x = hpos; x < ex; x++)
				{
					if (dest_bmp->bpp() == 16)
					{
						const u8 c = source[x_index >> 16];
						if (c != 0xff)
						{
							dest[x] = ((prival & 0xf) << 12) | ((pal + c) & 0xfff);
						}
						x_index += dx;
					}
					else if (dest_bmp->bpp() == 32)
					{
						int c = source[x_index >> 16];
						if (c != 15)
						{
							if (prival >= pri[x])
							{
								dest[x] = palpen[c];
								dest[x] |= 0xff000000;
							}
							else // sprites can have a 'masking' effect on other sprites
							{
								dest[x] = 0x00000000;
							}
						}

						x_index += dx;
					}
				}
				y_index += dy;
			}
		}
	}
}

void namco_c355spr_device::copybitmap(bitmap_ind16 &dest_bmp, const rectangle &clip, u8 pri)
{
	if (m_palxor)
	{
		for (int y = clip.min_y; y <= clip.max_y; y++)
		{
			u16 *const src = &m_renderbitmap.pix(y);
			u16 *const dest = &dest_bmp.pix(y);
			for (int x = clip.min_x; x <= clip.max_x; x++)
			{
				if (src[x] != 0xffff)
				{
					u8 srcpri = (src[x] >> 12) & 0xf;
					u16 c = src[x] & 0xfff;
					if (srcpri == pri)
					{
						if ((c & 0xff) != 0xff)
						{
							switch (c & 0xff)
							{
							case 0:
								dest[x] = 0x4000|(dest[x] & 0x1fff);
								break;
							case 1:
								dest[x] = 0x6000|(dest[x] & 0x1fff);
								break;
							default:
								dest[x] = gfx(0)->colorbase() + c;
								break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (int y = clip.min_y; y <= clip.max_y; y++)
		{
			u16 *const src = &m_renderbitmap.pix(y);
			u16 *const dest = &dest_bmp.pix(y);
			for (int x = clip.min_x; x <= clip.max_x; x++)
			{
				if (src[x] != 0xffff)
				{
					u8 srcpri = (src[x] >> 12) & 0xf;
					u16 c = src[x] & 0xfff;
					if (srcpri == pri)
					{
						if ((c & 0xff) != 0xff)
						{
							if (c == 0xffe)
							{
								dest[x] |= 0x800;
							}
							else
							{
								dest[x] = gfx(0)->colorbase() + c;
							}
						}
					}
				}
			}
		}
	}
}

void namco_c355spr_device::copybitmap(bitmap_rgb32 &dest_bmp, const rectangle &clip, u8 pri)
{
	device_palette_interface &palette = gfx(0)->palette();
	const pen_t *pal = &palette.pen(gfx(0)->colorbase());
	if (m_palxor)
	{
		for (int y = clip.min_y; y <= clip.max_y; y++)
		{
			u16 *const src = &m_renderbitmap.pix(y);
			u16 *const srcrender = &m_screenbitmap.pix(y);
			u32 *const dest = &dest_bmp.pix(y);
			for (int x = clip.min_x; x <= clip.max_x; x++)
			{
				if (src[x] != 0xffff)
				{
					u8 srcpri = (src[x] >> 12) & 0xf;
					u16 c = src[x] & 0xfff;
					if (srcpri == pri)
					{
						if ((c & 0xff) != 0xff)
						{
							switch (c & 0xff)
							{
							case 0:
								srcrender[x] = 0x4000|(srcrender[x] & 0x1fff);
								break;
							case 1:
								srcrender[x] = 0x6000|(srcrender[x] & 0x1fff);
								break;
							default:
								srcrender[x] = c;
								break;
							}
							dest[x] = pal[srcrender[x]];
						}
					}
				}
				else if (srcrender[x] != 0xffff)
					dest[x] = pal[srcrender[x]];
			}
		}
	}
	else
	{
		for (int y = clip.min_y; y <= clip.max_y; y++)
		{
			u16 *const src = &m_renderbitmap.pix(y);
			u16 *const srcrender = &m_screenbitmap.pix(y);
			u32 *const dest = &dest_bmp.pix(y);
			for (int x = clip.min_x; x <= clip.max_x; x++)
			{
				if (src[x] != 0xffff)
				{
					u8 srcpri = (src[x] >> 12) & 0xf;
					u16 c = src[x] & 0xfff;
					if (srcpri == pri)
					{
						if ((c & 0xff) != 0xff)
						{
							if (c == 0xffe)
							{
								srcrender[x] |= 0x800;
							}
							else
							{
								srcrender[x] = c;
							}
							dest[x] = pal[srcrender[x]];
						}
					}
				}
				else if (srcrender[x] != 0xffff)
					dest[x] = pal[srcrender[x]];
			}
		}
	}
}

void namco_c355spr_device::device_start()
{
	m_pri_cb.resolve();

	gfx_layout obj_layout =
	{
		16,16,
		0,
		8, /* bits per pixel */
		{ STEP8(0,1) },
		{ STEP16(0,8) },
		{ STEP16(0,8*16) },
		16*16*8
	};
	obj_layout.total = m_gfx_region->bytes() / (16*16*8 / 8);

	std::fill(std::begin(m_position), std::end(m_position), 0x0000);

	for (int i = 0; i < 2; i++)
	{
		m_spritelist[i] = std::make_unique<c355_sprite[]>(0x100);
		m_sprite_end[i] = m_spritelist[i].get();
	}

	if (m_device_allocates_spriteram_and_bitmaps)
	{
		for (int i = 0; i < 2; i++)
		{
			m_spriteram[i] = std::make_unique<u16[]>(0x20000 / 2);
			std::fill_n(m_spriteram[i].get(), 0x20000 / 2, 0);
			save_pointer(NAME(m_spriteram[i]), 0x20000 / 2, i);
		}

		screen().register_screen_bitmap(m_renderbitmap);
		screen().register_screen_bitmap(m_screenbitmap);
	}

	set_gfx(0, std::make_unique<gfx_element>(&palette(), obj_layout, m_gfx_region->base(), 0, m_colors, m_colbase));
	gfx(0)->set_granularity(m_granularity);

	m_read_spritetile.resolve();
	m_read_spriteformat.resolve();
	m_read_spritetable.resolve();
	m_read_cliptable.resolve();
	m_read_spritelist.resolve();

	save_item(NAME(m_position));

}

void namco_c355spr_device::device_stop()
{
	for (auto &spritelist : m_spritelist)
		spritelist.reset();
	for (auto &spriteram : m_spriteram)
		spriteram.reset();

	device_t::device_stop();
}

/**************************************************************************************/

void namco_c355spr_device::position_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_position[offset]);
}

u16 namco_c355spr_device::position_r(offs_t offset)
{
	return m_position[offset];
}

/**************************************************************************************************************/

template<class BitmapClass>
void namco_c355spr_device::draw_sprites(BitmapClass &bitmap, const rectangle &cliprect, int pri)
{
	if (pri == 0)
	{
		if (!m_external_prifill)
		{
			if (m_buffer == 0) // not buffered sprites
				build_sprite_list_and_render_sprites(cliprect);
		}
	}
	copybitmap(bitmap, cliprect, pri);
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites(bitmap, cliprect, pri);
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites( bitmap, cliprect, pri);
}

void namco_c355spr_device::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[0][offset]);
}

u16 namco_c355spr_device::spriteram_r(offs_t offset)
{
	return m_spriteram[0][offset];
}

WRITE_LINE_MEMBER(namco_c355spr_device::vblank)
{
	if (state)
	{
		if (m_buffer > 0)
			build_sprite_list_and_render_sprites(screen().visible_area());

		if (m_buffer > 1)
			std::copy_n(m_spriteram[0].get(), 0x20000/2, m_spriteram[1].get());
	}
}

/******************************************************************************/

u16 namco_c355spr_device::read_spriteformat(int entry, u8 attr)
{
	u16 *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	const u16 *spriteformat = &spriteram16[0x4000 / 2];
	return spriteformat[(entry << 2) + attr];
}

u16 namco_c355spr_device::read_spritetile(int entry)
{
	u16 *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	const u16 *spritetile = &spriteram16[0x8000 / 2];
	return spritetile[entry];
}

u16 namco_c355spr_device::read_spritetable(int entry, u8 attr, int whichlist)
{
	u16 *ram;
	int buffer = std::max(0, m_buffer - 1);
	if (whichlist == 0)
		ram = &m_spriteram[buffer][0x00000 / 2];
	else
		ram = &m_spriteram[buffer][0x10000 / 2];

	return ram[(entry << 3) + attr];
}

u16 namco_c355spr_device::read_cliptable(int entry, u8 attr)
{
	u16 *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	const u16 *ram = &spriteram16[0x2400 / 2];
	return ram[(entry << 2) + attr];
}

u16 namco_c355spr_device::read_spritelist(int entry, int whichlist)
{
	u16 *ram;
	int buffer = std::max(0, m_buffer - 1);
	if (whichlist == 0)
		ram = &m_spriteram[buffer][0x02000 / 2];
	else
		ram = &m_spriteram[buffer][0x14000 / 2];

	return ram[entry];
}


int namco_c355spr_device::default_code2tile(int code)
{
	return code;
}

void namco_c355spr_device::build_sprite_list(int no)
{
	/* draw the sprites */
	c355_sprite *sprite_ptr = m_spritelist[no].get();
	for (int i = 0; i < 256; i++)
	{
		sprite_ptr->disable = false;
		const u16 which = m_read_spritelist(i, no);
		get_single_sprite(which & 0xff, sprite_ptr, no);
		sprite_ptr++;
		if (which & 0x100) break;
	}
	m_sprite_end[no] = sprite_ptr;
}

void namco_c355spr_device::build_sprite_list_and_render_sprites(const rectangle cliprect)
{
	if (m_draw_2_lists)
		build_sprite_list(0);

	build_sprite_list(1);

	render_sprites(cliprect, nullptr, m_renderbitmap, 0);
}

template<class BitmapClass>
void namco_c355spr_device::render_sprites(const rectangle cliprect, bitmap_ind8 *pri_bitmap, BitmapClass &temp_bitmap, int alt_precision)
{
	temp_bitmap.fill(0xffff, cliprect);
	for (int no = 0; no < 2; no++)
	{
		int i = 0;
		c355_sprite *sprite_ptr = m_spritelist[no].get();

		while (sprite_ptr != m_sprite_end[no])
		{
			if (sprite_ptr->disable == false)
			{
				rectangle clip = sprite_ptr->clip;
				clip &= cliprect;
				for (int ind = 0; ind < sprite_ptr->size; ind++)
				{
					if ((sprite_ptr->tile[ind] & 0x8000) == 0)
					{
						int sprite_screen_height;
						int sprite_screen_width;

						if (alt_precision)
						{
							sprite_screen_width = ((sprite_ptr->x[ind] + (sprite_ptr->zoomx[ind] << 4)) >> 16) - (sprite_ptr->x[ind] >> 16);
							sprite_screen_height = ((sprite_ptr->y[ind] + (sprite_ptr->zoomy[ind] << 4)) >> 16) - (sprite_ptr->y[ind] >> 16);
						}
						else
						{
							sprite_screen_height = (sprite_ptr->zoomy[ind] * 16 + 0x8000) >> 16;
							sprite_screen_width = (sprite_ptr->zoomx[ind] * 16 + 0x8000) >> 16;
						}

						zdrawgfxzoom(
							&temp_bitmap,
							clip,
							gfx(0),
							m_code2tile(sprite_ptr->tile[ind]) + sprite_ptr->offset,
							sprite_ptr->color,
							sprite_ptr->flipx, sprite_ptr->flipy,
							sprite_ptr->x[ind] >> 16, sprite_ptr->y[ind] >> 16,
							sprite_ptr->zoomx[ind], sprite_ptr->zoomy[ind], sprite_ptr->pri,
							nullptr,
							sprite_screen_width, sprite_screen_height,
							pri_bitmap);
					}
				}
			}
			i++;
			sprite_ptr++;
		}
	}
}

void namco_c355spr_device::copybitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u32 const *const src = &temp_bitmap.pix(y);
		u32 *const dst = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u32 const srcpix = src[x];

			if ((srcpix & 0xff000000) == 0xff000000)
			{
				dst[x] = srcpix & 0x00ffffff;
			}
		}
	}
}

void namco_c355spr_device::draw_dg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &pri_bitmap, bitmap_rgb32 &temp_bitmap)
{
	build_sprite_list(0);

	render_sprites(cliprect, &pri_bitmap, temp_bitmap, 1);

	copybitmap(screen, bitmap, cliprect, pri_bitmap, temp_bitmap);
}

void namco_c355spr_device::get_single_sprite(u16 which, c355_sprite *sprite_ptr, int no)
{
	/**
	 * ----xxxx-------- window select
	 * --------xxxx---- priority
	 * ------------xxxx palette select
	 */
	const u16 palette = m_read_spritetable(which, 6, no);

	int priority = m_pri_cb(palette);

	if (priority == -1)
	{
		sprite_ptr->disable = true;
		return;
	}

	sprite_ptr->pri = priority;

	const u16 spriteformatram_offset = m_read_spritetable(which, 0, no) & 0x7ff; /* LINKNO     0x000..0x7ff for format table entries - finalapr code masks with 0x3ff, but vshoot requires 0x7ff */
	sprite_ptr->offset = m_read_spritetable(which, 1, no);         /* OFFSET */
	int hpos           = m_read_spritetable(which, 2, no);         /* HPOS       0x000..0x7ff (signed) */
	int vpos           = m_read_spritetable(which, 3, no);         /* VPOS       0x000..0x7ff (signed) */
	u16 hsize          = m_read_spritetable(which, 4, no);         /* HSIZE      max 0x3ff pixels */
	u16 vsize          = m_read_spritetable(which, 5, no);         /* VSIZE      max 0x3ff pixels */
	/* m_read_spritetable(which, 6, no)   contains priority/palette */
	/* m_read_spritetable(which, 7, no)   is used in Lucky & Wild, possibly for sprite-road priority */

	int xscroll = (s16)m_position[1];
	int yscroll = (s16)m_position[0];

	xscroll &= 0x1ff; if (xscroll & 0x100) xscroll |= ~0x1ff;
	yscroll &= 0x1ff; if (yscroll & 0x100) yscroll |= ~0x1ff;

	if (screen().height() > 384)
	{ /* Medium Resolution: system21 adjust */
			xscroll = (s16)m_position[1];
			xscroll &= 0x3ff; if (xscroll & 0x200) xscroll |= ~0x3ff;
			if (yscroll < 0)
			{ /* solvalou */
				yscroll += 0x20;
			}
			yscroll += 0x10;
	}
	else
	{
		xscroll += m_scrolloffs[0];
		yscroll += m_scrolloffs[1];
	}

	hpos -= xscroll;
	vpos -= yscroll;

	int clipentry = (palette >> 8) & 0xf;
	rectangle clip;
	clip.set(m_read_cliptable(clipentry, 0) - xscroll, m_read_cliptable(clipentry, 1) - xscroll, m_read_cliptable(clipentry, 2) - yscroll, m_read_cliptable(clipentry, 3) - yscroll);
	sprite_ptr->clip = clip;

	hpos &= 0x7ff; if (hpos & 0x400) hpos |= ~0x7ff; /* sign extend */
	vpos &= 0x7ff; if (vpos & 0x400) vpos |= ~0x7ff; /* sign extend */

	int tile_index   = m_read_spriteformat(spriteformatram_offset, 0);
	const u16 format = m_read_spriteformat(spriteformatram_offset, 1);
	const int dx     = m_read_spriteformat(spriteformatram_offset, 2) & 0x1ff;
	const int dy     = m_read_spriteformat(spriteformatram_offset, 3) & 0x1ff;
	int num_cols     = (format >> 4) & 0xf;
	int num_rows     = (format) & 0xf;

	if (num_cols == 0) num_cols = 0x10;
	const bool flipx = (hsize & 0x8000);
	hsize &= 0x3ff;
	if (hsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	u32 zoomx = (hsize << 16) / (num_cols * 16);
	s32 dx_zoomed = ((dx & 0xff) * zoomx + 0x8000) >> 16;
	if (dx & 0x100) dx_zoomed = -dx_zoomed;

	if (!flipx)
	{
		hpos -= dx_zoomed;
	}
	else
	{
		hpos += dx_zoomed;
	}

	if (num_rows == 0) num_rows = 0x10;
	const bool flipy = (vsize & 0x8000);
	vsize &= 0x3ff;
	if (vsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	u32 zoomy = (vsize << 16) / (num_rows * 16);
	s32 dy_zoomed = ((dy & 0xff) * zoomy + 0x8000) >> 16;
	if (dy & 0x100) dy_zoomed = -dy_zoomed;

	if (!flipy)
	{
		vpos -= dy_zoomed;
	}
	else
	{
		vpos += dy_zoomed;
	}

	sprite_ptr->flipx = flipx;
	sprite_ptr->flipy = flipy;
	sprite_ptr->size = num_rows * num_cols;
	sprite_ptr->color = (palette & (m_colors-1)) ^ m_palxor;

	u32 source_height_remaining = num_rows * 16;
	u32 screen_height_remaining = vsize;
	int y = vpos;
	int ind = 0;
	for (int row = 0; row < num_rows; row++)
	{
		int tile_screen_height = 16 * screen_height_remaining / source_height_remaining;
		zoomy = (screen_height_remaining << 16) / source_height_remaining;
		if (flipy)
		{
			y -= tile_screen_height;
		}
		u32 source_width_remaining = num_cols * 16;
		u32 screen_width_remaining = hsize;
		int x = hpos;
		for (int col = 0; col < num_cols; col++)
		{
			int tile_screen_width = 16 * screen_width_remaining / source_width_remaining;
			zoomx = (screen_width_remaining << 16) / source_width_remaining;
			if (flipx)
			{
				x -= tile_screen_width;
			}
			const u16 tile = m_read_spritetile(tile_index++);
			sprite_ptr->tile[ind] = tile;
			if ((tile & 0x8000) == 0)
			{
				sprite_ptr->x[ind] = x << 16;
				sprite_ptr->y[ind] = y << 16;
				sprite_ptr->zoomx[ind] = zoomx;
				sprite_ptr->zoomy[ind] = zoomy;
			}
			if (!flipx)
			{
				x += tile_screen_width;
			}
			screen_width_remaining -= tile_screen_width;
			source_width_remaining -= 16;
			ind++;
		} /* next col */
		if (!flipy)
		{
			y += tile_screen_height;
		}
		screen_height_remaining -= tile_screen_height;
		source_height_remaining -= 16;
	} /* next row */
}

