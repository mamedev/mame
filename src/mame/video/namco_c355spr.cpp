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

*/

#include "emu.h"
#include "namco_c355spr.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device, "namco_c355spr", "Namco 186/187 or C355 (Sprites)")
DEFINE_DEVICE_TYPE(DECO_ZOOMSPR, deco_zoomspr_device, "deco_zoomspr", "Namco 186/187 or C355 (Sprites) (Dragon Gun)")

namco_c355spr_device::namco_c355spr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gfx_interface(mconfig, *this, nullptr),
	device_video_interface(mconfig, *this),
	m_palxor(0),
	m_buffer(0),
	m_external_prifill(false),
	m_gfx_region(*this, DEVICE_SELF),
	m_colbase(0),
	m_colors(16),
	m_granularity(256)
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

void namco_c355spr_device::zdrawgfxzoom(
		bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		u32 code,u32 color,bool flipx,bool flipy,int hpos,int vpos,
		int hsize, int vsize, u8 prival)
{
	if (!hsize || !vsize) return;
	if (dest_bmp.bpp() == 16)
	{
		if (gfx)
		{
			const u32 pal = gfx->colorbase() + gfx->granularity() * (color % gfx->colors());
			const u8 *source_base = gfx->get_data(code % gfx->elements());
			int sprite_screen_height = (vsize * gfx->height() + 0x8000) >> 16;
			int sprite_screen_width = (hsize * gfx->width() + 0x8000) >> 16;
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
						u8 const *const source = source_base + (y_index>>16) * gfx->rowbytes();
						u16 *const dest = &dest_bmp.pix(y);
						int x_index = x_index_base;
						for (int x = hpos; x < ex; x++)
						{
							const u8 c = source[x_index>>16];
							if (c != 0xff)
							{
								dest[x] = ((prival & 0xf) << 12) | ((pal + c) & 0xfff);
							}
							x_index += dx;
						}
						y_index += dy;
					}
				}
			}
		}
	}
} /* zdrawgfxzoom */

