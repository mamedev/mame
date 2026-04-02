// license:BSD-3-Clause
// copyright-holders:Luca Elia
/* "013" Sprite generator manufactured by NEC

Used by:
- atlus/cave.cpp

TODO:
- Videoregs usage not verified other than described below

***************************************************************************

-- Original docs from atlus/cave.cpp:

    [ 1024 Zooming Sprites ]

        There are 2 or 4 0x4000 Sprite RAM Areas. A hardware register's
        bit selects an area to display (sprites double buffering).

        The sprites are NOT tile based: the "tile" size and start address
        is selectable for each sprite with a 16 pixel granularity.

        Also note that the zoom is of a peculiar type: pixels are never
        drawn more than once. So shrinking works as usual (some pixels are
        just not drawn) while enlarging adds some transparent pixels to
        the image, uniformly, to reach the final size.

***************************************************************************

                            Zoomed Sprites Drawing

    Sprite format with zoom, 16 bytes per each sprites

    Offset:     Bits:                   Value:

    00.w                                X Position*

    02.w                                Y Position*

    04.w        fe-- ---- ---- ----
                --dc ba98 ---- ----     Color
                ---- ---- 76-- ----
                ---- ---- --54 ----     Priority
                ---- ---- ---- 3---     Flip X
                ---- ---- ---- -2--     Flip Y
                ---- ---- ---- --10     Code High Bit(s?)

    06.w                                Code Low Bits

    08/0A.w                             Zoom X / Y

    0C.w        fedc ba98 ---- ----     Tile Size X
                ---- ---- 7654 3210     Tile Size Y

    0E.w                                Unused

    * S.9.6 Fixed point or 10 bit signed integer,
      Configured from videoregs


                                Sprites Drawing

    Sprite format without zoom, 16 bytes per each sprites

    Offset:     Bits:                   Value:

    00.w        fe-- ---- ---- ----
                --dc ba98 ---- ----     Color
                ---- ---- 76-- ----
                ---- ---- --54 ----     Priority
                ---- ---- ---- 3---     Flip X
                ---- ---- ---- -2--     Flip Y
                ---- ---- ---- --10     Code High Bit(s?)

    02.w                                Code Low Bits

    04.w        fedc ba-- ---- ----
                ---- --98 7654 3210     X Position**

    06.w        fedc ba-- ---- ----
                ---- --98 7654 3210     Y Position**

    08.w        fedc ba98 ---- ----     Tile Size X
                ---- ---- 7654 3210     Tile Size Y

    0A.w                                Unused

    0C.w                                Unused

    0E.w                                Unused

    ** 10 bit signed only? need verifications.

***************************************************************************

                      Sprites Registers (videoregs)


    Offset:     Bits:                   Value:

       00.w     f--- ---- ---- ----     Sprites Flip X
                -edc ba98 7654 3210     Sprites Offset X

       02.w     f--- ---- ---- ----     Sprites Flip Y
                -edc ba98 7654 3210     Sprites Offset Y

        ..

       08.w     fedc ba98 7654 32--
                ---- ---- ---- --10     Sprite RAM Bank

       0A.w     fe-- ---- ---- ----
                --dc ---- ---- ----     Sprite position format*
                ---- ba98 7654 3210

       There are more!

       * 0b00 = 10 bit signed integer, 0b11 = S.9.6 fixed point;
         Separated per X/Y?

***************************************************************************

*/

#include "emu.h"
#include "sprite013.h"

DEFINE_DEVICE_TYPE(SPRITE013, sprite013_device, "sprite013", "013 Sprite generator")

sprite013_device::sprite013_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPRITE013, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this)
	, m_spriteram(*this, finder_base::DUMMY_TAG)
	, m_videoregs(*this, "videoregs", 0x80, ENDIANNESS_BIG)
	, m_colpri_cb(*this)
	, m_decrypt_cb(*this)
	, m_alt_format(false)
	, m_delay_sprite(false)
	, m_x_offset(0)
	, m_y_offset(0)
	, m_transpen(0)
	, m_granularity(16)
	, m_sprite_gfx_mask(0)
	, m_max_sprite_clk(0)
	, m_sprite_bitmap(512, 512) // actually double buffered (256K x 16 bit DRAM per buffer)
	, m_spriteram_bank(0)
	, m_spriteram_bank_delay(0)
{
}

