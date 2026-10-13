// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino, Bryan McPhail

/*

Namco 186/187 Zooming Sprites
Namco C355/187 Zooming sprites

used by:
- namcofl.cpp (all games)
- namconb1.cpp (all games)
- gal3.cpp (all games)
- namcos21*.cpp (Driver's Eyes, Solvalou, Starblade, Air Combat, Cyber Sled) (everything except Winning Run series)
- namcos2.cpp (Steel Gunner, Steel Gunner 2, Lucky & Wild, Suzuka 8 Hours, Suzuka 8 Hours 2)
- deco32.cpp (Dragon Gun, Lock 'n' Loaded)

earlier titles use the 186/187 pair (eg. Steel Gunner, Dragon Gun) while later boards appear to integrate the
186 into the C355 custom (eg. Starblade, NB1 hardware, Final Lap R etc.). It is not known if there are any
differences in capability.

TODO:
- verify which boards use which chips
- dragngun does a masking trick on the dragon during the attract intro, it should not be visible but rather
  cause the fire to be invisible in the shape of the dragon
- dragngun 'waterfall' prior to one of the bosses also needs correct priority
- aircomb ranking screen during attract also seems masking related, or something else? it appears that the
  sprites aren't processed (if they even are in the spritelist)

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

solvalou service mode wants lists / attributes at 0x02000/2 & 0x00000/2.
Do any games need 2 lists at the same time? It seems unlikely there are really 2 lists.

Maybe the list written by solvalou's service mode is where data gets copied for
rendering by the sprite DMA trigger, but solvalou is choosing to write it there
directly rather than trigger the DMA?

Some other games (eg. nebulray, outfxies) deliberately write 0s to page0. If it's
presumed this is the buffered list, it means they want to clear it immediately
without a 1-frame delay?

*/

#include "emu.h"
#include "namco_c355spr.h"

#include <algorithm>

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

GFXDECODE_MEMBER(namco_c355spr_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x8_raw, 0, 16)
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCO_C355SPR, namco_c355spr_device, "namco_c355spr", "Namco 186/187 or C355/187 (sprites)")

namco_c355spr_device::namco_c355spr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_gfx_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_code2tile(*this),
	m_pri_cb(*this, DEVICE_SELF, FUNC(namco_c355spr_device::default_priority)),
	m_mix_cb(*this, DEVICE_SELF, FUNC(namco_c355spr_device::default_mix)),
	m_read_spritetile(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritetile)),
	m_read_spriteformat(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spriteformat)),
	m_read_spritetable(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritetable)),
	m_read_cliptable(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_cliptable)),
	m_read_spritelist(*this, DEVICE_SELF, FUNC(namco_c355spr_device::read_spritelist)),
	m_buffer(0),
	m_external_prifill(false),
	m_transpen(255),
	m_colbase(0),
	m_colors(16),
	m_granularity(256),
	m_device_allocates_spriteram(true)
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

bool namco_c355spr_device::default_mix(u16 &dest, u8 &destpri, u16 colbase, u16 src, int srcpri, int pri)
{
	if ((src & 0xff) != 0xff)
	{
		dest = colbase + src;
		return true;
	}
	return false;
}

void namco_c355spr_device::copybitmap(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, int pri)
{
	for (int y = clip.top(); y <= clip.bottom(); y++)
	{
		u16 *const src = &m_renderbitmap.pix(y);
		u16 *const dest = &dest_bmp.pix(y);
		u8 *const destpri = &screen.priority().pix(y);
		for (int x = clip.left(); x <= clip.right(); x++)
		{
			if (src[x] != 0xffff)
			{
				const u8 srcpri = (src[x] >> 12) & 0xf;
				const u16 c = src[x] & 0xfff;
				m_mix_cb(dest[x], destpri[x], gfx(0)->colorbase(), c, srcpri, pri);
			}
		}
	}
}

void namco_c355spr_device::copybitmap(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, int pri)
{
	device_palette_interface &palette = gfx(0)->palette();
	const pen_t *pal = palette.pens();
	for (int y = clip.top(); y <= clip.bottom(); y++)
	{
		u16 *const src = &m_renderbitmap.pix(y);
		u16 *const srcrender = &m_screenbitmap.pix(y);
		u32 *const dest = &dest_bmp.pix(y);
		u8 *const destpri = &screen.priority().pix(y);
		for (int x = clip.left(); x <= clip.right(); x++)
		{
			if (src[x] != 0xffff)
			{
				const u8 srcpri = (src[x] >> 12) & 0xf;
				const u16 c = src[x] & 0xfff;
				if (m_mix_cb(srcrender[x], destpri[x], gfx(0)->colorbase(), c, srcpri, pri))
					dest[x] = pal[srcrender[x]];
			}
			else if (srcrender[x] != 0xffff)
				dest[x] = pal[gfx(0)->colorbase() + srcrender[x]];
		}
	}
}

