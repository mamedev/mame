// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    Yamaha YGV625 "PVDC6R" sprite processor / CRTC

    All of the information below was derived from observing the Amuzy
    games, since no programming manual for this chip is available.

    Sprite attribute entry (16 bytes, big-endian words):

      +0  bit 11      transparency enable (the transparent colour index is
                      taken from the sprite header, 0x0001 for direct colour)
          bit 10      palette bit 8
          bits 7..0   palette number
      +2  bits 15..8  height - 1 (low 8 bits)
          bits 7..0   width - 1 (low 8 bits)
      +4  CG memory byte address of the sprite data (32-bit)
      +8  bits 11..0  Y position of the sprite centre (signed)
          bit 12      height - 1, bit 8
          bit 15      Y flip
      +10 bits 11..0  X position of the sprite centre (signed)
          bit 12      width - 1, bit 8
          bit 15      X flip
      +12 bits 15..8  vertical zoom factor, 0x40 = 1.0
          bits 7..0   horizontal zoom factor, 0x40 = 1.0
          (quadrilateral mode: bits 7..0 index the corner table instead,
          entry = 0x2000 + index * 8; the games set bits 15..8 to 0x84/0xc4)
      +14 bit 15      quadrilateral mode: the sprite is mapped onto the four
                      corners given by the corner table entry: Y (positive
                      upwards) and X pairs for the bottom-left, top-left,
                      top-right and bottom-right corners, 11-bit two's
                      complement pixel offsets from the centre position.
                      Used for rotating/distorting sprites: wanpakup's sinking
                      ships, mmhammer's tumbling characters and spinning
                      banner, docchift's bouncing ball (verified against
                      recordings of the real machines)
          bit 14      last entry of the display list
          bit 13      entry disabled
          bit 12      entry enabled

    Window layout: 0x0000-0x1fff attribute table (512 entries), 0x2000-
    corner table (16 bytes per entry), 0x3800-0x38ff colour table,
    0x3c00- registers.

    Registers (word offsets from 0x3c00):

      0x3c06-0x3c0e  palette table base addresses in CG memory, in 32 byte
                     units, for 4, 5, 6, 7 and 8 bit per pixel sprites;
                     a palette of 2^depth colours (RGB565, big-endian) is at
                     base + palette * 2^(depth+1).  Palettes 0x100 and up
                     come from the colour table in the chip (0x3800) instead.
      0x3c30-0x3c46  CRTC timing (0x3c3e/0x3c42 horizontal display 72..392,
                     0x3c3c/0x3c40 vertical display 14..254)
      0x3c4c         status: bit 0 vblank, bit 3 busy, bit 8 CG read error,
                     bit 14 CG read data ready
      0x3c4e         control: bit 5 "update display" (self clearing)
      0x3c50         bit 15 busy
      0x3c6c/0x3c6e  CG memory read address (bit 15 of 0x3c6c starts a read)
      0x3c70         CG memory read data

    Sprite data in CG memory starts with a format byte:

      0x01  raw 4 bpp, 16x16 blocks        0x21  raw 8 bpp, 16x16 blocks
      0x80/0x81  SPINET 4 bpp              0x88/0x89  SPINET 5 bpp
      0x90/0x91  SPINET 6 bpp              0x98/0x99  SPINET 7 bpp
      0xa0/0xa1  SPINET 8 bpp              0xa8  SPINET RGB565 direct colour

    Bit 0 of the format byte means a transparent colour index byte follows.

    SPINET (indexed): header fmt,[transparent],b2,b3 where
      b2 = 1 F1 F0 M2 M1 M0 T2, b3 = T1 T0 Z2 Z1 Z0 W2 W1 W0
      cmax = M+1 (long literal width), cth = T+1 (short literal width),
      esc = Z+1 (escape run width), Wn = W+1 (run width), FF = flag layout.
    Stream (MSB first): k(3) 00000, then per 16x16 block:
      scan(2) p0(k+1) override-flags [3-bit data per overridden parameter]
      and tokens until 255 differences have been produced:
        00 run(Wn)      run==0 -> run(esc)
        01 s            repeat the last difference, negated if s
        10 tc(cth)      short two's complement literal difference
        11 tc(cmax)     long two's complement literal difference
    Pixel values wrap modulo 2^depth.  The repeat token is seeded with p0.

    SPINET RGB565: header fmt,b2,b3 then k(3) 00000 FF23(2) f1(6) f2(6) f3(6):
      planes 2-3 parameters cmax=(f1>>3)+1 cth=(f1&7)+1 esc=(f2>>3)+1 Wn=(f2&7)+1,
      k2 = f3>>3, block 0 scan = (f3>>1)&3, block 0 G msb = f3&1.
    Per block: [scan(2) Gmsb(1) for blocks > 0] G low(k bits) [flags for plane 1
    with the header parameters] R(k2+1 bits) B(k2+1 bits) [flags for planes 2-3]
    then the G (6-bit), R (5-bit) and B (5-bit) planes as above.  The colour
    0x0001 is transparent, as on the YGV629.