void sprite013_device::device_start()
{
	m_colpri_cb.resolve_safe(false);
	m_decrypt_cb.resolve();

	// decrypt sprite ROM first for get gfxs, programmable or hardcoded per games?
	if (!m_decrypt_cb.isnull())
		m_decrypt_cb();

	decode_gfx();
	gfx(0)->set_granularity(m_granularity);

	// unpack sprites
	gfx_element *gfx = this->gfx(0);
	m_sprite_gfx_mask = 1;
	const u32 needed = gfx->elements() * gfx->height() * gfx->width();
	while (m_sprite_gfx_mask < needed)
	{
		m_sprite_gfx_mask <<= 1;
	}
	m_sprite_gfx = make_unique_clear<u8[]>(m_sprite_gfx_mask);

	u8 *dst = m_sprite_gfx.get();
	for (int e = 0; e < gfx->elements(); e++)
	{
		const u8 *data = gfx->get_data(e);
		for (int y = 0; y < gfx->height(); y++)
		{
			const u8 *datatmp = data;
			for (int x = 0; x < gfx->width(); x++)
			{
				*dst++ = *datatmp++;
			}
			data += gfx->rowbytes();
		}
	}
	m_sprite_gfx_mask--;

	// get max usable clock for sprites
	m_max_sprite_clk = (screen().visible_area().width() > 360 ? 512 : 448) * 272 * 2; // whole screen size related?

	save_item(NAME(m_sprite_bitmap));
	save_item(NAME(m_spriteram_bank));
	save_item(NAME(m_spriteram_bank_delay));
}

void sprite013_device::device_reset()
{
}

u16 sprite013_device::videoregs_r(offs_t offset)
{
	return m_videoregs[offset];
}

void sprite013_device::videoregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_videoregs[offset]);
	offset <<= 1;
	// offset 0x04 and 0x06 is offset related?
	// offset 0x0a is position mode toggle in bit 13-12 (separated for X and Y?), other bits used but unknown
	// offset 0x68 or 0x78 is commonly watchdog or DMA command?
	// offset 0x6c or 0x7c is encryption key or CRTC or something else?
	// offset 0x6e is commonly communication when sound CPU exists
	// other registers unknown
	switch (offset)
	{
	case 0x00:
	case 0x02:
	case 0x04:
	case 0x06:
	case 0x08:
	case 0x0a:
	case 0x68:
	case 0x6e:
	case 0x78:
		break;
	default:
		logerror("%s: Unknown videoregs written %04X = %04X & %04X\n", machine().describe_context(), offset, data, mem_mask);
	}
}

void sprite013_device::get_sprite_info(const rectangle &cliprect)
{
	const int spriteram_bankmax = m_spriteram.bytes() / 0x4000;
	const u8 bank = (m_videoregs[4] & 3) % spriteram_bankmax;

	if (m_delay_sprite)   /* mazinger metmqstr */
	{
		if (machine().video().skip_this_frame() == 0)
		{
			m_spriteram_bank = m_spriteram_bank_delay;
			draw_sprites(cliprect);
		}
		m_spriteram_bank_delay = bank;
	}
	else
	{
		if (machine().video().skip_this_frame() == 0)
		{
			m_spriteram_bank = bank;
			draw_sprites(cliprect);
		}
	}
}

