// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Nicola Salmoria,Bryan McPhail,David Haywood,R. Belmont,Alex Marshall,Angelo Salese,Luca Elia
// thanks-to:Richard Bush
/*
    NMK 16 bit sprite hardware

    Configured in NMK008 + one or more NMK009s or TTL logics or FPGA

    From hardware manual : http://upl-gravedigger.boo.jp/pcb_info/pcb_manual_7.jpg

    used by:
    nmk16.cpp
    powerins.cpp

    Sprite format (16 byte per each sprite):

    Offset Bits              Description
           fedcba98 76543210
    00     -------- -------s Visible
    02     ---x---- -------- Flip X (powerins)
           ------x- -------- Flip Y (manybloc)
           -------x -------- Flip X (manybloc) or Code hi bits (powerins)
           -------- xxxx---- Number of tiles along Y, minus 1 (1-16)
           -------- ----xxxx Number of tiles along X, minus 1 (1-16)
    04     -------- -------- Unused
    06     xxxxxxxx xxxxxxxx Code (low 15 bit for powerins)
    08     ------xx xxxxxxxx X (10 bit for powerins, 9 bit for others)
    0a     -------- -------- Unused
    0c     ------xx xxxxxxxx Y (10 bit for powerins, 9 bit for others)
    0e     -------- --xxxxxx Palette select (differ bits per game (4/5/6 bits))
*/


#include "emu.h"
#include "nmk16spr.h"


DEFINE_DEVICE_TYPE(NMK_16BIT_SPRITE, nmk_16bit_sprite_device, "nmk16spr", "NMK 16 bit Sprite hardware")

nmk_16bit_sprite_device::nmk_16bit_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK_16BIT_SPRITE, tag, owner, clock)
	, m_colpri_cb(*this)
	, m_ext_cb(*this)
	, m_flip_screen(false)
	, m_videoshift(0)
	, m_xmask(0x1ff), m_ymask(0x1ff)
	, m_screen_width(384), m_screen_height(256)
	, m_max_sprite_clock(384 * 263)
{
}

