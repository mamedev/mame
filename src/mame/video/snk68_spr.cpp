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
#include "snk68_spr.h"

DEFINE_DEVICE_TYPE(SNK68_SPR, snk68_spr_device, "snk68_spr", "SNK68 Sprites")

snk68_spr_device::snk68_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNK68_SPR, tag, owner, clock)
	, m_newtilecb(*this, FUNC(snk68_spr_device::tile_callback_noindirect))
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_spriteram(*this, "^spriteram")
	, m_screen(*this, "^screen")
	, m_flipscreen(false)
	, m_partialupdates(1)
{
}

void snk68_spr_device::tile_callback_noindirect(int &tile, int& fx, int& fy, int& region)
{
}

void snk68_spr_device::device_start()
{
	// bind our handler
	m_newtilecb.resolve();
}

void snk68_spr_device::device_reset()
{
}

READ16_MEMBER(snk68_spr_device::spriteram_r)
{
	// streetsj expects the MSB of every 32-bit word to be FF. Presumably RAM
	// exists only for 3 bytes out of 4 and the fourth is unmapped.
	if (!(offset & 1))
		return m_spriteram[offset] | 0xff00;
	else
		return m_spriteram[offset];
}

WRITE16_MEMBER(snk68_spr_device::spriteram_w)
{
	uint16_t newword = m_spriteram[offset];

	if (!(offset & 1))
		data |= 0xff00;

	COMBINE_DATA(&newword);

	if (m_spriteram[offset] != newword)
	{
		int vpos = m_screen->vpos();

		if (vpos > 0)
			if (m_partialupdates) m_screen->update_partial(vpos - 1);

		m_spriteram[offset] = newword;
	}
}

void snk68_spr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group, u16 start_offset, u16 end_offset)
{
	const uint16_t* tiledata = &m_spriteram[0x800*group + start_offset];

	bool const flip = m_flipscreen;

	for (int offs = start_offset; offs < end_offset; offs += 0x40)
	{
		int mx = (m_spriteram[offs + 2*group] & 0xff) << (16-m_xpos_shift);
		int my = m_spriteram[offs + 2*group + 1];
		int i;

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
		for (i = 0; i < 0x20; ++i)
		{
			my &= 0x1ff;

			if (my <= cliprect.max_y && my + 15 >= cliprect.min_y)
			{
				int color = *(tiledata++) & m_color_entry_mask;
				int tile = *(tiledata++);
				int fx = 0,fy = 0;
				int region = 0;

				m_newtilecb(tile, fx, fy, region);

				// the black touch '96 cloned hardware has some tiles
				// as 8bpp, we need to shift the colour bits in those cases
				int depth = m_gfxdecode->gfx(region)->depth();
				if (depth == 256) color >>= 4;

				if (flip)
				{
					fx = !fx;
					fy = !fy;
				}

				m_gfxdecode->gfx(region)->transpen(bitmap,cliprect,
						tile,
						color,
						fx, fy,
						mx, my, 0);
			}
			else
			{
				tiledata += 2;
			}

			if (flip)
				my -= 16;
			else
				my += 16;
		}
	}
}

void snk68_spr_device::draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* This appears to be the correct priority order */
	draw_sprites(bitmap, cliprect, 2, 0, 0x800);
	draw_sprites(bitmap, cliprect, 3, 0, 0x800);
	draw_sprites(bitmap, cliprect, 1, 0, 0x800);
}

void snk68_spr_device::draw_sprites_alt(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// TODO: alpha68k priority, different vram accessors? Special meaning of bit 0 for my variable?
	draw_sprites(bitmap, cliprect, 1, 0x7c0, 0x800);
	draw_sprites(bitmap, cliprect, 2, 0x000, 0x800);
	draw_sprites(bitmap, cliprect, 3, 0x000, 0x800);
	draw_sprites(bitmap, cliprect, 1, 0x000, 0x7c0);
}

void snk68_spr_device::set_flip(bool flip)
{
	m_flipscreen = flip;
}
