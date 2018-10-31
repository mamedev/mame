// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
    C355 Zooming sprites
    used by
    namcofl.cpp (all games)
    namconb1.cpp (all games)
    gal3.cpp (all games)
    namcos21.cpp (Driver's Eyes, Solvalou, Starblade, Air Combat, Cyber Sled) (everything except Winning Run series)
    namcos2.cpp (Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours, Suzuka 8 Hours 2)

*/

#include "emu.h"
#include "namco_c355spr.h"

DEFINE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device, "namco_c355spr", "Namco C355 (Sprites)")

namco_c355spr_device::namco_c355spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCO_C355SPR, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_gfx_region(0),
	m_palxor(0),
	m_scrolloffs{ 0, 0 },
	m_buffer(0),
	m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}


/**************************************************************************************/

void namco_c355spr_device::zdrawgfxzoom(
		screen_device &screen,
		bitmap_ind16 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	if (!scalex || !scaley) return;
	if (dest_bmp.bpp() == 16)
	{
		if (gfx)
		{
			device_palette_interface &palette = m_gfxdecode->palette();
			int shadow_offset = (palette.shadows_enabled())?palette.entries():0;
			const pen_t *pal = &palette.pen(gfx->colorbase() + gfx->granularity() * (color % gfx->colors()));
			const uint8_t *source_base = gfx->get_data(code % gfx->elements());
			int sprite_screen_height = (scaley*gfx->height()+0x8000)>>16;
			int sprite_screen_width = (scalex*gfx->width()+0x8000)>>16;
			if (sprite_screen_width && sprite_screen_height)
			{
				/* compute sprite increment per screen pixel */
				int dx = (gfx->width()<<16)/sprite_screen_width;
				int dy = (gfx->height()<<16)/sprite_screen_height;

				int ex = sx+sprite_screen_width;
				int ey = sy+sprite_screen_height;

				int x_index_base;
				int y_index;

				if (flipx)
				{
					x_index_base = (sprite_screen_width-1)*dx;
					dx = -dx;
				}
				else
				{
					x_index_base = 0;
				}

				if (flipy)
				{
					y_index = (sprite_screen_height-1)*dy;
					dy = -dy;
				}
				else
				{
					y_index = 0;
				}

				if (sx < clip.min_x)
				{ /* clip left */
					int pixels = clip.min_x-sx;
					sx += pixels;
					x_index_base += pixels*dx;
				}
				if (sy < clip.min_y)
				{ /* clip top */
					int pixels = clip.min_y-sy;
					sy += pixels;
					y_index += pixels*dy;
				}
				if (ex > clip.max_x+1)
				{ /* clip right */
					int pixels = ex-clip.max_x-1;
					ex -= pixels;
				}
				if (ey > clip.max_y+1)
				{ /* clip bottom */
					int pixels = ey-clip.max_y-1;
					ey -= pixels;
				}

				if (ex>sx)
				{ /* skip if inner loop doesn't draw anything */
					int y;
					bitmap_ind8 &priority_bitmap = screen.priority();
					if (priority_bitmap.valid())
					{
						for (y = sy; y < ey; y++)
						{
							const uint8_t *source = source_base + (y_index>>16) * gfx->rowbytes();
							uint16_t *dest = &dest_bmp.pix16(y);
							uint8_t *pri = &priority_bitmap.pix8(y);
							int x, x_index = x_index_base;
							if (m_palxor)
							{
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index>>16];
									if (c != 0xff)
									{
										if (pri[x] <= zpos)
										{
											switch (c)
											{
											case 0:
												dest[x] = 0x4000|(dest[x]&0x1fff);
												break;
											case 1:
												dest[x] = 0x6000|(dest[x]&0x1fff);
												break;
											default:
												dest[x] = pal[c];
												break;
											}
											pri[x] = zpos;
										}
									}
									x_index += dx;
								}
								y_index += dy;
							}
							else
							{
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index>>16];
									if (c != 0xff)
									{
										if (pri[x] <= zpos )
										{
											if (color == 0xf && c == 0xfe && shadow_offset)
											{
												dest[x] |= shadow_offset;
											}
											else
											{
												dest[x] = pal[c];
											}
											pri[x] = zpos;
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
	}
} /* zdrawgfxzoom */

void namco_c355spr_device::zdrawgfxzoom(
		screen_device &screen,
		bitmap_rgb32 &dest_bmp,const rectangle &clip,gfx_element *gfx,
		uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,
		int scalex, int scaley, int zpos )
{
	/* nop */
}

void namco_c355spr_device::device_start()
{
	std::fill(std::begin(m_position), std::end(m_position), 0x0000);
	for (int i = 0; i < 2; i++)
	{
		m_spritelist[i] = std::make_unique<c355_sprite []>(0x100);
		m_sprite_end[i] = m_spritelist[i].get();
		m_spriteram[i] = std::make_unique<uint16_t []>(0x20000/2);
		std::fill_n(m_spriteram[i].get(), 0x20000/2, 0);
		save_pointer(NAME(m_spriteram[i]), 0x20000/2, i);
	}

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

WRITE16_MEMBER( namco_c355spr_device::position_w )
{
	COMBINE_DATA(&m_position[offset]);
}
READ16_MEMBER( namco_c355spr_device::position_r )
{
	return m_position[offset];
}

/**************************************************************************************************************/

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
void namco_c355spr_device::get_single_sprite(const uint16_t *pSource, c355_sprite *sprite_ptr)
{
	uint16_t *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	unsigned screen_height_remaining, screen_width_remaining;
	unsigned source_height_remaining, source_width_remaining;
	int hpos,vpos;
	uint16_t hsize,vsize;
	uint16_t palette;
	uint16_t linkno;
	uint16_t format;
	int tile_index;
	int num_cols,num_rows;
	int dx,dy;
	int row,col;
	int sx,sy,tile;
	int flipx,flipy;
	uint32_t zoomx, zoomy;
	int tile_screen_width;
	int tile_screen_height;
	const uint16_t *spriteformat16 = &spriteram16[0x4000/2];
	const uint16_t *spritetile16   = &spriteram16[0x8000/2];
	const uint16_t *pWinAttr;
	rectangle clip;
	int xscroll, yscroll;

	/**
	 * ----xxxx-------- window select
	 * --------xxxx---- priority
	 * ------------xxxx palette select
	 */
	palette = pSource[6];
	sprite_ptr->pri = ((palette>>4)&0xf);

	linkno             = pSource[0]; /* LINKNO */
	sprite_ptr->offset = pSource[1]; /* OFFSET */
	hpos               = pSource[2]; /* HPOS       0x000..0x7ff (signed) */
	vpos               = pSource[3]; /* VPOS       0x000..0x7ff (signed) */
	hsize              = pSource[4]; /* HSIZE      max 0x3ff pixels */
	vsize              = pSource[5]; /* VSIZE      max 0x3ff pixels */
	/* pSource[6] contains priority/palette */
	/* pSource[7] is used in Lucky & Wild, possibly for sprite-road priority */

	if (linkno*4>=0x4000/2) /* avoid garbage memory reads */
	{
		sprite_ptr->disable = true;
		return;
	}

	xscroll = (int16_t)m_position[1];
	yscroll = (int16_t)m_position[0];

//  xscroll &= 0x3ff; if (xscroll & 0x200 ) xscroll |= ~0x3ff;
	xscroll &= 0x1ff; if (xscroll & 0x100 ) xscroll |= ~0x1ff;
	yscroll &= 0x1ff; if (yscroll & 0x100 ) yscroll |= ~0x1ff;

	if (screen().height() > 384)
	{ /* Medium Resolution: System21 adjust */
			xscroll = (int16_t)m_position[1];
			xscroll &= 0x3ff; if (xscroll & 0x200) xscroll |= ~0x3ff;
			if (yscroll<0)
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
	pWinAttr = &spriteram16[0x2400/2+((palette>>8)&0xf)*4];
	clip.set(pWinAttr[0] - xscroll, pWinAttr[1] - xscroll, pWinAttr[2] - yscroll, pWinAttr[3] - yscroll);
	sprite_ptr->clip = clip;
	hpos&=0x7ff; if (hpos&0x400 ) hpos |= ~0x7ff; /* sign extend */
	vpos&=0x7ff; if (vpos&0x400 ) vpos |= ~0x7ff; /* sign extend */

	tile_index      = spriteformat16[linkno*4+0];
	format          = spriteformat16[linkno*4+1];
	dx              = spriteformat16[linkno*4+2];
	dy              = spriteformat16[linkno*4+3];
	num_cols        = (format>>4)&0xf;
	num_rows        = (format)&0xf;

	if (num_cols == 0) num_cols = 0x10;
	flipx = (hsize&0x8000)?1:0;
	hsize &= 0x3ff;//0x1ff;
	if (hsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	zoomx = (hsize<<16)/(num_cols*16);
	dx = (dx*zoomx+0x8000)>>16;
	if (flipx)
	{
		hpos += dx;
	}
	else
	{
		hpos -= dx;
	}

	if (num_rows == 0) num_rows = 0x10;
	flipy = (vsize&0x8000)?1:0;
	vsize &= 0x3ff;
	if (vsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	zoomy = (vsize<<16)/(num_rows*16);
	dy = (dy*zoomy+0x8000)>>16;
	if (flipy)
	{
		vpos += dy;
	}
	else
	{
		vpos -= dy;
	}

	sprite_ptr->flipx = flipx;
	sprite_ptr->flipy = flipy;
	sprite_ptr->size = num_rows * num_cols;
	sprite_ptr->color = (palette&0xf)^m_palxor;

	source_height_remaining = num_rows*16;
	screen_height_remaining = vsize;
	sy = vpos;
	int ind = 0;
	for (row = 0; row < num_rows; row++)
	{
		tile_screen_height = 16*screen_height_remaining/source_height_remaining;
		zoomy = (screen_height_remaining<<16)/source_height_remaining;
		if (flipy)
		{
			sy -= tile_screen_height;
		}
		source_width_remaining = num_cols*16;
		screen_width_remaining = hsize;
		sx = hpos;
		for (col = 0; col < num_cols; col++)
		{
			tile_screen_width = 16*screen_width_remaining/source_width_remaining;
			zoomx = (screen_width_remaining<<16)/source_width_remaining;
			if (flipx)
			{
				sx -= tile_screen_width;
			}
			tile = spritetile16[tile_index++];
			sprite_ptr->tile[ind] = tile;
			if ((tile&0x8000)==0)
			{
				sprite_ptr->x[ind] = sx;
				sprite_ptr->y[ind] = sy;
				sprite_ptr->zoomx[ind] = zoomx;
				sprite_ptr->zoomy[ind] = zoomy;
			}
			if (!flipx)
			{
				sx += tile_screen_width;
			}
			screen_width_remaining -= tile_screen_width;
			source_width_remaining -= 16;
			ind++;
		} /* next col */
		if (!flipy)
		{
			sy += tile_screen_height;
		}
		screen_height_remaining -= tile_screen_height;
		source_height_remaining -= 16;
	} /* next row */
}


int namco_c355spr_device::default_code2tile(int code)
{
	return code;
}

void namco_c355spr_device::get_list(int no, const uint16_t *pSpriteList16, const uint16_t *pSpriteTable)
{
	/* draw the sprites */
	c355_sprite *sprite_ptr = m_spritelist[no].get();
	for (int i = 0; i < 256; i++)
	{
		sprite_ptr->disable = false;
		uint16_t which = pSpriteList16[i];
		get_single_sprite(&pSpriteTable[(which&0xff)*8], sprite_ptr);
		sprite_ptr++;
		if (which&0x100) break;
	}
	m_sprite_end[no] = sprite_ptr;
}

void namco_c355spr_device::get_sprites()
{
	int buffer = std::max(0, m_buffer - 1);
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */

//  if (offs == 0)  // boot
	// TODO: solvalou service mode wants 0x14000/2 & 0x00000/2
		get_list(0, &m_spriteram[buffer][0x02000/2], &m_spriteram[buffer][0x00000/2]);
//  else
		get_list(1, &m_spriteram[buffer][0x14000/2], &m_spriteram[buffer][0x10000/2]);
}

template<class BitmapClass>
void namco_c355spr_device::draw_sprites(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int pri)
{
//  int offs = spriteram16[0x18000/2]; /* end-of-sprite-list */
	if (pri == 0)
	{
		screen.priority().fill(0, cliprect);
		if (m_buffer == 0) // not buffered sprites
			get_sprites();
	}

	for (int no = 0; no < 2; no++)
	{
		//if (offs == no)
		{
			int i = 0;
			c355_sprite *sprite_ptr = m_spritelist[no].get();

			while (sprite_ptr != m_sprite_end[no])
			{
				if ((sprite_ptr->pri == pri) && (sprite_ptr->disable == false))
				{
					rectangle clip = sprite_ptr->clip;
					clip &= cliprect;
					for (int ind = 0; ind < sprite_ptr->size; ind++)
					{
						if ((sprite_ptr->tile[ind] & 0x8000) == 0)
						{
							zdrawgfxzoom(
								screen,
								bitmap,
								clip,
								m_gfxdecode->gfx(m_gfx_region),
								m_code2tile(sprite_ptr->tile[ind]) + sprite_ptr->offset,
								sprite_ptr->color,
								sprite_ptr->flipx, sprite_ptr->flipy,
								sprite_ptr->x[ind], sprite_ptr->y[ind],
								sprite_ptr->zoomx[ind], sprite_ptr->zoomy[ind], i);
						}
					}
				}
				i++;
				sprite_ptr++;
			}
		}
	}
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites(screen, bitmap, cliprect, pri);
}

void namco_c355spr_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri)
{
	draw_sprites(screen, bitmap, cliprect, pri);
}

WRITE16_MEMBER( namco_c355spr_device::spriteram_w )
{
	COMBINE_DATA(&m_spriteram[0][offset]);
}

READ16_MEMBER( namco_c355spr_device::spriteram_r )
{
	return m_spriteram[0][offset];
}

WRITE_LINE_MEMBER( namco_c355spr_device::vblank )
{
	if (state)
	{
		if (m_buffer > 0)
			get_sprites();

		if (m_buffer > 1)
			std::copy_n(m_spriteram[0].get(), 0x20000/2, m_spriteram[1].get());
	}
}

