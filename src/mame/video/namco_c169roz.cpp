// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino
/******************************************************************************

    Namco C169 (ROZ - Rotate and Zoom)

  Advanced rotate-zoom chip manages two layers.
  Each layer uses a designated subset of a master 256x256 tile tilemap (4096x4096 pixels).
  Each layer has configurable color and tile banking.
  ROZ attributes may be specified independently for each scanline.

  Used by:
   Namco NB2 - The Outfoxies, Mach Breakers
   Namco System 2 - Metal Hawk, Lucky and Wild
   Namco System FL - Final Lap R, Speed Racer

******************************************************************************/

#include "emu.h"
#include "namco_c169roz.h"

static const gfx_layout layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,8*16) },
	16*128
};

GFXDECODE_START( namco_c169roz_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, layout, 0, 32 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCO_C169ROZ, namco_c169roz_device, "namco_c169roz", "Namco C169 (ROZ)")

namco_c169roz_device::namco_c169roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCO_C169ROZ, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_color_base(0),
	m_is_namcofl(false),
	m_mask(*this, "mask")
{
}

void namco_c169roz_device::device_start()
{
	m_videoram.resize(m_ramsize);
	std::fill(std::begin(m_videoram), std::end(m_videoram), 0x0000);

	m_tilemap[0] = &machine().tilemap().create(*this,
			tilemap_get_info_delegate(*this, FUNC(namco_c169roz_device::get_info<0>)),
			tilemap_mapper_delegate(*this, FUNC(namco_c169roz_device::mapper)),
			16, 16,
			256, 256);

	m_tilemap[1] = &machine().tilemap().create(*this,
			tilemap_get_info_delegate(*this, FUNC(namco_c169roz_device::get_info<1>)),
			tilemap_mapper_delegate(*this, FUNC(namco_c169roz_device::mapper)),
			16, 16,
			256, 256);

	save_item(NAME(m_control));
	save_item(NAME(m_videoram));
}


// for bank changes
void namco_c169roz_device::mark_all_dirty()
{
	for (auto & elem : m_tilemap)
		elem->mark_all_dirty();
}

/**
 * Graphics ROM addressing varies across games.
 * (mostly scrambling, which could be handled in the game inits, but NB1 also has banking)
 */
template<int Which>
TILE_GET_INFO_MEMBER(namco_c169roz_device::get_info)
{
	int tile = 0, mask = 0;
	m_c169_cb(m_videoram[tile_index&(m_ramsize-1)] & 0x3fff, &tile, &mask, Which); // need to mask with ramsize because the nb1/fl games have twice as much RAM, presumably the tilemaps mirror in ns2?

	tileinfo.mask_data = m_mask + 32 * mask;
	tileinfo.set(0, tile, 0/*color*/, 0/*flag*/);
}

TILEMAP_MAPPER_MEMBER( namco_c169roz_device::mapper )
{
	return ((col & 0x80) << 8) | ((row & 0xff) << 7) | (col & 0x7f);
}

void namco_c169roz_device::unpack_params(const uint16_t *source, roz_parameters &params)
{
	const int xoffset = 36, yoffset = 3;

	/**
	 * x-------.-------- disable layer
	 * ----x---.-------- wrap?
	 * ------xx.-------- size
	 * --------.xxxx---- priority
	 * --------.----xxxx color
	 */

	uint16_t temp = source[1];
	params.wrap = BIT(~temp, 11);
	params.size = 512 << ((temp & 0x0300) >> 8);
	if (m_is_namcofl)
		params.color = (temp & 0x0007) * 256;
	else
		params.color = (temp & 0x000f) * 256;
	params.priority = (temp & 0x00f0) >> 4;

	temp = source[2];
	params.left = (temp & 0x7000) >> 3;
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incxx = int16_t(temp);

	temp = source[3];
	params.top = (temp&0x7000)>>3;
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incxy = int16_t(temp);

	temp = source[4];
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incyx = int16_t(temp);

	temp = source[5];
	if (temp & 0x8000) temp |= 0xf000; else temp &= 0x0fff; // sign extend
	params.incyy = int16_t(temp);

	params.startx = int16_t(source[6]);
	params.starty = int16_t(source[7]);
	params.startx <<= 4;
	params.starty <<= 4;

	params.startx += xoffset * params.incxx + yoffset * params.incyx;
	params.starty += xoffset * params.incxy + yoffset * params.incyy;

	// normalize
	params.startx <<= 8;
	params.starty <<= 8;
	params.incxx <<= 8;
	params.incxy <<= 8;
	params.incyx <<= 8;
	params.incyy <<= 8;
}

