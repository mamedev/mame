// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria

/* SNK68 Sprites */
#include "emu.h"
#include "snk68_spr.h"

DEFINE_DEVICE_TYPE(SNK68_SPR, snk68_spr_device, "snk68_spr", "SNK68 Sprites")

snk68_spr_device::snk68_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNK68_SPR, tag, owner, clock)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_spriteram(*this, "^spriteram")
	, m_screen(*this, "^screen")
	, m_flipscreen(false)
	, m_partialupdates(1)
{
	m_newtilecb =  snk68_tile_indirection_delegate(FUNC(snk68_spr_device::tile_callback_noindirect), this);
}

void snk68_spr_device::tile_callback_noindirect(int &tile, int& fx, int& fy, int& region)
{
}

void snk68_spr_device::device_start()
{
	// bind our handler
	m_newtilecb.bind_relative_to(*owner());
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

void snk68_spr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group)
{
	const uint16_t* tiledata = &m_spriteram[0x800*group];

	bool const flip = m_flipscreen;

	for (int offs = 0; offs < 0x800; offs += 0x40)
	{
		int mx = (m_spriteram[offs + 2*group] & 0xff) << 4;
		int my = m_spriteram[offs + 2*group + 1];
		int i;

		mx = mx | (my >> 12);

		mx = ((mx + 16) & 0x1ff) - 16;
		my = -my;

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
				int color = *(tiledata++) & 0x7f;
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
	draw_sprites(bitmap, cliprect, 2);
	draw_sprites(bitmap, cliprect, 3);
	draw_sprites(bitmap, cliprect, 1);
}

void snk68_spr_device::set_flip(bool flip)
{
	m_flipscreen = flip;
}
