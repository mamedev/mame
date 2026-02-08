// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
/*
 * Alpha 68k II/V sprite system
 * tile-based, with 8-bit accesses.
 * Two banks, first at 0x0000-0x1000, second at 0x1000 until end of VRAM.
 * First bank is processed by 64 bytes stepping starting from address $8,
 * then once it reaches end it restarts at $c, finally at $4.
 *
 * 0x0000-0x1000
 * [0]
 * ???? ???? untested in POST, actually written to by Gang Wars ($ff in snk68.cpp so NOP)
 * [1]
 * XXXX XXXX lower X offset
 * [2]
 * X--- ---- upper X offset
 * -*** ---- unknown, more fractional X? Definitely used by snk68.cpp
 * ---- ---Y upper Y offset
 * [3]
 * YYYY YYYY lower Y offset
 *
 * Second bank has the actual tile info, and is arranged in vertical strips.
 * [0]
 * ???? ????  untested in POST, actually written to by Sky Adventure ($ff in snk68.cpp so NOP)
 * [1]
 * cccc cccc color entry
 * [2]
 * uuu- ---- user selectable, either flipx/flipy or tile bank
 * ---t tttt high tile offset
 * [3]
 * tttt tttt low tile offset
 *
 * TODO:
 * - Currently lags compared to itself in Sky Adventure at least, examples:
 *   - player death animation has first frame with inverted horizontal halves;
 *   - stage 1 priest desyncs with background;
 *   - glitchy first frame on title screen;
 *   Given how this and the actual HW works it is pretty likely this having a consistent delay,
 *   however it isn't known how exactly DMA triggers, and one frame of bufferd spriteram isn't enough.
 * - Why entry 0x7c0 requires a one line and a priority hack for alpha68k.cpp games?
 * - Super Champion Baseball: selecting a 3rd or 5th column team causes a shadow to appear on the drawn
 *   object which isn't present on the other columns, verify real HW behaviour;
 * - Sky Adventure: stage 1 priest priority goes above the bonus ship, confirmed to be a btanb;
 *
 */

#include "emu.h"
#include "alpha68k_spr.h"

DEFINE_DEVICE_TYPE(ALPHA68K_SPR, alpha68k_sprite_device, "alpha68k_spr", "Alpha Denshi 68K Sprite System")

alpha68k_sprite_device::alpha68k_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ALPHA68K_SPR, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_newtilecb(*this, FUNC(alpha68k_sprite_device::tile_callback_noindirect))
	, m_spriteram(*this, finder_base::DUMMY_TAG)
	, m_flipscreen(false)
	, m_partialupdates(true)
	, m_xpos_shift(0)
{
}

void alpha68k_sprite_device::tile_callback_noindirect(u32 &tile, bool &fx, bool &fy, u8 &region, u32 &color)
{
}

void alpha68k_sprite_device::device_start()
{
	save_item(NAME(m_flipscreen));

	// bind our handler
	m_newtilecb.resolve();
}

void alpha68k_sprite_device::device_reset()
{
}

u16 alpha68k_sprite_device::spriteram_r(offs_t offset)
{
	// streetsj expects the MSB of every 32-bit word to be FF. Presumably RAM
	// exists only for 3 bytes out of 4 and the fourth is unmapped.
	if (!(offset & 1))
		return m_spriteram[offset] | 0xff00;
	else
		return m_spriteram[offset];
}

void alpha68k_sprite_device::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 newword = m_spriteram[offset];

	if (!(offset & 1))
		data |= 0xff00;

	COMBINE_DATA(&newword);

	if (m_spriteram[offset] != newword)
	{
		int vpos = screen().vpos();

		if (vpos > 0)
			if (m_partialupdates) screen().update_partial(vpos - 1);

		m_spriteram[offset] = newword;
	}
}

void alpha68k_sprite_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group, u16 start_offset, u16 end_offset)
{
	const u16* tiledata = &m_spriteram[0x800*group + start_offset];

	bool const flip = m_flipscreen;

	for (int offs = start_offset; offs < end_offset; offs += 0x40)
	{
		int mx = (m_spriteram[offs + 2*group] & 0xff) << (16-m_xpos_shift);
		int my = m_spriteram[offs + 2*group + 1];

		// TODO: for some reason alpha68k wants this to be 15 instead of 12, different row/col arrangement?
		mx = mx | (my >> m_xpos_shift);

		mx = ((mx + 16) & 0x1ff) - 16;
		my = -my;

		// TODO: alpha68k games all wants this hack, why?
		if (group == 1 && start_offset == 0x7c0)
			my++;

		if (flip)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		// every sprite is a column 32 tiles (512 pixels) tall
		for (int i = 0; i < 0x20; ++i)
		{
			my &= 0x1ff;

			if (my <= cliprect.max_y && my + 15 >= cliprect.min_y)
			{
				u32 color = tiledata[0] & 0xff;
				u32 tile = tiledata[1];
				bool fx = false, fy = false;
				u8 region = 0;

				m_newtilecb(tile, fx, fy, region, color);

				if (flip)
				{
					fx = !fx;
					fy = !fy;
				}

				gfx(region)->transpen(bitmap,cliprect,
						tile,
						color,
						fx, fy,
						mx, my, 0);
			}
			tiledata += 2;

			if (flip)
				my -= 16;
			else
				my += 16;
		}
	}
}

void alpha68k_sprite_device::draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* This appears to be the correct priority order */
	draw_sprites(bitmap, cliprect, 2, 0, 0x800);
	draw_sprites(bitmap, cliprect, 3, 0, 0x800);
	draw_sprites(bitmap, cliprect, 1, 0, 0x800);
}

void alpha68k_sprite_device::draw_sprites_alt(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: alpha68k priority, different vram accessors? Special meaning of bit 0 for my variable?
	draw_sprites(bitmap, cliprect, 1, 0x7c0, 0x800);
	draw_sprites(bitmap, cliprect, 2, 0x000, 0x800);
	draw_sprites(bitmap, cliprect, 3, 0x000, 0x800);
	draw_sprites(bitmap, cliprect, 1, 0x000, 0x7c0);
}

void alpha68k_sprite_device::set_flip(bool flip)
{
	m_flipscreen = flip;
}
