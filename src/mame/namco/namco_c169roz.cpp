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

  Draw is a single pixmap walk. Firmware conventions that used to hide behind
  m_is_namcofl are split out:
    - scanline layer (FL layer 0, NB-2 / System 2 layer 1)
    - color nibble width (FL 3-bit, otherwise 4-bit)

  Per-scanline records already contain that line's startx/starty.
  Adding clip.min_y * incyx/incyy on a 1-line clip double-counted
  Speed Racer's road.

  Wrap-on tiles at params.size then adds left/top. Wrap-off walk is already
  in full 4096×4096 tilemap space: left/top/size are the visible rectangle,
  and samples outside that window are skipped (not wrapped, not sampled).

  Analog (36,3) stays in unpack for both layer-wide and scanline walks
  (chip front-porch).

  Subtracting it on scanlines shifted Speed Racer / Final Lap R roads
  by 36px X and 3px of texture Y.

  Scanline records already hold that line's start,
  so min_y * inc is not applied again.

  FL per-line inc is a 16-bit scale (Speed Racer ldis); bits 14-12 are
  not left/top.

  Mach Breakers producer C ANDs word2/3 with $8FFF then ORs left/top into
  bits 14-12 — same 12-bit signed inc as layer-wide.

  Unpacking those as int16_t turns 0x8xxx (small negative) into ~-28000
  and shears the ranking floor.

  Speed Racer scanline word0 $6000 is the Y wrap period (>>3 → 3072px).
  Mach Breakers word0 $4000 is not a 2048px wrap.

  Firmware starty is already % (word0<<1). Analog is applied after that wrap,
  then the chip's 12-bit wrap; dest-0 also writes row 192.

  Scanline mode stays control[0] == 0x8000.

******************************************************************************/

#include "emu.h"
#include "namco_c169roz.h"

GFXDECODE_START( namco_c169roz_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, gfx_16x16x8_raw, 0, 32 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCO_C169ROZ, namco_c169roz_device, "namco_c169roz", "Namco C169 (ROZ)")

namco_c169roz_device::namco_c169roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCO_C169ROZ, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	m_c169_cb(*this),
	m_color_base(0),
	m_scanline_layer(1),
	m_color_nibble_mask(0x000f),
	m_mask(*this, "mask")
{
}