void sprite013_device::draw_sprites(const rectangle &cliprect)
{
	rectangle clip(cliprect);
	clip &= m_sprite_bitmap.cliprect();
	m_sprite_bitmap.fill(0, clip);

	const bool glob_flipx = m_videoregs[0] & 0x8000;
	const bool glob_flipy = m_videoregs[1] & 0x8000;

	const int max_x = screen().width();
	const int max_y = screen().height();

	const u16 *source = &m_spriteram[(0x4000 / 2) * m_spriteram_bank];
	const u16 *finish = source + (0x4000 / 2);
	u32 clk = 0; // used clock cycle for sprites

	for (; source < finish; source += 8)
	{
		clk += 32; // 32 clock per each sprites
		if (clk > m_max_sprite_clk)
			break;

		int total_width, total_height;

		if (m_alt_format) // no zoomed
		{
			const u16 attr = source[0];
			const u32 code = source[1] + ((attr & 3) << 16);
			int x          = source[2] & 0x3ff;
			int y          = source[3] & 0x3ff;

			const u16 size = source[4];

			const int tile_width  = total_width  = ((size >> 8) & 0x1f) * 16;
			const int tile_height = total_height = ((size >> 0) & 0x1f) * 16;

			x = util::sext((x + m_x_offset) & 0x3ff, 10);
			y = util::sext((y + m_y_offset) & 0x3ff, 10);

			if (!tile_width || !tile_height ||
				x + total_width <= 0 || x >= max_x || y + total_height <= 0 || y >= max_y)
				continue;

			clk += tile_width * tile_height; // 256 clock per each sprite blocks
			if (clk > m_max_sprite_clk)
				break;

			bool flipx = BIT(attr, 3);
			bool flipy = BIT(attr, 2);

			const u16 colpri = ((attr & 0x0030) << 10) | (attr & 0x3f00);

			if (glob_flipx) { x = m_x_offset_flip - x - total_width;  flipx = !flipx; }
			if (glob_flipy) { y = m_y_offset_flip - y - total_height; flipy = !flipy; }

			draw_single_sprite_nozoom(clip, code, colpri, x, y, flipx, flipy, tile_width, tile_height);
		}
		else // zoomed
		{
			int x, y;
			int total_width_f, total_height_f;

			if ((m_videoregs[5] & 0x3000) == 0)    // if bit 12/13 is 0 (or separated per X and Y?)
			{
				x = (source[0] & 0x3ff) << 8;
				y = (source[1] & 0x3ff) << 8;
			}
			else
			{
				x = source[0] << 2;
				y = source[1] << 2;
			}
			const u16 attr  = source[2];
			const u32 code  = source[3] + ((attr & 3) << 16);
			const int zoomx = source[4];
			const int zoomy = source[5];
			const u16 size  = source[6];

			const int tile_width  = ((size >> 8) & 0x1f) * 16;
			const int tile_height = ((size >> 0) & 0x1f) * 16;

			if (!tile_width || !tile_height)
				continue;

			clk += tile_width * tile_height; // 256 clock per each sprite blocks
			if (clk > m_max_sprite_clk)
				break;

			bool flipx = BIT(attr, 3);
			bool flipy = BIT(attr, 2);

			total_width  = (total_width_f  = tile_width  * zoomx) / 0x100;
			total_height = (total_height_f = tile_height * zoomy) / 0x100;

			if (total_width <= 1)
			{
				x -= 0x80;
			}

			if (total_height <= 1)
			{
				y -= 0x80;
			}

			if ((m_videoregs[5] & 0x3000) == 0)
			{
				x >>= 8;
				y >>= 8;
				if (flipx && (zoomx != 0x100)) x += tile_width - total_width;
				if (flipy && (zoomy != 0x100)) y += tile_height - total_height;
			}
			else
			{
				if (flipx && (zoomx != 0x100)) x += (tile_width << 8) - total_width_f - 0x80;
				if (flipy && (zoomy != 0x100)) y += (tile_height << 8) - total_height_f - 0x80;
				x >>= 8;
				y >>= 8;
			}

			x = util::sext((x + m_x_offset) & 0x3ff, 10);
			y = util::sext((y + m_y_offset) & 0x3ff, 10);

			if (x + total_width <= 0 || x >= max_x || y + total_height <= 0 || y >= max_y)
				continue;

			const u16 colpri = ((attr & 0x0030) << 10) | (attr & 0x3f00);

			if (glob_flipx) { x = m_x_offset_flip - x - total_width;  flipx = !flipx; }
			if (glob_flipy) { y = m_y_offset_flip - y - total_height; flipy = !flipy; }

			if ((zoomx == 0x100) && (zoomy == 0x100))
				draw_single_sprite_nozoom(clip, code, colpri, x, y, flipx, flipy, tile_width, tile_height);
			else
				draw_single_sprite_zoom(clip, code, colpri, x, y, flipx, flipy, tile_width, tile_height, zoomx, zoomy);
		}
	}
}

void sprite013_device::draw_single_sprite_nozoom(
		const rectangle &cliprect,
		u32 code, u16 colpri,
		int x, int y,
		bool flipx, bool flipy,
		int width, int height)
{
	int drawx_start = x;
	int drawy_start = y;
	int srcx_start, srcx_end, srcx_inc;
	int srcy_start, srcy_end, srcy_inc;
	if (flipx)
	{
		srcx_start = width - 1;
		srcx_end = -1;
		srcx_inc = -1;
	}
	else
	{
		srcx_start = 0;
		srcx_end = width;
		srcx_inc = 1;
	}
	if (flipy)
	{
		srcy_start = height - 1;
		srcy_end = -1;
		srcy_inc = -1;
	}
	else
	{
		srcy_start = 0;
		srcy_end = height;
		srcy_inc = 1;
	}
	int srcx_base = srcx_start;
	int srcy = srcy_start;
	if (drawx_start < cliprect.min_x)
	{
		const int diff = cliprect.min_x - drawx_start;
		srcx_base += srcx_inc * diff;
		if ((flipx && srcx_base < 0) || ((!flipx) && srcx_base >= width))
			return;

		drawx_start += diff;
	}
	if (drawy_start < cliprect.min_y)
	{
		const int diff = cliprect.min_y - drawy_start;
		srcy += srcy_inc * diff;
		if ((flipy && srcy < 0) || ((!flipy) && srcy >= height))
			return;

		drawy_start += diff;
	}
	if (drawx_start > cliprect.max_x || drawy_start > cliprect.max_y)
		return;

	code <<= 8;
	for (int drawy = drawy_start; (srcy != srcy_end) && (drawy <= cliprect.max_y); drawy++, srcy += srcy_inc)
	{
		u16 *const dstbitmap = &m_sprite_bitmap.pix(drawy);
		const u32 srcoffs = code + (srcy * width);
		for (int drawx = drawx_start, srcx = srcx_base; (srcx != srcx_end) && (drawx <= cliprect.max_x); drawx++, srcx += srcx_inc)
		{
			const u8 pix = m_sprite_gfx[(srcoffs + srcx) & m_sprite_gfx_mask];
			if (pix != m_transpen)
				dstbitmap[drawx] = pix | colpri;
		}
	}
}