void namco_c169roz_device::draw_helper(screen_device &screen, bitmap_ind16 &bitmap, tilemap_t &tmap, const rectangle &clip, const roz_parameters &params)
{
	if (!m_is_namcofl)
//  if (m_gametype != NAMCOFL_FINAL_LAP_R) // Fix speedrcr some title animations, but broke at road scene
	{
		uint32_t size_mask = params.size - 1;
		bitmap_ind16 &srcbitmap = tmap.pixmap();
		bitmap_ind8 &flagsbitmap = tmap.flagsmap();
		uint32_t startx = params.startx + clip.min_x * params.incxx + clip.min_y * params.incyx;
		uint32_t starty = params.starty + clip.min_x * params.incxy + clip.min_y * params.incyy;
		int sx = clip.min_x;
		int sy = clip.min_y;
		while (sy <= clip.max_y)
		{
			int x = sx;
			uint32_t cx = startx;
			uint32_t cy = starty;
			uint16_t *dest = &bitmap.pix(sy, sx);
			while (x <= clip.max_x)
			{ // TODO : Wraparound disable isn't implemented
				uint32_t xpos = (((cx >> 16) & size_mask) + params.left) & 0xfff;
				uint32_t ypos = (((cy >> 16) & size_mask) + params.top) & 0xfff;
				if (flagsbitmap.pix(ypos, xpos) & TILEMAP_PIXEL_LAYER0)
					*dest = srcbitmap.pix(ypos, xpos) + params.color + m_color_base;
				cx += params.incxx;
				cy += params.incxy;
				x++;
				dest++;
			}
			startx += params.incyx;
			starty += params.incyy;
			sy++;
		}
	}
	else
	{
		tmap.set_palette_offset(m_color_base + params.color);
		tmap.draw_roz(
			screen,
			bitmap,
			clip,
			params.startx, params.starty,
			params.incxx, params.incxy,
			params.incyx, params.incyy,
			params.wrap,0,0); // wrap, flags, pri
	}
}

void namco_c169roz_device::draw_scanline(screen_device &screen, bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect)
{
	if (line >= cliprect.min_y && line <= cliprect.max_y)
	{
		int row = line / 8;
		int offs = row * 0x100 + (line & 7) * 0x10 + 0xe080;
		uint16_t *source = &m_videoram[offs / 2];

		// if enabled
		if ((source[1] & 0x8000) == 0)
		{
			roz_parameters params;
			unpack_params(source, params);

			// check priority
			if (pri == params.priority)
			{
				rectangle clip(0, bitmap.width() - 1, line, line);
				clip &= cliprect;
				draw_helper(screen, bitmap, *m_tilemap[which], clip, params);
			}
		}
	}
}

void namco_c169roz_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	int special = (m_is_namcofl) ? 0 : 1;
	int mode = m_control[0]; // 0x8000 or 0x1000

	for (int which = 1; which >= 0; which--)
	{
		const uint16_t *source = &m_control[which * 8];
		uint16_t attrs = source[1];

		// if enabled
		if ((attrs & 0x8000) == 0)
		{
			// second ROZ layer is configured to use per-scanline registers
			if (which == special && mode == 0x8000)
			{
				for (int line = cliprect.min_y; line <= cliprect.max_y; line++)
					draw_scanline(screen, bitmap, line, which, pri, cliprect);
			}
			else
			{
				roz_parameters params;
				unpack_params(source, params);
				if (params.priority == pri)
					draw_helper(screen, bitmap, *m_tilemap[which], cliprect, params);
			}
		}
	}
}

uint16_t namco_c169roz_device::control_r(offs_t offset)
{
	return m_control[offset];
}

void namco_c169roz_device::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_control[offset]);
}

uint16_t namco_c169roz_device::videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

void namco_c169roz_device::videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	for (auto & elem : m_tilemap)
		elem->mark_tile_dirty(offset);
}