***************************************************************************/

#include "emu.h"
#include "ygv625.h"

#include "screen.h"

#include <algorithm>
#include <cmath>

#define LOG_REGS   (1U << 1)
#define LOG_SPRITE (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(YGV625, ygv625_device, "ygv625", "Yamaha YGV625 PVDC6R")


ygv625_device::ygv625_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, YGV625, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_cg(*this, finder_base::DUMMY_TAG),
	m_cg_read_data(0),
	m_cg_read_ready(false)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void ygv625_device::device_start()
{
	m_ram = std::make_unique<u16[]>(0x2000);
	std::fill_n(m_ram.get(), 0x2000, 0);
	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	m_bitmap.allocate(512, 512);
	m_bitmap.fill(0);

	// precompute the four 16x16 block scan orders
	for (u32 scan = 0; scan < 4; scan++)
		for (u32 i = 0; i < 256; i++)
		{
			u32 row, col;
			scan_position(scan, i, row, col);
			m_scan_tables[scan][i][0] = row;
			m_scan_tables[scan][i][1] = col;
		}

	save_pointer(NAME(m_ram), 0x2000);
	save_item(NAME(m_regs));
	save_item(NAME(m_cg_read_data));
	save_item(NAME(m_cg_read_ready));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void ygv625_device::device_reset()
{
	m_cg_read_ready = false;
	m_cg_read_data = 0;
}


//-------------------------------------------------
//  scan_position - row/column of the n-th pixel
//  of a 16x16 block for the given scan order
//-------------------------------------------------

void ygv625_device::scan_position(u32 scan, u32 index, u32 &row, u32 &col)
{
	switch (scan)
	{
	case 0: // rows, boustrophedon
	{
		row = index >> 4;
		col = index & 15;
		if (row & 1)
			col = 15 - col;
		break;
	}
	case 1: // columns, boustrophedon
	{
		col = index >> 4;
		row = index & 15;
		if (col & 1)
			row = 15 - row;
		break;
	}
	case 2: // 8x4 sub-blocks visited in row boustrophedon order, columns inside scanned vertically
	{
		u32 si = index >> 5;              // sub-block number 0..7
		u32 by = si >> 1;
		u32 bx = (by & 1) ? 1 - (si & 1) : (si & 1);
		u32 j = (index >> 2) & 7;         // column index within the sub-block
		u32 c = (by & 1) ? 7 - j : j;
		u32 r = index & 3;
		if ((j + si) & 1)
			r = 3 - r;
		row = by * 4 + r;
		col = bx * 8 + c;
		break;
	}
	default: // 4x8 sub-blocks visited in column boustrophedon order, rows inside scanned horizontally
	{
		u32 si = index >> 5;
		u32 bx = si >> 1;
		u32 by = (bx & 1) ? 1 - (si & 1) : (si & 1);
		u32 j = (index >> 2) & 7;         // row index within the sub-block
		u32 r = (bx & 1) ? 7 - j : j;
		u32 c = index & 3;
		if ((j + si) & 1)
			c = 3 - c;
		row = by * 8 + r;
		col = bx * 4 + c;
		break;
	}
	}
}


//-------------------------------------------------
//  bit_reader::read
//-------------------------------------------------

u32 ygv625_device::bit_reader::read(u32 bits)
{
	u32 value = 0;
	while (bits--)
	{
		u64 byte = m_pos >> 3;
		u32 bit = 0;
		if (byte < m_size)
			bit = (m_base[byte] >> (7 - (m_pos & 7))) & 1;
		value = (value << 1) | bit;
		m_pos++;
	}
	return value;
}


//-------------------------------------------------
//  read_overrides - per block parameter override
//  flags.  The parameters are grouped as
//  (cmax,cth) (esc,Wn); FF bit 1 splits cth off,
//  FF bit 0 splits Wn off, split groups are
//  appended after the two main groups.
//-------------------------------------------------

bool ygv625_device::read_overrides(bit_reader &br, plane_params &p, const plane_params &base, u32 flag_layout)
{
	p = base;

	u32 nflags = 2 + ((flag_layout >> 1) & 1) + (flag_layout & 1);
	u32 flags = br.read(nflags);

	// expand the flags into a mask of overridden parameters: bit 0 cmax, 1 cth, 2 esc, 3 Wn
	u32 over = 0;
	u32 shift = nflags - 1;
	auto take = [&flags, &shift]() { u32 f = (flags >> shift) & 1; shift--; return f; };
	if (take())
		over |= (flag_layout & 2) ? 0x1 : 0x3;
	if (take())
		over |= (flag_layout & 1) ? 0x4 : 0xc;
	if (flag_layout & 2)
		if (take())
			over |= 0x2;
	if (flag_layout & 1)
		if (take())
			over |= 0x8;

	if (over & 1) p.cmax = br.read(3) + 1;
	if (over & 2) p.cth = br.read(3) + 1;
	if (over & 4) p.esc = br.read(3) + 1;
	if (over & 8) p.wn = br.read(3) + 1;
	return true;
}


//-------------------------------------------------
//  decode_plane - decode 256 values of one plane
//  (one 16x16 block) in scan order
//-------------------------------------------------

bool ygv625_device::decode_plane(bit_reader &br, const plane_params &p, int p0, u32 mod, u16 *out)
{
	int cur = p0;
	int lastd = p0;
	u32 n = 1;
	out[0] = cur & (mod - 1);

	while (n < 256)
	{
		if (br.overrun())
			return false;

		u32 tok = br.read(2);
		if (tok == 0)
		{
			u32 run = br.read(p.wn);
			if (run == 0)
			{
				run = br.read(p.esc);
				if (run == 0)
					return false;
			}
			if (run > 256 - n)
				return false;
			for (u32 i = 0; i < run; i++)
				out[n++] = cur & (mod - 1);
		}
		else if (tok == 1)
		{
			int d = br.read(1) ? -lastd : lastd;
			cur += d;
			lastd = d;
			out[n++] = cur & (mod - 1);
		}
		else
		{
			u32 w = (tok == 2) ? p.cth : p.cmax;
			int v = br.read(w);
			if (v & (1 << (w - 1)))
				v -= 1 << w;
			cur += v;
			if (v)
				lastd = v;
			out[n++] = cur & (mod - 1);
		}
	}
	return true;
}


//-------------------------------------------------
//  decode_indexed - SPINET palette formats
//-------------------------------------------------

bool ygv625_device::decode_indexed(decoded_sprite &spr, u32 addr)
{
	const u8 fmt = cg_byte(addr);
	u32 pos = addr + 1;
	spr.has_transparent = BIT(fmt, 0);
	if (fmt & 1)
		spr.transparent = cg_byte(pos++); // transparent colour index

	const u8 b2 = cg_byte(pos);
	const u8 b3 = cg_byte(pos + 1);
	pos += 2;

	plane_params base;
	base.cmax = ((b2 >> 1) & 7) + 1;
	base.cth = (((b2 & 1) << 2) | (b3 >> 6)) + 1;
	base.esc = ((b3 >> 3) & 7) + 1;
	base.wn = (b3 & 7) + 1;
	const u32 flag_layout = (b2 >> 4) & 3;

	static const u8 depth_table[6] = { 4, 5, 6, 7, 8, 8 };
	spr.depth = depth_table[std::min<u32>((fmt >> 3) & 7, 5)];
	const u32 mod = 1 << spr.depth;

	bit_reader br(&m_cg[0], m_cg.length(), pos);
	const u32 k = br.read(3);
	br.read(5);

	const u32 bw = (spr.width + 15) / 16;
	const u32 bh = (spr.height + 15) / 16;
	spr.pixels.assign(bw * 16 * bh * 16, 0);

	u16 block[256];
	for (u32 bi = 0; bi < bw * bh; bi++)
	{
		const u32 scan = br.read(2);
		const int p0 = br.read(k + 1);
		plane_params p;
		read_overrides(br, p, base, flag_layout);
		if (!decode_plane(br, p, p0, mod, block))
		{
			LOGMASKED(LOG_SPRITE, "sprite %06x: decode error in block %u\n", addr, bi);
			return bi > 0;
		}
		const u32 bx = bi % bw, by = bi / bw;
		for (u32 i = 0; i < 256; i++)
		{
			const u32 row = m_scan_tables[scan][i][0], col = m_scan_tables[scan][i][1];
			spr.pixels[(by * 16 + row) * (bw * 16) + bx * 16 + col] = block[i];
		}
	}
	return true;
}


//-------------------------------------------------
//  decode_rgb - SPINET RGB565 direct colour
//-------------------------------------------------

bool ygv625_device::decode_rgb(decoded_sprite &spr, u32 addr)
{
	const u8 b2 = cg_byte(addr + 1);
	const u8 b3 = cg_byte(addr + 2);

	plane_params base1;
	base1.cmax = ((b2 >> 1) & 7) + 1;
	base1.cth = (((b2 & 1) << 2) | (b3 >> 6)) + 1;
	base1.esc = ((b3 >> 3) & 7) + 1;
	base1.wn = (b3 & 7) + 1;
	const u32 ff1 = (b2 >> 4) & 3;

	bit_reader br(&m_cg[0], m_cg.length(), addr + 3);
	const u32 k = br.read(3);
	br.read(5);
	const u32 ff23 = br.read(2);
	const u32 f1 = br.read(6);
	const u32 f2 = br.read(6);
	const u32 f3 = br.read(6);

	plane_params base23;
	base23.cmax = (f1 >> 3) + 1;
	base23.cth = (f1 & 7) + 1;
	base23.esc = (f2 >> 3) + 1;
	base23.wn = (f2 & 7) + 1;
	const u32 k2 = f3 >> 3;

	spr.depth = 16;
	const u32 bw = (spr.width + 15) / 16;
	const u32 bh = (spr.height + 15) / 16;
	spr.pixels.assign(bw * 16 * bh * 16, 0x0001);

	u16 g[256], r[256], b[256];
	for (u32 bi = 0; bi < bw * bh; bi++)
	{
		const u32 x3 = (bi == 0) ? (f3 & 7) : br.read(3);
		const u32 scan = x3 >> 1;
		const u32 glow = br.read(k);
		plane_params p1, p23;
		read_overrides(br, p1, base1, ff1);
		const int r0 = br.read(k2 + 1);
		const int b0 = (br.read(k2) << 1) | br.read(1);
		const int g0 = ((x3 & 1) << k) | glow;
		read_overrides(br, p23, base23, ff23);

		if (!decode_plane(br, p1, g0, 64, g) || !decode_plane(br, p23, r0, 32, r) || !decode_plane(br, p23, b0, 32, b))
		{
			LOGMASKED(LOG_SPRITE, "sprite %06x: RGB decode error in block %u\n", addr, bi);
			return bi > 0;
		}
		const u32 bx = bi % bw, by = bi / bw;
		for (u32 i = 0; i < 256; i++)
		{
			const u32 row = m_scan_tables[scan][i][0], col = m_scan_tables[scan][i][1];
			spr.pixels[(by * 16 + row) * (bw * 16) + bx * 16 + col] = (r[i] << 11) | (g[i] << 5) | b[i];
		}
	}
	return true;
}


//-------------------------------------------------
//  decode_raw - uncompressed 16x16 block data
//-------------------------------------------------

bool ygv625_device::decode_raw(decoded_sprite &spr, u32 addr, u32 bpp)
{
	spr.depth = bpp;
	spr.has_transparent = true;
	spr.transparent = cg_byte(addr + 1);
	const u32 bw = (spr.width + 15) / 16;
	const u32 bh = (spr.height + 15) / 16;
	spr.pixels.assign(bw * 16 * bh * 16, 0);

	u32 pos = addr + 2;
	for (u32 bi = 0; bi < bw * bh; bi++)
	{
		const u32 bx = bi % bw, by = bi / bw;
		for (u32 y = 0; y < 16; y++)
			for (u32 x = 0; x < 16; x++)
			{
				u8 v;
				if (bpp == 8)
					v = cg_byte(pos++);
				else
				{
					const u8 byte = cg_byte(pos + (x >> 1));
					v = (x & 1) ? (byte & 15) : (byte >> 4);
					if (x == 15)
						pos += 8;
				}
				spr.pixels[(by * 16 + y) * (bw * 16) + bx * 16 + x] = v;
			}
	}
	return true;
}


//-------------------------------------------------
//  decode_sprite
//-------------------------------------------------

bool ygv625_device::decode_sprite(decoded_sprite &spr, u32 addr)
{
	const u8 fmt = cg_byte(addr);
	spr.format = fmt;
	switch (fmt)
	{
	case 0x01: return decode_raw(spr, addr, 4);
	case 0x21: return decode_raw(spr, addr, 8);
	case 0x80: case 0x81: case 0x88: case 0x89: case 0x90: case 0x91:
	case 0x98: case 0x99: case 0xa0: case 0xa1:
		return decode_indexed(spr, addr);
	case 0xa8:
		return decode_rgb(spr, addr);
	default:
		LOGMASKED(LOG_SPRITE, "sprite %06x: unknown format %02x\n", addr, fmt);
		return false;
	}
}


//-------------------------------------------------
//  get_sprite - cached decode
//-------------------------------------------------

const ygv625_device::decoded_sprite &ygv625_device::get_sprite(u32 addr, u16 width, u16 height)
{
	const u64 key = (u64(addr) << 20) | (u64(width) << 10) | height;
	auto it = m_cache.find(key);
	if (it != m_cache.end())
		return it->second;

	if (m_cache.size() > 4096)
		m_cache.clear();

	decoded_sprite &spr = m_cache[key];
	spr.width = width;
	spr.height = height;
	spr.valid = decode_sprite(spr, addr);
	return spr;
}


//-------------------------------------------------
//  lookup_colour - palette lookup for indexed
//  sprites.  The palettes live in CG memory: the
//  five words at 0x3c06-0x3c0e give the base
//  address (in 32 byte units) of the 4, 5, 6, 7
//  and 8 bit per pixel palette tables, and each
//  table holds consecutive palettes of 2^depth
//  colours (the YGV629 uses the same addressing
//  for its internal colour palette table).
//-------------------------------------------------

rgb_t ygv625_device::lookup_colour(u32 palette, u32 depth, u32 index) const
{
	const u32 d = std::clamp<u32>(depth, 4, 8);

	// palette bit 8 selects the CPU loadable colour table in the chip
	// (0x3800-0x38ff, 128 colours) instead of CG memory; the BIOS uses it
	// for the red "ON" text of the I/O test
	if (palette & 0x100)
	{
		const u32 offset = (((palette & 0xff) << d) + index) & 0x7f;
		return rgb565(m_ram[0x1c00 + offset]);
	}

	const u32 base = u32(m_regs[0x03 + (d - 4)]) << 5;
	const u32 addr = base + (palette << (d + 1)) + (index << 1);
	return rgb565(cg_word(addr));
}


//-------------------------------------------------
//  draw_sprite
//-------------------------------------------------

void ygv625_device::draw_sprite(const decoded_sprite &spr, int cx, int cy, u32 palette, u32 zoomx, u32 zoomy, bool flipx, bool flipy, bool transparency)
{
	if (!spr.valid || spr.pixels.empty())
		return;

	const u32 stride = ((spr.width + 15) / 16) * 16;
	if (zoomx == 0) zoomx = 0x40;
	if (zoomy == 0) zoomy = 0x40;

	// displayed size
	const int dw = std::max<int>(1, (spr.width * zoomx + 0x20) / 0x40);
	const int dh = std::max<int>(1, (spr.height * zoomy + 0x20) / 0x40);
	const int x0 = cx - dw / 2;
	const int y0 = cy - dh / 2;

	// colour lookup table for indexed sprites
	rgb_t clut[256];
	if (spr.depth <= 8)
		for (u32 i = 0; i < (1u << spr.depth); i++)
			clut[i] = lookup_colour(palette, spr.depth, i);

	// transparent pixel value: the index given in the sprite header, or the
	// clear colour 0x0001 for direct colour sprites; -1 when the attribute
	// entry disables transparency
	int transparent = -1;
	if (transparency)
		transparent = (spr.depth == 16) ? 0x0001 : (spr.has_transparent ? spr.transparent : 0);

	const int xmax = m_bitmap.width(), ymax = m_bitmap.height();
	for (int dy = 0; dy < dh; dy++)
	{
		const int sy_dst = y0 + dy;
		if (sy_dst < 0 || sy_dst >= ymax)
			continue;
		int sy = (dy * spr.height) / dh;
		if (flipy) sy = spr.height - 1 - sy;
		const u16 *src = &spr.pixels[sy * stride];
		u32 *dst = &m_bitmap.pix(sy_dst);
		for (int dx = 0; dx < dw; dx++)
		{
			const int sx_dst = x0 + dx;
			if (sx_dst < 0 || sx_dst >= xmax)
				continue;
			int sx = (dx * spr.width) / dw;
			if (flipx) sx = spr.width - 1 - sx;
			const u16 v = src[sx];
			if (int(v) == transparent)
				continue;
			dst[sx_dst] = (spr.depth == 16) ? rgb565(v) : clut[v];
		}
	}
}


//-------------------------------------------------
//  draw_sprite_quad - map the sprite onto an
//  arbitrary quadrilateral (corners TL, TR, BR, BL
//  in screen coordinates), used for rotated and
//  perspective-distorted sprites
//-------------------------------------------------

void ygv625_device::draw_sprite_quad(const decoded_sprite &spr, const double (&qx)[4], const double (&qy)[4], u32 palette, bool flipx, bool flipy, bool transparency)
{
	if (!spr.valid || spr.pixels.empty())
		return;

	const u32 stride = ((spr.width + 15) / 16) * 16;

	rgb_t clut[256];
	if (spr.depth <= 8)
		for (u32 i = 0; i < (1u << spr.depth); i++)
			clut[i] = lookup_colour(palette, spr.depth, i);

	int transparent = -1;
	if (transparency)
		transparent = (spr.depth == 16) ? 0x0001 : (spr.has_transparent ? spr.transparent : 0);

	// P(s,t) = P0 + s*e + t*f + s*t*g with s, t in [0,1) across the texture
	const double ex = qx[1] - qx[0], ey = qy[1] - qy[0];
	const double fx = qx[3] - qx[0], fy = qy[3] - qy[0];
	const double gx = qx[0] - qx[1] + qx[2] - qx[3], gy = qy[0] - qy[1] + qy[2] - qy[3];
	const double area = ex * fy - ey * fx;
	if (std::abs(area) < 0.5 && std::abs(gx) + std::abs(gy) < 0.5)
		return;

	const int xmax = m_bitmap.width(), ymax = m_bitmap.height();
	const int bx0 = std::max<int>(0, int(std::floor(std::min({qx[0], qx[1], qx[2], qx[3]}))));
	const int bx1 = std::min<int>(xmax - 1, int(std::ceil(std::max({qx[0], qx[1], qx[2], qx[3]}))));
	const int by0 = std::max<int>(0, int(std::floor(std::min({qy[0], qy[1], qy[2], qy[3]}))));
	const int by1 = std::min<int>(ymax - 1, int(std::ceil(std::max({qy[0], qy[1], qy[2], qy[3]}))));

	const double k2 = gx * fy - gy * fx;
	const double kef = ex * fy - ey * fx;

	for (int py = by0; py <= by1; py++)
	{
		u32 *dst = &m_bitmap.pix(py);
		for (int px = bx0; px <= bx1; px++)
		{
			const double hx = (px + 0.5) - qx[0], hy = (py + 0.5) - qy[0];
			const double k1 = kef + (hx * gy - hy * gx);
			const double k0 = hx * ey - hy * ex;
			// solve k2*t^2 + k1*t + k0 = 0 for t, then s from the row equation; the
			// quads are usually nearly parallelograms (k2 ~ 0), so use the stable
			// form of the quadratic formula to keep the small root accurate
			double s = -1.0, t = -1.0;
			bool found = false;
			auto solve_s = [&](double tt, double &ss) -> bool
			{
				const double dx = ex + gx * tt, dy = ey + gy * tt;
				if (std::abs(dx) < 1e-9 && std::abs(dy) < 1e-9)
					return false;
				ss = (std::abs(dx) >= std::abs(dy)) ? (hx - fx * tt) / dx : (hy - fy * tt) / dy;
				return (ss >= 0.0 && ss < 1.0 && tt >= 0.0 && tt < 1.0);
			};
			if (k2 == 0.0)
			{
				if (k1 == 0.0)
					continue;
				t = -k0 / k1;
				found = solve_s(t, s);
			}
			else
			{
				const double disc = k1 * k1 - 4.0 * k0 * k2;
				if (disc < 0.0)
					continue;
				const double w = std::sqrt(disc);
				const double q = -0.5 * (k1 + ((k1 >= 0.0) ? w : -w));
				// root 1: q / k2 (the large one when k2 is small), root 2: k0 / q
				if (q != 0.0)
				{
					t = k0 / q;
					found = solve_s(t, s);
				}
				if (!found)
				{
					t = q / k2;
					found = solve_s(t, s);
				}
			}
			if (!found)
				continue;
			if (s < 0.0 || s >= 1.0 || t < 0.0 || t >= 1.0)
				continue;

			int sx = std::min<int>(spr.width - 1, int(s * spr.width));
			int sy = std::min<int>(spr.height - 1, int(t * spr.height));
			if (flipx) sx = spr.width - 1 - sx;
			if (flipy) sy = spr.height - 1 - sy;
			const u16 v = spr.pixels[sy * stride + sx];
			if (int(v) == transparent)
				continue;
			dst[px] = (spr.depth == 16) ? rgb565(v) : clut[v];
		}
	}
}


//-------------------------------------------------
//  render_frame - walk the attribute table
//-------------------------------------------------

void ygv625_device::render_frame()
{
	m_bitmap.fill(rgb_t::black());

	for (u32 entry = 0; entry < 512; entry++)
	{
		const u16 *a = &m_ram[entry * 8];
		const u16 flags = a[7];
		if ((flags & 0x3000) == 0x1000)
		{
			const u32 palette = (a[0] & 0xff) | ((a[0] & 0x0400) ? 0x100 : 0);
			const u32 width = ((a[1] & 0xff) | ((a[5] & 0x1000) ? 0x100 : 0)) + 1;
			const u32 height = ((a[1] >> 8) | ((a[4] & 0x1000) ? 0x100 : 0)) + 1;
			const u32 addr = (u32(a[2]) << 16) | a[3];
			int y = a[4] & 0xfff;
			int x = a[5] & 0xfff;
			if (y & 0x800) y -= 0x1000;
			if (x & 0x800) x -= 0x1000;

			if (addr < m_cg.length())
			{
				const decoded_sprite &spr = get_sprite(addr, width, height);
				if (flags & 0x8000)
				{
					// quadrilateral mode: +12 bits 7..0 index a 16 byte entry of the
					// corner table at 0x2000.  Each corner is two words, Y (positive
					// upwards) then X, 11-bit two's complement pixel offsets from the
					// centre position, in the order bottom-left, top-left, top-right,
					// bottom-right.
					const u16 *q = &m_ram[0x1000 + (a[6] & 0xff) * 4];
					double sx[4], sy[4];
					for (int c = 0; c < 4; c++)
					{
						int cy = q[c * 2] & 0x7ff, cx = q[c * 2 + 1] & 0x7ff;
						if (cx & 0x400) cx -= 0x800;
						if (cy & 0x400) cy -= 0x800;
						sx[c] = x + cx;
						sy[c] = y - cy;
					}
					// draw_sprite_quad wants TL, TR, BR, BL
					const double qx[4] = { sx[1], sx[2], sx[3], sx[0] };
					const double qy[4] = { sy[1], sy[2], sy[3], sy[0] };
					draw_sprite_quad(spr, qx, qy, palette, BIT(a[5], 15), BIT(a[4], 15), BIT(a[0], 11));
				}
				else
				{
					draw_sprite(spr, x, y, palette, a[6] & 0xff, a[6] >> 8, BIT(a[5], 15), BIT(a[4], 15), BIT(a[0], 11));
				}
			}
		}
		if (flags & 0x4000)
			break;
	}
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u16 ygv625_device::read(offs_t offset, u16 mem_mask)
{
	if (offset < 0x1e00)
		return m_ram[offset];

	const u32 reg = offset & 0x1ff;
	u16 data = m_regs[reg];
	switch (reg)
	{
	case 0x26: // 0x3c4c status
		data = (m_regs[reg] & ~0x4109) | (screen().vblank() ? 1 : 0) | (m_cg_read_ready ? 0x4000 : 0);
		break;

	case 0x27: // 0x3c4e control, bit 5 self clears
		data = m_regs[reg] & ~0x0020;
		break;

	case 0x28: // 0x3c50
		data = m_regs[reg] & 0x7fff;
		break;

	case 0x36: // 0x3c6c CG read address high / start, bit 15 reads back clear once done
		data = m_regs[reg] & 0x7fff;
		break;

	case 0x38: // 0x3c70 CG read data
		data = m_cg_read_data;
		break;

	default:
		break;
	}
	return data;
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void ygv625_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset < 0x1e00)
	{
		COMBINE_DATA(&m_ram[offset]);
		return;
	}

	const u32 reg = offset & 0x1ff;
	COMBINE_DATA(&m_regs[reg]);
	LOGMASKED(LOG_REGS, "%s: reg %04x = %04x\n", machine().describe_context(), 0x3c00 + reg * 2, m_regs[reg]);

	switch (reg)
	{
	case 0x27: // 0x3c4e control
		if (data & 0x0020)
			render_frame();
		break;

	case 0x36: // 0x3c6c CG read: bit 15 starts the access
		if (data & 0x8000)
		{
			const u32 addr = (u32(m_regs[0x36] & 0x07ff) << 16) | m_regs[0x37];
			m_cg_read_data = cg_word(addr);
			m_cg_read_ready = true;
			m_regs[0x36] &= 0x7fff;
		}
		break;

	case 0x38:
		// TODO: CG memory writes (flash programming through the chip)
		break;

	default:
		break;
	}
}


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

u32 ygv625_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