void namco_c355spr_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colors(m_colors);
	gfx(0)->set_colorbase(m_colbase);
	gfx(0)->set_granularity(m_granularity);

	m_code2tile.resolve();
	m_pri_cb.resolve();

	screen().register_screen_bitmap(m_renderbitmap);
	screen().register_screen_bitmap(m_screenbitmap);

	std::fill(std::begin(m_position), std::end(m_position), 0x0000);

	m_spritelist = std::make_unique<c355_sprite[]>(0x100);
	m_sprite_end = m_spritelist.get();

	if (m_device_allocates_spriteram)
	{
		for (int i = 0; i < 2; i++)
		{
			m_spriteram[i] = std::make_unique<u16[]>(0x20000 / 2);
			std::fill_n(m_spriteram[i].get(), 0x20000 / 2, 0);
			save_pointer(NAME(m_spriteram[i]), 0x20000 / 2, i);
		}
	}

	m_read_spritetile.resolve();
	m_read_spriteformat.resolve();
	m_read_spritetable.resolve();
	m_read_cliptable.resolve();
	m_read_spritelist.resolve();
	m_mix_cb.resolve();

	save_item(NAME(m_position));
}

void namco_c355spr_device::device_stop()
{
	m_spritelist.reset();
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
				build_sprite_list_and_render_sprites(cliprect);
		}
	}
	copybitmap(screen, bitmap, cliprect, pri);
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

	// update page 0 tables
	if (offset >= 0x10000 / 2 && offset < 0x12000 / 2)
		COMBINE_DATA(&m_spriteram[0][offset - 0x10000 / 2]);

	else if (offset >= 0x14000 / 2 && offset < 0x14400 / 2)
		COMBINE_DATA(&m_spriteram[0][offset - 0x12000 / 2]);
}

u16 namco_c355spr_device::spriteram_r(offs_t offset)
{
	return m_spriteram[0][offset];
}

void namco_c355spr_device::vblank(int state)
{
	if (state)
	{
		if (m_buffer > 0)
			build_sprite_list_and_render_sprites(screen().visible_area());

		if ((m_buffer > 1) && m_device_allocates_spriteram)
			std::copy_n(m_spriteram[0].get(), 0x20000/2, m_spriteram[1].get());
	}
}

/******************************************************************************/

u16 namco_c355spr_device::read_spritetable(int entry, u8 attr)
{
	const int buffer = std::max(0, m_buffer - 1);
	u16 *ram = &m_spriteram[buffer][0x0000 / 2];
	return ram[(entry << 3) + attr];
}

u16 namco_c355spr_device::read_spritelist(int entry)
{
	const int buffer = std::max(0, m_buffer - 1);
	u16 *ram = &m_spriteram[buffer][0x2000 / 2];
	return ram[entry];
}

u16 namco_c355spr_device::read_cliptable(int entry, u8 attr)
{
	u16 *spriteram16 = &m_spriteram[std::max(0, m_buffer - 1)][0];
	const u16 *ram = &spriteram16[0x2400 / 2];
	return ram[(entry << 2) + attr];
}

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


void namco_c355spr_device::build_sprite_list()
{
	/* draw the sprites */
	c355_sprite *sprite_ptr = m_spritelist.get();
	for (int i = 0; i < 256; i++)
	{
		sprite_ptr->disable = false;
		const u16 which = m_read_spritelist(i);
		get_single_sprite(which & 0xff, sprite_ptr);
		sprite_ptr++;
		if (which & 0x100) break;
	}
	m_sprite_end = sprite_ptr;
}

void namco_c355spr_device::build_sprite_list_and_render_sprites(const rectangle cliprect)
{
	build_sprite_list();
	render_sprites(cliprect);
}

void namco_c355spr_device::render_sprites(const rectangle cliprect)
{
	m_renderbitmap.fill(0xffff, cliprect);
	gfx_element *const gfx = this->gfx(0);
	c355_sprite *sprite_ptr = m_spritelist.get();

	while (sprite_ptr != m_sprite_end)
	{
		if (sprite_ptr->disable == false)
		{
			rectangle clip = sprite_ptr->clip;
			clip &= cliprect;

			// the render bitmap holds the priority in the top 4 bits and the pen in
			// the low 12; pal is a multiple of the granularity so adding the pen
			// cannot carry into the priority
			const u32 pal = gfx->colorbase() + gfx->granularity() * (sprite_ptr->color % gfx->colors());
			const u32 color = ((sprite_ptr->pri & 0xf) << 12) | (pal & 0xfff);

			for (int ind = 0; ind < sprite_ptr->cols * sprite_ptr->rows; ind++)
			{
				if ((sprite_ptr->tile[ind] & 0x8000) == 0)
				{
					// TODO: offset is also affected?
					int tile = sprite_ptr->tile[ind];
					if (!m_code2tile.isnull())
						tile = m_code2tile(tile);

					// tiles are stored left to right, top to bottom before flipping
					const int col = ind % sprite_ptr->cols;
					const int row = ind / sprite_ptr->cols;
					gfx->zoom_subpixel_transpen_raw(
						m_renderbitmap,
						clip,
						tile + sprite_ptr->offset,
						color,
						sprite_ptr->flipx, sprite_ptr->flipy,
						sprite_ptr->x, sprite_ptr->y,
						sprite_ptr->flipx ? (sprite_ptr->cols - 1 - col) : col,
						sprite_ptr->flipy ? (sprite_ptr->rows - 1 - row) : row,
						sprite_ptr->stepx, sprite_ptr->stepy,
						m_transpen);
				}
			}
		}
		sprite_ptr++;
	}
}