void sprite013_device::draw_single_sprite_zoom(
		const rectangle &cliprect,
		u32 code, u16 colpri,
		int x, int y,
		bool flipx, bool flipy,
		int width, int height,
		int zoomx, int zoomy)
{
	const rectangle clip_scaled(cliprect.min_x << 8, (cliprect.max_x + 1) << 8, cliprect.min_y << 8, (cliprect.max_y + 1) << 8);
	int drawx_start = x << 8;
	int drawy_start = y << 8;
	int srcx_start, srcx_end, srcx_inc;
	int srcy_start, srcy_end, srcy_inc;
	if (flipx)
	{
		srcx_start = width - 1;
		srcx_end = -1;
		srcx_inc = -1;
	}
	else
	{
		srcx_start = 0;
		srcx_end = width;
		srcx_inc = 1;
	}
	if (flipy)
	{
		srcy_start = height - 1;
		srcy_end = -1;
		srcy_inc = -1;
	}
	else
	{
		srcy_start = 0;
		srcy_end = height;
		srcy_inc = 1;
	}
	int srcx_base = srcx_start;
	int srcy = srcy_start;
	while (drawx_start < clip_scaled.min_x)
	{
		srcx_base += srcx_inc;
		if (srcx_base == srcx_end)
			return;

		drawx_start += zoomx;
	}
	while (drawy_start < clip_scaled.min_y)
	{
		srcy += srcy_inc;
		if (srcy == srcy_end)
			return;

		drawy_start += zoomy;
	}
	if (drawx_start >= clip_scaled.max_x || drawy_start >= clip_scaled.max_y)
		return;

	code <<= 8;
	for (int drawy = drawy_start; (srcy != srcy_end) && (drawy < clip_scaled.max_y); drawy += zoomy, srcy += srcy_inc)
	{
		u16 *const dstbitmap = &m_sprite_bitmap.pix(drawy >> 8);
		const u32 srcoffs = code + (srcy * width);
		for (int drawx = drawx_start, srcx = srcx_base; (srcx != srcx_end) && (drawx < clip_scaled.max_x); drawx += zoomx, srcx += srcx_inc)
		{
			const u8 pix = m_sprite_gfx[(srcoffs + srcx) & m_sprite_gfx_mask];
			if (pix != m_transpen)
				dstbitmap[drawx >> 8] = pix | colpri;
		}
	}
}

void sprite013_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip(cliprect);
	clip &= m_sprite_bitmap.cliprect();

	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		u16 const *const srcbitmap = &m_sprite_bitmap.pix(y);
		u16 *const dstbitmap = &bitmap.pix(y);
		u8 *const dstprimap = &screen.priority().pix(y);
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			const u16 pixel = srcbitmap[x];
			if (pixel != 0)
			{
				const u8 pen = pixel & 0xff;
				u32 colpri = (pixel >> 8) & 0xff;
				if (m_colpri_cb(dstprimap[x], colpri))
					dstbitmap[x] = gfx(0)->colorbase() + pen + ((colpri % gfx(0)->colors()) * gfx(0)->granularity());
			}
		}
	}
}

void sprite013_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *palette = &gfx(0)->palette().pens()[gfx(0)->colorbase()];
	rectangle clip(cliprect);
	clip &= m_sprite_bitmap.cliprect();

	for (int y = clip.min_y; y <= clip.max_y; y++)
	{
		u16 const *const srcbitmap = &m_sprite_bitmap.pix(y);
		u32 *const dstbitmap = &bitmap.pix(y);
		u8 *const dstprimap = &screen.priority().pix(y);
		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			const u16 pixel = srcbitmap[x];
			if (pixel != 0)
			{
				const u8 pen = pixel & 0xff;
				u32 colpri = (pixel >> 8) & 0xff;
				if (m_colpri_cb(dstprimap[x], colpri))
					dstbitmap[x] = palette[pen + ((colpri % gfx(0)->colors()) * gfx(0)->granularity())];
			}
		}
	}
}