void namco_c355spr_device::copybitmap(bitmap_ind16 &dest_bmp, const rectangle &clip, u8 pri)
{
	if (m_palxor)
	{
		for (int y = clip.min_y; y <= clip.max_y; y++)
		{
			u16 *const src = &m_tempbitmap.pix(y);
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
			u16 *const src = &m_tempbitmap.pix(y);
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
			u16 *const src = &m_tempbitmap.pix(y);
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
			u16 *const src = &m_tempbitmap.pix(y);
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
	screen().register_screen_bitmap(m_tempbitmap);
	screen().register_screen_bitmap(m_screenbitmap);
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
		m_spritelist[i] = std::make_unique<c355_sprite []>(0x100);
		m_sprite_end[i] = m_spritelist[i].get();
		m_spriteram[i] = std::make_unique<u16 []>(0x20000/2);
		std::fill_n(m_spriteram[i].get(), 0x20000/2, 0);
		save_pointer(NAME(m_spriteram[i]), 0x20000/2, i);
	}
	set_gfx(0, std::make_unique<gfx_element>(&palette(), obj_layout, m_gfx_region->base(), 0, m_colors, m_colbase));
	gfx(0)->set_granularity(m_granularity);

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
void namco_c355spr_device::draw_sprites(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int pri)
{
	if (pri == 0)
	{
		if (!m_external_prifill)
		{
			if (m_buffer == 0) // not buffered sprites
				get_sprites(cliprect);
		}
	}
	copybitmap(bitmap, cliprect, pri);
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites(screen, bitmap, cliprect, pri);
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites(screen, bitmap, cliprect, pri);
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
			get_sprites(screen().visible_area());

		if (m_buffer > 1)
			std::copy_n(m_spriteram[0].get(), 0x20000/2, m_spriteram[1].get());
	}
}



/* the sprites used dy DragonGun + Lock 'n' Loaded */

// helicopter in lockload is only half drawn? (3rd attract demo)
// how are we meant to mix these with the tilemaps, parts must go above / behind parts of tilemap '4/7' in lockload, could be more priority masking sprites we're missing / not drawing tho?
// dragongun also does a masking trick on the dragon during the attract intro, it should not be visible but rather cause the fire to be invisible in the shape of the dragon (see note / hack in code to enable this effect)
// dragongun 'waterfall' prior to one of the bosses also needs correct priority



deco_zoomspr_device::deco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namco_c355spr_device(mconfig, DECO_ZOOMSPR, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

/******************************************************************************/


inline void deco_zoomspr_device::dragngun_drawgfxzoom(
	bitmap_rgb32& dest_bmp, const rectangle& clip, gfx_element* gfx,
	uint32_t code, uint32_t color, int flipx, int flipy, int hpos, int vpos,
	int transparent_color,
	int hsize, int vsize, bitmap_ind8* pri_buffer, uint32_t pri_mask, int sprite_screen_width, int  sprite_screen_height,
	bitmap_ind8& pri_bitmap, bitmap_rgb32& temp_bitmap,
	int priority)
{
	rectangle myclip;

	if (!hsize || !vsize) return;

	/*
	hsize and vsize are 16.16 fixed point numbers
	1<<15 : shrink to 50%
	1<<16 : uniform scale
	1<<17 : double to 200%
	*/

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= temp_bitmap.cliprect();

	{
		if (gfx)
		{
			const pen_t* pal = &gfx->palette().pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const uint8_t* code_base = gfx->get_data(code % gfx->elements());

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

				if (hpos < clip.left())
				{ /* clip left */
					int pixels = clip.left() - hpos;
					hpos += pixels;
					x_index_base += pixels * dx;
				}
				if (vpos < clip.top())
				{ /* clip top */
					int pixels = clip.top() - vpos;
					vpos += pixels;
					y_index += pixels * dy;
				}
				/* NS 980211 - fixed incorrect clipping */
				if (ex > clip.right() + 1)
				{ /* clip right */
					int pixels = ex - clip.right() - 1;
					ex -= pixels;
				}
				if (ey > clip.bottom() + 1)
				{ /* clip bottom */
					int pixels = ey - clip.bottom() - 1;
					ey -= pixels;
				}

				if (ex > hpos)
				{ /* skip if inner loop doesn't draw anything */


					for (int y = vpos; y < ey; y++)
					{
						uint8_t const* const source = code_base + (y_index >> 16) * gfx->rowbytes();
						uint32_t* const dest = &temp_bitmap.pix(y);
						uint8_t* const pri = &pri_bitmap.pix(y);


						int x_index = x_index_base;
						for (int x = hpos; x < ex; x++)
						{
							int c = source[x_index >> 16];
							if (c != transparent_color)
							{
								if (priority >= pri[x])
								{
									dest[x] = pal[c];
									dest[x] |= 0xff000000;
								}
								else // sprites can have a 'masking' effect on other sprites
								{
									dest[x] = 0x00000000;
								}
							}

							x_index += dx;
						}

						y_index += dy;
					}
				}
			}
		}
	}
}


/**
 * 0x00000 sprite attr (page0)
 * 0x02000 sprite list (page0)
 *
 * 0x02400 window attributes
 * 0x04000 format
 * 0x08000 tile
 * 0x10000 sprite attr (page1)
 * 0x14000 sprite list (page1)
 */
void namco_c355spr_device::get_single_sprite(const u16 *spritedata, c355_sprite *sprite_ptr)
{
	u16 *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	const u16 *spriteformat = &spriteram16[0x4000 / 2];
	const u16 *spritetile   = &spriteram16[0x8000 / 2];
	rectangle clip;

	/**
	 * ----xxxx-------- window select
	 * --------xxxx---- priority
	 * ------------xxxx palette select
	 */
	const u16 palette = spritedata[6];
	sprite_ptr->pri = ((palette >> 4) & 0xf);

	const u16 spriteformatram_offset = spritedata[0] & 0x7ff; /* LINKNO     0x000..0x7ff for format table entries - finalapr code masks with 0x3ff, but vshoot requires 0x7ff */
	sprite_ptr->offset = spritedata[1];         /* OFFSET */
	int hpos           = spritedata[2];         /* HPOS       0x000..0x7ff (signed) */
	int vpos           = spritedata[3];         /* VPOS       0x000..0x7ff (signed) */
	u16 hsize          = spritedata[4];         /* HSIZE      max 0x3ff pixels */
	u16 vsize          = spritedata[5];         /* VSIZE      max 0x3ff pixels */
	/* spritedata[6] contains priority/palette */
	/* spritedata[7] is used in Lucky & Wild, possibly for sprite-road priority */

	int xscroll = (s16)m_position[1];
	int yscroll = (s16)m_position[0];

//  xscroll &= 0x3ff; if (xscroll & 0x200) xscroll |= ~0x3ff;
	xscroll &= 0x1ff; if (xscroll & 0x100) xscroll |= ~0x1ff;
	yscroll &= 0x1ff; if (yscroll & 0x100) yscroll |= ~0x1ff;

	if (screen().height() > 384)
	{ /* Medium Resolution: vposstem21 adjust */
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
	const u16 *pWinAttr = &spriteram16[0x2400 / 2 + ((palette >> 8) & 0xf) * 4];
	clip.set(pWinAttr[0] - xscroll, pWinAttr[1] - xscroll, pWinAttr[2] - yscroll, pWinAttr[3] - yscroll);
	sprite_ptr->clip = clip;
	hpos &= 0x7ff; if (hpos & 0x400) hpos |= ~0x7ff; /* sign extend */
	vpos &= 0x7ff; if (vpos & 0x400) vpos |= ~0x7ff; /* sign extend */

	int tile_index   = spriteformat[(spriteformatram_offset << 2) + 0];
	const u16 format = spriteformat[(spriteformatram_offset << 2) + 1];
	int dx           = spriteformat[(spriteformatram_offset << 2) + 2]; // should this also be masked and have a sign bit like dy?
	const int dy     = spriteformat[(spriteformatram_offset << 2) + 3] & 0x1ff;
	int num_cols     = (format >> 4) & 0xf;
	int num_rows     = (format) & 0xf;

	if (num_cols == 0) num_cols = 0x10;
	const bool flipx = (hsize & 0x8000);
	hsize &= 0x3ff;//0x1ff;
	if (hsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	u32 zoomx = (hsize << 16) / (num_cols * 16);
	dx = (dx * zoomx + 0x8000) >> 16;
	if (flipx)
	{
		hpos += dx;
	}
	else
	{
		hpos -= dx;
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
	sprite_ptr->color = (palette & 0xf) ^ m_palxor;

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
			const u16 tile = spritetile[tile_index++];
			sprite_ptr->tile[ind] = tile;
			if ((tile & 0x8000) == 0)
			{
				sprite_ptr->x[ind] = x;
				sprite_ptr->y[ind] = y;
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


int namco_c355spr_device::default_code2tile(int code)
{
	return code;
}

void namco_c355spr_device::get_list(int no, const u16 *pSpriteList16, const u16 *pSpriteTable)
{
	/* draw the sprites */
	c355_sprite *sprite_ptr = m_spritelist[no].get();
	for (int i = 0; i < 256; i++)
	{
		sprite_ptr->disable = false;
		const u16 which = pSpriteList16[i];
		get_single_sprite(&pSpriteTable[(which & 0xff) * 8], sprite_ptr);
		sprite_ptr++;
		if (which & 0x100) break;
	}
	m_sprite_end[no] = sprite_ptr;
}

void namco_c355spr_device::get_sprites(const rectangle cliprect)
{
	int buffer = std::max(0, m_buffer - 1);
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */

//  if (offs == 0)  // boot
	// TODO: solvalou service mode wants 0x14000/2 & 0x00000/2
	// drawing this is what causes the bad tile in vshoot, do any games need 2 lists at the same time or should 1 list be configurable?
		get_list(0, &m_spriteram[buffer][0x02000/2], &m_spriteram[buffer][0x00000/2]);
//  else
		get_list(1, &m_spriteram[buffer][0x14000/2], &m_spriteram[buffer][0x10000/2]);

	copy_sprites(cliprect);
}

void namco_c355spr_device::copy_sprites(const rectangle cliprect)
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	m_tempbitmap.fill(0xffff, cliprect);
	for (int no = 0; no < 2; no++)
	{
		//if (offs == no)
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
							zdrawgfxzoom(
								m_tempbitmap,
								clip,
								gfx(0),
								m_code2tile(sprite_ptr->tile[ind]) + sprite_ptr->offset,
								sprite_ptr->color,
								sprite_ptr->flipx, sprite_ptr->flipy,
								sprite_ptr->x[ind], sprite_ptr->y[ind],
								sprite_ptr->zoomx[ind], sprite_ptr->zoomy[ind], sprite_ptr->pri);
						}
					}
				}
				i++;
				sprite_ptr++;
			}
		}
	}
}