void namco_c355spr_device::get_single_sprite(u16 which, c355_sprite *sprite_ptr)
{
	/**
	 * ----xxxx-------- window select
	 * --------xxxx---- priority
	 * ------------xxxx palette select
	 */
	u16 palette = m_read_spritetable(which, 6);
	const int priority = m_pri_cb(palette);

	if (priority == -1)
	{
		sprite_ptr->disable = true;
		return;
	}

	sprite_ptr->pri = priority;

	const u16 spriteformatram_offset = m_read_spritetable(which, 0) & 0x7ff; /* LINKNO     0x000..0x7ff for format table entries - finalapr code masks with 0x3ff, but vshoot requires 0x7ff */
	sprite_ptr->offset = m_read_spritetable(which, 1);         /* OFFSET */
	int hpos           = m_read_spritetable(which, 2);         /* HPOS       0x000..0x7ff (signed) */
	int vpos           = m_read_spritetable(which, 3);         /* VPOS       0x000..0x7ff (signed) */
	u16 hsize          = m_read_spritetable(which, 4);         /* HSIZE      max 0x3ff pixels */
	u16 vsize          = m_read_spritetable(which, 5);         /* VSIZE      max 0x3ff pixels */
	/* m_read_spritetable(which, 6)   contains priority/palette (handled above) */
	/* m_read_spritetable(which, 7)   is used in Lucky & Wild, possibly for sprite-road priority */

	int xscroll = util::sext<s16>(m_position[1], 9);
	int yscroll = util::sext<s16>(m_position[0], 9);

	/* Medium Resolution: system21 adjust */
	if (screen().height() > 384)
		xscroll = util::sext<s16>(m_position[1], 10);

	xscroll += m_scrolloffs[0];
	yscroll += m_scrolloffs[1];

	hpos -= xscroll;
	vpos -= yscroll;

	const int clipentry = (palette >> 8) & 0xf;
	rectangle clip;
	clip.set(m_read_cliptable(clipentry, 0) - xscroll, m_read_cliptable(clipentry, 1) - xscroll, m_read_cliptable(clipentry, 2) - yscroll, m_read_cliptable(clipentry, 3) - yscroll);
	sprite_ptr->clip = clip;

	hpos = util::sext(hpos & 0x7ff, 11); /* sign extend */
	vpos = util::sext(vpos & 0x7ff, 11); /* sign extend */

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
	// 16.16 position offset by dx source pixels; when flipped, hpos is the right edge
	const u32 zoomx = (hsize << 16) / (num_cols * 16);
	s32 dx_zoomed = (dx & 0xff) * zoomx;
	if (dx & 0x100) dx_zoomed = -dx_zoomed;

	s32 x;
	if (!flipx)
	{
		x = (hpos << 16) - dx_zoomed;
	}
	else
	{
		x = (hpos << 16) + dx_zoomed - (hsize << 16);
	}

	if (num_rows == 0) num_rows = 0x10;
	const bool flipy = (vsize & 0x8000);
	vsize &= 0x3ff;
	if (vsize == 0)
	{
		sprite_ptr->disable = true;
		return;
	}
	// 16.16 position offset by dy source pixels; when flipped, vpos is the bottom edge
	const u32 zoomy = (vsize << 16) / (num_rows * 16);
	s32 dy_zoomed = (dy & 0xff) * zoomy;
	if (dy & 0x100) dy_zoomed = -dy_zoomed;

	s32 y;
	if (!flipy)
	{
		y = (vpos << 16) - dy_zoomed;
	}
	else
	{
		y = (vpos << 16) + dy_zoomed - (vsize << 16);
	}

	sprite_ptr->flipx = flipx;
	sprite_ptr->flipy = flipy;
	sprite_ptr->cols = num_cols;
	sprite_ptr->rows = num_rows;
	sprite_ptr->color = palette & (m_colors - 1);
	sprite_ptr->x = x;
	sprite_ptr->y = y;
	// HSIZE/VSIZE are the sprite size on screen
	sprite_ptr->stepx = (((num_cols * 16) << 16) + hsize - 1) / hsize;
	sprite_ptr->stepy = (((num_rows * 16) << 16) + vsize - 1) / vsize;

	for (int ind = 0; ind < num_rows * num_cols; ind++)
		sprite_ptr->tile[ind] = m_read_spritetile(tile_index++);
}