void namco_c169roz_device::device_start()
{
	m_c169_cb.resolve();

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
 * (mostly scrambling, which could be handled in the game inits, but NB2 also has banking)
 */
template<int Which>
TILE_GET_INFO_MEMBER(namco_c169roz_device::get_info)
{
	// System 2 C169 RAM is half of FL/NB-2 (64 KiB / 32K words vs 128 KiB).
	// 256×256 tilemaps therefore alias through this mask (NS2 wrap/mirror).
	const uint16_t data = m_videoram[tile_index & (m_ramsize - 1)] & 0x3fff;
	int tile = 0, mask = 0;
	if (m_c169_cb.isnull())
		tile = mask = data;
	else
		m_c169_cb(data, tile, mask, Which);

	tileinfo.mask_data = m_mask + 32 * mask;
	tileinfo.set(0, tile, 0/*color*/, 0/*flag*/);
}

TILEMAP_MAPPER_MEMBER(namco_c169roz_device::mapper)
{
	return ((col & 0x80) << 8) | ((row & 0xff) << 7) | (col & 0x7f);
}

void namco_c169roz_device::unpack_params(const uint16_t *source, roz_parameters &params, bool scanline_inc)
{
	// Analogous to Mach Breakers WriteRozMatrixDirect addi.w #0x26 (=38).
	// Kept as the historical C169 analog front-porch; not applied by firmware.
	const int xoffset = 36, yoffset = 3;

	/**
	 * Word 0: used but unknown (per-scanline draw if word0 == 0x8000, bit 14-13 used but unknown method)
	 * Word 1:
	 * x-------.-------- disable layer
	 * ----x---.-------- wrap (0 = wrap, 1 = off)
	 * ------xx.-------- size (512 << n)
	 * word2/3 bits 14-12: left/top of the visible window (512px steps on the 4096 map)
	 * wrap-off: 12-bit full-map coords; scissor [left, left+size) × [top, top+size)
	 * scanline wrap-on: X is the 4096 map. Y period is word0 >> 3 only when
	 * that is the Speed Racer 192-row ring ($6000 → 3072px). Mach Breakers
	 * scanline word0 is $4000 (not a 2048px wrap); treating it as one
	 * squashed the ranking-screen floor.
	 * FL scanline inc is 16-bit. NB-2/System 2 scanline inc is 12-bit
	 * (ANDI #$8FFF, bit15 sign, bits 14-12 left/top) like layer-wide.
	 * --------.xxxx---- priority
	 * --------.----xxxx color (FL uses 3 bits, NB-2 uses 4)
	 */

	uint16_t temp = source[1];
	params.wrap = BIT(~temp, 11);
	params.size = 512 << ((temp & 0x0300) >> 8);
	params.color = (temp & m_color_nibble_mask) * 256;
	params.priority = (temp & 0x00f0) >> 4;
	params.wrap_y = 0x1000;
	if (scanline_inc && m_scanline_layer == 0)
	{
		const uint32_t period = uint32_t(source[0]) >> 3;
		if (period == 0xC00)
			params.wrap_y = period;
	}

	// Speed Racer ldis writes a 16-bit scale. Mach Breakers producer C
	// masks word2/3 with $8FFF (12-bit + bit15) then add.l left/top.
	const bool scanline_16bit_inc = scanline_inc && (m_scanline_layer == 0);

	temp = source[2];
	if (scanline_16bit_inc)
	{
		params.left = 0;
		params.incxx = int16_t(temp);
	}
	else
	{
		params.left = (temp & 0x7000) >> 3;
		if (BIT(temp, 15)) temp |= 0xf000; else temp &= 0x0fff; // sign extend
		params.incxx = int16_t(temp);
	}

	temp = source[3];
	if (scanline_16bit_inc)
	{
		params.top = 0;
		params.incxy = int16_t(temp);
	}
	else
	{
		params.top = (temp & 0x7000) >> 3;
		if (BIT(temp, 15)) temp |= 0xf000; else temp &= 0x0fff; // sign extend
		params.incxy = int16_t(temp);
	}

	temp = source[4];
	if (scanline_16bit_inc)
	{
		params.incyx = int16_t(temp);
	}
	else
	{
		if (BIT(temp, 15)) temp |= 0xf000; else temp &= 0x0fff; // sign extend
		params.incyx = int16_t(temp);
	}

	temp = source[5];
	if (scanline_16bit_inc)
	{
		params.incyy = int16_t(temp);
	}
	else
	{
		if (BIT(temp, 15)) temp |= 0xf000; else temp &= 0x0fff; // sign extend
		params.incyy = int16_t(temp);
	}

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
	params.analog_x = xoffset * params.incxx + yoffset * params.incyx;
	params.analog_y = xoffset * params.incxy + yoffset * params.incyy;
}

void namco_c169roz_device::draw_helper(screen_device &screen, bitmap_ind16 &bitmap, tilemap_t &tmap, const rectangle &clip, const roz_parameters &params, uint8_t prival, uint8_t primask, bool scanline_space)
{
	const uint32_t size_mask = params.size - 1;
	bitmap_ind16 &srcbitmap = tmap.pixmap();
	bitmap_ind8 &flagsbitmap = tmap.flagsmap();

	// Analog is already in start. Scanline records are that line's origin,
	// so adding clip.min_y * inc would pitch the road twice.
	int32_t startx = params.startx + clip.min_x * params.incxx;
	int32_t starty = params.starty + clip.min_x * params.incxy;
	if (!scanline_space)
	{
		startx += clip.min_y * params.incyx;
		starty += clip.min_y * params.incyy;
	}

	int sx = clip.min_x;
	int sy = clip.min_y;
	while (sy <= clip.max_y)
	{
		int x = sx;
		int32_t cx = startx;
		int32_t cy = starty;
		uint16_t *dest = &bitmap.pix(sy, sx);
		uint8_t *destpri = &screen.priority().pix(sy, sx);
		while (x <= clip.max_x)
		{
			uint32_t xpos;
			uint32_t ypos;
			if (params.wrap)
			{
				// Layer-wide: wrap in the size window, then place at left/top.
				// Scanline: firmware writes a 16-bit scale (not 12-bit + left).
				// Official FL draw_roz also wrapped the 4096 pixmap and ignored
				// left/size — road RLE decrements word1 and would shrink size.
				if (scanline_space)
				{
					// X: 4096 map. Y: Speed Racer word0 $6000 is the 192-row
					// ring (3072px). Wrap firmware start at that period, add
					// analog, then 12-bit chip wrap (dest-0 copies row 192).
					// Mach Breakers word0 $4000 is not a 2048px wrap — using
					// it, and splitting analog on every scanline, squashed
					// the ranking floor.
					xpos = uint32_t(cx >> 16) & 0xfff;
					if (params.wrap_y < 0x1000)
					{
						const int32_t cy_fw = cy - params.analog_y;
						uint32_t py = uint32_t(cy_fw >> 16) & 0xfff;
						py %= params.wrap_y;
						const int32_t ypos_s = int32_t(py) + (params.analog_y >> 16);
						ypos = uint32_t(ypos_s) & 0xfff;
					}
					else
					{
						ypos = uint32_t(cy >> 16) & 0xfff;
					}
				}
				else
				{
					const uint32_t wx = uint32_t((cx >> 16) & int32_t(size_mask));
					const uint32_t wy = uint32_t((cy >> 16) & int32_t(size_mask));
					xpos = (wx + params.left) & 0xfff;
					ypos = (wy + params.top) & 0xfff;
				}
			}
			else
			{
				// Full 4096×4096 map, 12-bit coords. Visible window is
				// [left, left+size) × [top, top+size) on that map (wraps if
				// the window crosses 4096). Signed linear clip skipped the
				// Metal Hawk title once zoom pushed start outside the
				// window; analog-strip of the sample then shifted it.
				const uint32_t wx = uint32_t(cx >> 16) & 0xfff;
				const uint32_t wy = uint32_t(cy >> 16) & 0xfff;
				const uint32_t x1 = params.left + params.size;
				const uint32_t y1 = params.top + params.size;
				const bool in_x = (x1 <= 0x1000)
					? (wx >= params.left && wx < x1)
					: (wx >= params.left || wx < x1 - 0x1000);
				const bool in_y = (y1 <= 0x1000)
					? (wy >= params.top && wy < y1)
					: (wy >= params.top || wy < y1 - 0x1000);
				if (!in_x || !in_y)
				{
					cx += params.incxx;
					cy += params.incxy;
					x++;
					dest++;
					destpri++;
					continue;
				}
				xpos = wx;
				ypos = wy;
			}
			if (flagsbitmap.pix(ypos, xpos) & TILEMAP_PIXEL_LAYER0)
			{
				*dest = srcbitmap.pix(ypos, xpos) + params.color + m_color_base;
				*destpri = (*destpri & primask) | prival;
			}
			cx += params.incxx;
			cy += params.incxy;
			x++;
			dest++;
			destpri++;
		}
		startx += params.incyx;
		starty += params.incyy;
		sy++;
	}
}

void namco_c169roz_device::draw_scanline(screen_device &screen, bitmap_ind16 &bitmap, int line, int which, int pri, const rectangle &cliprect, uint8_t prival, uint8_t primask)
{
	if (line >= cliprect.min_y && line <= cliprect.max_y)
	{
		const int row = line >> 3;
		const int offs = row * 0x100 + (line & 7) * 0x10 + 0xe080;
		uint16_t *source = &m_videoram[offs / 2];

		// if enabled
		if (BIT(~source[1], 15))
		{
			roz_parameters params;
			unpack_params(source, params, true);

			// check priority
			if (pri == params.priority)
			{
				rectangle clip(0, bitmap.width() - 1, line, line);
				clip &= cliprect;
				draw_helper(screen, bitmap, *m_tilemap[which], clip, params, prival, primask, true);
			}
		}
	}
}

void namco_c169roz_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, uint8_t prival, uint8_t primask)
{
	// FL/NB-2 firmware tests bit15, but pack 0x8000 / 0x1000 exactly.
	// Matching bit15 alone sent System 2 titles (extra high bits) down the
	// scanline path into empty 0xE080 RAM, so the Metal Hawk logo vanished.
	const bool scanline_mode = (m_control[0] == 0x8000);

	for (int which = 1; which >= 0; which--)
	{
		const uint16_t *source = &m_control[which * 8];
		const uint16_t attrs = source[1];

		// if enabled
		if (BIT(~attrs, 15))
		{
			if (which == m_scanline_layer && scanline_mode)
			{
				for (int line = cliprect.min_y; line <= cliprect.max_y; line++)
					draw_scanline(screen, bitmap, line, which, pri, cliprect, prival, primask);
			}
			else
			{
				roz_parameters params;
				unpack_params(source, params);
				if (params.priority == pri)
					draw_helper(screen, bitmap, *m_tilemap[which], cliprect, params, prival, primask, false);
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