void deco_zoomspr_device::dragngun_draw_sprites(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect, const uint32_t* spritedata, uint32_t* spriteformat_ram0, uint32_t* spriteformat_ram1, uint32_t* spritetile_ram0, uint32_t* spritetile_ram1, bitmap_ind8& pri_bitmap, bitmap_rgb32& temp_bitmap)
{
	const uint32_t* spriteformat;
	const uint32_t* spritetile;
	int offs;
	temp_bitmap.fill(0x00000000, cliprect);

	/*
		Sprites are built from main control ram, which references tile
		layout ram, which finally references tile lookup ram which holds
		the actual tile indices to draw and index into the banking
		control.  Tile lookup and tile layout ram are double buffered.


		Main sprite control ram, 8 * 32 bit words per sprite, so

		Word 0:
			0x0400 - Banking control for tile layout RAM + tile lookup ram
			0x0200 - ?
			0x01ff - Index into tile layout RAM
		Word 1 :
		Word 2 : X base position
		Word 3 : Y base position
		Word 4 :
			0x8000: X flip
			0x03ff: X size of block in pixels (for scaling)
		Word 5 :
			0x8000: Y flip
			0x03ff: Y size of block in pixels (for scaling)
		Word 6 :
			0x0000001f - colour.
			0x00000020 - ?  Used for background at 'frog' boss and title screen dragon.
			0x00000040 - ?  priority?
			0x00000080 - flicker
			0x40000000 - Additive/Subtractable blend? (dragngun)
		Word 7 :


		Tile layout ram, 4 * 32 bit words per sprite, so

		Word 0:
			0x2000 - Selector for tile lookup bank!?!?!?!?!?!?
			0x1fff - Index into tile lookup ram (16 bit word based, NOT 32)
		Word 1:
			0xff00 - ?
			0x00f0 - Width
			0x000f - Height
		Word 2:
			0x01ff - X block offset
		Word 3:
			0x01ff - Y block offset
	*/

	/* Sprite global disable bit - can't be, it's set in lockload calibration menu where the targets are sprites */
//  if (dragngun_sprite_ctrl&0x40000000)
//      return;

	for (offs = 0; offs < 0x800; offs += 8)
	{
		if ((spritedata[offs + 6] & 0x80) && (screen.frame_number() & 1)) // flicker
			continue;

		int hpos, vpos, colour, fx, fy, w, h, x, y, dx, dy, hsize, vsize;
		int zoomx, zoomy;
		int xpos, ypos;

		hsize = spritedata[offs + 4] & 0x3ff;
		vsize = spritedata[offs + 5] & 0x3ff;
		if (!hsize || !vsize) /* Zero pixel size in X or Y - skip block */
			continue;

		int spriteformatram_offset = (spritedata[offs + 0] & 0x1ff);

		if (spritedata[offs + 0] & 0x400)
			spriteformat = spriteformat_ram1;
		else
			spriteformat = spriteformat_ram0;

		h = (spriteformat[(spriteformatram_offset << 2) + 1] >> 0) & 0xf;
		w = (spriteformat[(spriteformatram_offset << 2) + 1] >> 4) & 0xf;
		if (!h || !w)
			continue;
		
		hpos = spritedata[offs + 2] & 0x3ff;
		vpos = spritedata[offs + 3] & 0x3ff;
		dx = spriteformat[(spriteformatram_offset << 2) + 2] & 0x1ff;
		dy = spriteformat[(spriteformatram_offset << 2) + 3] & 0x1ff;
		if (dx & 0x100) dx = 1 - (dx & 0xff);
		if (dy & 0x100) dy = 1 - (dy & 0xff); /* '1 - ' is strange, but correct for Dragongun 'Winners' screen. */
		if (hpos >= 512) hpos -= 1024;
		if (vpos >= 512) vpos -= 1024;

		colour = spritedata[offs + 6] & 0x1f;

		int priority = (spritedata[offs + 6] & 0x60) >> 5;

		if (priority == 0) priority = 7;
		else if (priority == 1) priority = 7; // set to 1 to have the 'masking effect' with the dragon on the dragngun attract mode, but that breaks the player select where it needs to be 3, probably missing some bits..
		else if (priority == 2) priority = 7;
		else if (priority == 3) priority = 7;

		fx = spritedata[offs + 4] & 0x8000;
		fy = spritedata[offs + 5] & 0x8000;

		int lookupram_offset = spriteformat[(spriteformatram_offset << 2) + 0] & 0x1fff;

		if (spriteformat[(spriteformatram_offset << 2) + 0] & 0x2000)
			spritetile = spritetile_ram1;
		else
			spritetile = spritetile_ram0;

		zoomx = hsize * 0x10000 / (w * 16);
		zoomy = vsize * 0x10000 / (h * 16);

		if (!fy)
			ypos = (vpos << 16) - (dy * zoomy); /* The block offset scales with zoom, the base position does not */
		else
			ypos = (vpos << 16) + (dy * zoomy) - (16 * zoomy);

		for (y = 0; y < h; y++)
		{
			if (!fx)
				xpos = (hpos << 16) - (dx * zoomx); /* The block offset scales with zoom, the base position does not */
			else
				xpos = (hpos << 16) + (dx * zoomx) - (16 * zoomx);

			for (x = 0; x < w; x++)
			{
				int sprite = spritetile[lookupram_offset] & 0x3fff;

				lookupram_offset++;

				dragngun_drawgfxzoom(
					bitmap, cliprect, gfx(0),
					m_code2tile(sprite),
					colour,
					fx, fy,
					xpos >> 16, ypos >> 16,
					15, zoomx, zoomy, nullptr, 0,
					((xpos + (zoomx << 4)) >> 16) - (xpos >> 16), ((ypos + (zoomy << 4)) >> 16) - (ypos >> 16),
					pri_bitmap, temp_bitmap,
					priority
				);


				if (fx)
					xpos -= zoomx << 4;
				else
					xpos += zoomx << 4;
			}
			if (fy)
				ypos -= zoomy << 4;
			else
				ypos += zoomy << 4;
		}
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t const* const src = &temp_bitmap.pix(y);
		uint32_t* const dst = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			uint32_t const srcpix = src[x];

			if ((srcpix & 0xff000000) == 0xff000000)
			{
				dst[x] = srcpix & 0x00ffffff;
			}
		}
	}
}