// this implementation was originally from nmk16.cpp
void nmk_16bit_sprite_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size)
{
	const bool priority = !m_colpri_cb.isnull();
	sprite_t *sprite_ptr = m_spritelist.get();
	const int xpos_max = m_xmask + 1;
	const int ypos_max = m_ymask + 1;

	u32 clk = 0;
	for (int offs = 0; offs < size; offs += 8)
	{
		clk += 16; // 16 clock per each sprites
		if (clk >= m_max_sprite_clock)
			break;

		if (!(spriteram[offs + 0] & 0x0001))
			continue;

		// extract parameters
		u32 pri_mask = 0;
		int sx          = (spriteram[offs + 4] & m_xmask) + m_videoshift;
		int sy          =  spriteram[offs + 6] & m_ymask;
		int code        =  spriteram[offs + 3];
		u32 colour      =  spriteram[offs + 7];
		if (priority)
			m_colpri_cb(colour, pri_mask);

		const int w     =  spriteram[offs + 1] & 0x00f;
		const int h     = (spriteram[offs + 1] & 0x0f0) >> 4;
		int flipy       = 0;
		int flipx       = 0;
		if (!m_ext_cb.isnull())
			m_ext_cb(spriteram[offs + 1], flipx, flipy, code);

		clk += 128 * w * h; // 128 clock per each 16x16 tile
		if (clk >= m_max_sprite_clock)
			break;

		int delta = 16;

		if (m_flip_screen)
		{
			sx = m_screen_width - 16 - sx;
			sy = m_screen_height - 16 - sy;
			delta = -delta;
		}

		// calculate accumulators
		const int flipx_global = flipx ^ m_flip_screen;
		const int flipy_global = flipy ^ m_flip_screen;

		const int xinc = delta * (flipx ? -1 : 1);
		const int yinc = delta * (flipy ? -1 : 1);

		int xx_base = w;
		int yy = h;

		sx += flipx ? (delta * w) : 0;
		sy += flipy ? (delta * h) : 0;

		// restrict to cliprect
		if (m_flip_screen)
		{
			if (sx < cliprect.min_x - 0xf) sx += xpos_max;
			if (sx > cliprect.max_x)
			{
				const int pixels = (sx - cliprect.max_x) / 16;
				code += pixels;
				sx += pixels * xinc;
				xx_base -= pixels;
			}

			if (sy < cliprect.min_y - 0xf) sy += ypos_max;
			if (sy > cliprect.max_y)
			{
				const int pixels = (sy - cliprect.max_y) / 16;
				code += pixels * (w + 1);
				sy += pixels * yinc;
				yy -= pixels;
			}
		}
		else
		{
			if (sx > cliprect.max_x) sx -= xpos_max;
			if (sx < cliprect.min_x - 0xf)
			{
				const int pixels = ((cliprect.min_x - 0xf) - sx) / 16;
				code += pixels;
				sx += pixels * xinc;
				xx_base -= pixels;
			}

			if (sy > cliprect.max_y) sy -= ypos_max;
			if (sy < cliprect.min_y - 0xf)
			{
				const int pixels = ((cliprect.min_y - 0xf) - sy) / 16;
				code += pixels * (w + 1);
				sy += pixels * yinc;
				yy -= pixels;
			}
		}
		if ((xx_base < 0) || (yy < 0))
			continue;

		// draw single sprite
		do
		{
			// wraparound Y
			if (yinc > 0)
			{
				if (sy > cliprect.max_y) sy -= ypos_max;
				if (sy < cliprect.min_y - 0xf)
				{
					const int pixels = ((cliprect.min_y - 0xf) - sy) / 16;
					code += pixels * (w + 1);
					sy += pixels * yinc;
					yy -= pixels;
				}
			}
			else if (yinc < 0)
			{
				if (sy < cliprect.min_y - 0xf) sy += ypos_max;
				if (sy > cliprect.max_y)
				{
					const int pixels = (sy - cliprect.max_y) / 16;
					code += pixels * (w + 1);
					sy += pixels * yinc;
					yy -= pixels;
				}
			}
			if (yy < 0)
				continue;

			int x = sx;
			int xx = xx_base;
			int codecol = code;
			do
			{
				// wraparound X
				if (xinc > 0)
				{
					if (x > cliprect.max_x) x -= xpos_max;
					if (x < cliprect.min_x - 0xf)
					{
						const int pixels = ((cliprect.min_x - 0xf) - x) / 16;
						codecol += pixels;
						x += pixels * xinc;
						xx -= pixels;
					}
				}
				else if (xinc < 0)
				{
					if (x < cliprect.min_x - 0xf) x += xpos_max;
					if (x > cliprect.max_x)
					{
						const int pixels = (x - cliprect.max_x) / 16;
						codecol += pixels;
						x += pixels * xinc;
						xx -= pixels;
					}
				}
				if (xx < 0)
					continue;

				if (priority)
				{
					sprite_ptr->code = codecol;
					sprite_ptr->colour = colour;
					sprite_ptr->flipx = flipx_global;
					sprite_ptr->flipy = flipy_global;
					sprite_ptr->x = x;
					sprite_ptr->y = sy;
					sprite_ptr->pri_mask = pri_mask;
					sprite_ptr++;
				}
				else
				{
					gfx->transpen(bitmap, cliprect,
						codecol,
						colour,
						flipx_global, flipy_global,
						x, sy, 15);
				}
				codecol++;
				x += xinc;
			} while (--xx >= 0);
			code += (w + 1);
			sy += yinc;
		} while (--yy >= 0);
	}

	if (priority)
	{
		while (sprite_ptr != m_spritelist.get())
		{
			sprite_ptr--;

			gfx->prio_transpen(bitmap, cliprect,
					sprite_ptr->code,
					sprite_ptr->colour,
					sprite_ptr->flipx, sprite_ptr->flipy,
					sprite_ptr->x, sprite_ptr->y, screen.priority(), sprite_ptr->pri_mask, 15);
		}
	}
}

void nmk_16bit_sprite_device::device_start()
{
	m_colpri_cb.resolve();
	m_ext_cb.resolve();
	m_flip_screen = false;
	m_spritelist = std::make_unique<sprite_t[]>((0x1000/0x10) * 16 * 16);

	save_item(NAME(m_flip_screen));
}

void nmk_16bit_sprite_device::device_reset()
{
}
