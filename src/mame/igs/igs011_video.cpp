// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/*

IGS011 blitter with protection

implementation based from igs/igs011.cpp, by Luca Elia/Olivier Galibert.

TODO:
- realistic blitter timings
- blitter busy flag

*/

#include "emu.h"
#include "igs011_video.h"

#define LOG_BLITTER (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_BLITTER)
#include "logmacro.h"

void igs011_device::map(address_map &map)
{
	// mapped at 0x858000 or 0xa58000
	map(0x0000, 0x0001).w(FUNC(igs011_device::blit_x_w));
	map(0x0800, 0x0801).w(FUNC(igs011_device::blit_y_w));
	map(0x1000, 0x1001).w(FUNC(igs011_device::blit_w_w));
	map(0x1800, 0x1801).w(FUNC(igs011_device::blit_h_w));
	map(0x2000, 0x2001).w(FUNC(igs011_device::blit_gfx_lo_w));
	map(0x2800, 0x2801).w(FUNC(igs011_device::blit_gfx_hi_w));
	map(0x3000, 0x3001).w(FUNC(igs011_device::blit_flags_w));
	map(0x3800, 0x3801).w(FUNC(igs011_device::blit_pen_w));
	map(0x4000, 0x4001).w(FUNC(igs011_device::blit_depth_w));
}

DEFINE_DEVICE_TYPE(IGS011, igs011_device, "igs011", "IGS011 Blitter + Protection Device")

igs011_device::igs011_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS011, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_blitter()
	, m_priority(0)
	, m_blitter_pen_hi(0)
	, m_prot(0)
	, m_prot_swap(0)
	, m_prot_addr(0)
	, m_prev_prot_addr(-1)
	, m_layer_ram(*this, "layer%u_ram", 0U, 512U * 256U, ENDIANNESS_BIG)
	, m_priority_ram(*this, "priority_ram", 0x1000, ENDIANNESS_BIG)
	, m_gfx(*this, DEVICE_SELF)
	, m_gfx_hi(*this, "gfx_hi")
	, m_host_space(*this, finder_base::DUMMY_TAG, -1, 16)
	, m_restore_space_cb(*this)
{
}

void igs011_device::device_start()
{
	m_blitter_pen_hi = 0;

	save_item(NAME(m_blitter.x));
	save_item(NAME(m_blitter.y));
	save_item(NAME(m_blitter.w));
	save_item(NAME(m_blitter.h));
	save_item(NAME(m_blitter.gfx_lo));
	save_item(NAME(m_blitter.gfx_hi));
	save_item(NAME(m_blitter.depth));
	save_item(NAME(m_blitter.pen));
	save_item(NAME(m_blitter.flags));

	save_item(NAME(m_priority));
	save_item(NAME(m_blitter_pen_hi));

	save_item(NAME(m_prot));
	save_item(NAME(m_prot_swap));
	save_item(NAME(m_prot_addr));
	save_item(NAME(m_prev_prot_addr));
}

void igs011_device::device_post_load()
{
	prot_addr_change_w(m_prev_prot_addr);
}

/***************************************************************************

    Video

    There are 8 non scrolling layers as big as the screen (512 x 256).
    Each layer has 256 colors and its own palette.

    There are 8 priority codes with RAM associated to each (8 x 256 values).
    For each screen position, to determine which pixel to display, the video
    chip associates a bit to the opacity of that pixel for each layer
    (1 = transparent) to form an address into the selected priority RAM.
    The value at that address (0-7) is the topmost layer.

***************************************************************************/

void igs011_device::blitter_pen_hi_w(u8 data)
{
	m_blitter_pen_hi = data & 7;
}

void igs011_device::priority_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_priority);

//  logerror("%s: priority = %04x\n", machine().describe_context(), m_priority);

	if (data & ~0x7)
		logerror("%s: warning, unknown bits written to priority = %04x & %04x\n", machine().describe_context(), m_priority, mem_mask);
}

u16 igs011_device::priority_ram_r(offs_t offset)
{
	return m_priority_ram[offset];
}

void igs011_device::priority_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_priority_ram[offset]);
}

int igs011_device::blitter_busy_r()
{
	return 0; // TODO
}

u32 igs011_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#ifdef MAME_DEBUG
	int layer_enable = -1;
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  mask |= 0x01;
		if (machine().input().code_pressed(KEYCODE_W))  mask |= 0x02;
		if (machine().input().code_pressed(KEYCODE_E))  mask |= 0x04;
		if (machine().input().code_pressed(KEYCODE_R))  mask |= 0x08;
		if (machine().input().code_pressed(KEYCODE_A))  mask |= 0x10;
		if (machine().input().code_pressed(KEYCODE_S))  mask |= 0x20;
		if (machine().input().code_pressed(KEYCODE_D))  mask |= 0x40;
		if (machine().input().code_pressed(KEYCODE_F))  mask |= 0x80;
		if (mask)   layer_enable &= mask;
	}
#endif

	u16 const *const pri_ram = &m_priority_ram[(m_priority & 7) * 512/2];
	u32 const hibpp_layers = std::min<u32>(4 - (m_blitter.depth & 0x07), std::size(m_layer_ram));

	if (BIT(m_blitter.depth, 4))
	{
		// video output disabled
		u16 const pri = pri_ram[0xff] & 7;
		bitmap.fill((pri << 8) | 0xff, cliprect);
		return 0;
	}

	u8 layerpix[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int const scr_addr = (y << 9) | x;
			int pri_addr = 0xff;

			int l = 0;
			u32 i = 0;
			while (hibpp_layers > i)
			{
				layerpix[l] = m_layer_ram[i++][scr_addr];
				if (layerpix[l] != 0xff)
#ifdef MAME_DEBUG
					if (BIT(layer_enable, l))
#endif
						pri_addr &= ~(1 << l);
				++l;
			}
			while (std::size(m_layer_ram) > i)
			{
				u8 const pixdata = m_layer_ram[i++][scr_addr];
				layerpix[l] = pixdata & 0x0f;
				if (layerpix[l] != 0x0f)
				{
#ifdef MAME_DEBUG
					if (BIT(layer_enable, l))
#endif
						pri_addr &= ~(1 << l);
				}
				++l;
				layerpix[l] = (pixdata >> 4) & 0x0f;
				if (layerpix[l] != 0x0f)
				{
#ifdef MAME_DEBUG
					if (BIT(layer_enable, l))
#endif
						pri_addr &= ~(1 << l);
				}
				++l;
			}

			u16 const pri = pri_ram[pri_addr] & 7;
#ifdef MAME_DEBUG
			if ((layer_enable != -1) && (pri_addr == 0xff))
				bitmap.pix(y, x) = palette().black_pen();
			else
#endif
				bitmap.pix(y, x) = layerpix[pri] | (pri << 8);
		}
	}
	return 0;
}

/***************************************************************************

    In addition to the blitter, the CPU can directly access video RAM.
    There are four buffers of 128 KiB each, organised as 256 rows by 512
    columns.  Each buffer can be treated as a single 8-bit layer or two
    4-bit layers.  The most and least significant bits of the offset
    select the buffer, and the remaining bits address a byte within the
    buffer.

    - Buffer 0 is accessed at 0x300000, 0x300004, 0x300008, etc.
      (offset 0x00000, 0x00002, 0x00004, etc.)
    - Buffer 1 is accessed at 0x300002, 0x300006, 0x30000a, etc.
      (offset 0x00001, 0x00003, 0x00005, etc.)
    - Buffer 2 is accessed at 0x380000, 0x380004, 0x380008, etc.
      (offset 0x40000, 0x40002, 0x40004, etc.)
    - Buffer 3 is accessed at 0x380002, 0x380006, 0x38000a, etc.
      (offset 0x40001, 0x40003, 0x40005, etc.)

***************************************************************************/

u8 igs011_device::layers_r(offs_t offset)
{
	u32 const layer = bitswap<2>(offset, 18, 0);
	offs_t const byteoffset = (offset >> 1) & 0x1ffff;
	return m_layer_ram[layer][byteoffset];
}

void igs011_device::layers_w(offs_t offset, u8 data)
{
	u32 const layer = bitswap<2>(offset, 18, 0);
	offs_t const byteoffset = (offset >> 1) & 0x1ffff;
	m_layer_ram[layer][byteoffset] = data;
}

/***************************************************************************

    Blitter

***************************************************************************/


void igs011_device::blit_x_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.x);
}

void igs011_device::blit_y_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.y);
}

void igs011_device::blit_gfx_lo_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.gfx_lo);
}

void igs011_device::blit_gfx_hi_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.gfx_hi);
}

void igs011_device::blit_w_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.w);
}

void igs011_device::blit_h_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.h);
}

void igs011_device::blit_depth_w(offs_t offset, u16 data, u16 mem_mask)
{
	// ---X ---- disable video output
	// ---- -XXX layer depth configuration expressed as number of buffers used as dual 4bpp layers
	COMBINE_DATA(&m_blitter.depth);
}

void igs011_device::blit_pen_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.pen);
}


void igs011_device::blit_flags_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_blitter.flags);

	LOGMASKED(LOG_BLITTER, "%s: blit x %03x, y %03x, w %03x, h %03x, gfx %03x%04x, depth %02x, pen %02x, flags %03x\n",
			machine().describe_context(),
			m_blitter.x, m_blitter.y, m_blitter.w, m_blitter.h,
			m_blitter.gfx_hi, m_blitter.gfx_lo,
			m_blitter.depth, m_blitter.pen, m_blitter.flags);

	u32 const layer   = m_blitter.flags & 0x0007;
	bool const opaque = BIT(~m_blitter.flags,  3);
	bool const clear  = BIT( m_blitter.flags,  4);
	bool const flipx  = BIT( m_blitter.flags,  5);
	bool const flipy  = BIT( m_blitter.flags,  6);
	bool const blit   = BIT( m_blitter.flags, 10);

	if (!blit)
		return;

	u8 const pen_hi = (m_blitter_pen_hi & 0x07) << 5;
	u32 const hibpp_layers = 4 - (m_blitter.depth & 0x07);
	bool const dst4 = layer >= hibpp_layers;
	bool const src4 = dst4 || (m_blitter.gfx_hi & 0x80); // see lhb2
	u32 const shift = dst4 ? (((layer - hibpp_layers) & 0x01) << 2) : 0;
	u8 const mask = dst4 ? (0xf0 >> shift) : 0x00;
	u32 const buffer = dst4 ? (hibpp_layers + ((layer - hibpp_layers) >> 1)) : layer;

	if (std::size(m_layer_ram) <= buffer)
	{
		logerror("%s: layer %u out of range depth %02x (%u 8-bit layers)\n", machine().describe_context(), layer, m_blitter.depth, hibpp_layers);
		return;
	}

	auto &dest = m_layer_ram[buffer];

	// pixel source address
	u32 z = (u32(m_blitter.gfx_hi & 0x7f) << 16) | m_blitter.gfx_lo;

	u8 const clear_pen = src4 ? (m_blitter.pen | 0xf0) : m_blitter.pen;

	u8 trans_pen;
	if (src4)
	{
		z <<= 1;
		if (m_gfx_hi && (m_blitter.gfx_hi & 0x80)) trans_pen = 0x1f;   // lhb2
		else                                       trans_pen = 0x0f;
	}
	else
	{
		if (m_gfx_hi) trans_pen = 0x1f;   // vbowl
		else          trans_pen = 0xff;
	}

	int const xstart = util::sext(m_blitter.x, 10);
	int const ystart = util::sext(m_blitter.y, 9);
	int const xsize = (m_blitter.w & 0x1ff) + 1;
	int const ysize = (m_blitter.h & 0x0ff) + 1;
	int const xend = flipx ? (xstart - xsize) : (xstart + xsize);
	int const yend = flipy ? (ystart - ysize) : (ystart + ysize);
	int const xinc = flipx ? -1 : 1;
	int const yinc = flipy ? -1 : 1;

	rectangle const clip(0, 512 - 1, 0, 256 - 1);
	for (int y = ystart; y != yend; y += yinc)
	{
		for (int x = xstart; x != xend; x += xinc)
		{
			if (clip.contains(x, y))
			{
				// fetch the pixel
				u8 pen = 0;
				if (!clear)
				{
					if (src4)
						pen = (m_gfx[(z >> 1) % m_gfx.length()] >> (BIT(z, 0) << 2)) & 0x0f;
					else
						pen = m_gfx[z % m_gfx.length()];

					if (m_gfx_hi)
						pen = (pen & 0x0f) | (BIT(m_gfx_hi[(z >> 3) % m_gfx_hi.length()], z & 0x07) << 4);
				}

				// plot it
				if (clear || (pen != trans_pen) || opaque)
				{
					u8 const val = clear ? clear_pen : (pen != trans_pen) ? (pen | pen_hi) : 0xff;
					u8 &destbyte = dest[(y << 9) | x];
					if (dst4)
						destbyte = (destbyte & mask) | ((val & 0x0f) << shift);
					else
						destbyte = val;
				}
			}
			++z;
		}
	}
}

/***************************************************************************

    Gfx Decryption

***************************************************************************/


void igs011_device::lhb2_gfx_decrypt()
{
	u32 const rom_size = 0x200000;
	u8 *src = m_gfx.target();
	std::unique_ptr<u8 []> result_data(new u8[rom_size]);

	for (int i = 0; i < rom_size; i++)
		result_data[i] = src[bitswap<24>(i, 23,22,21,20, 19, 17,16,15, 13,12, 10,9,8,7,6,5,4, 2,1, 3, 11, 14, 18, 0)];

	memcpy(src, &result_data[0], rom_size);
}

void igs011_device::drgnwrld_gfx_decrypt()
{
	u32 const rom_size = 0x400000;
	u8 *src = m_gfx.target();
	std::unique_ptr<u8 []> result_data(new u8[rom_size]);

	for (int i = 0; i < rom_size; i++)
		result_data[i] = src[bitswap<24>(i, 23,22,21,20,19,18,17,16,15, 12, 13, 14, 11,10,9,8,7,6,5,4,3,2,1,0)];

	memcpy(src, &result_data[0], rom_size);
}

void igs011_device::vbowl_gfx_unpack()
{
	u8 *gfx = m_gfx.target();
	for (int i = 0x400000 - 1; i >= 0; i--)
	{
		gfx[i * 2 + 1] = (gfx[i] & 0xf0) >> 4;
		gfx[i * 2 + 0] = (gfx[i] & 0x0f) >> 0;
	}
}


/***************************************************************************

    Protection ("ASIC11 CHECK PORT ERROR")

    The chip holds an internal value, a buffered value and an address base register.
    The address base register determines where the protection device is mapped in memory
    (0x00000-0xffff0), has itself a fixed address, and writes to it reset the state.
    The internal and buffered value are manipulated by issuing commands, where
    each command is assigned a specific offset, and is triggered by writing a specific
    byte value to that offset:

    Offs.   R/W     Result
    0         W     COPY: copy buffer to value
    2         W     INC:  increment value
    4         W     DEC:  decrement value
    6         W     SWAP: write bitswap1(value) to buffer
    8       R       READ: read bitswap2(value). Only 2 bits are checked (bitmask 0x24).

***************************************************************************/


void igs011_device::prot_w(offs_t offset, u8 data)
{
	offset *= 2;

	switch (offset)
	{
		case 0: // COPY
			if ((data & 0xff) == 0x33)
			{
				m_prot = m_prot_swap;
				return;
			}
			break;

		case 2: // INC
			if ((data & 0xff) == 0xff)
			{
				m_prot++;
				return;
			}
			break;

		case 4: // DEC
			if ((data & 0xff) == 0xaa)
			{
				m_prot--;
				return;
			}
			break;

		case 6: // SWAP
			if ((data & 0xff) == 0x55)
			{
				// b1 . (b2|b3) . b2 . (b0&b3)
				u8 const x = m_prot;
				m_prot_swap = (BIT(x, 1) << 3) | ((BIT(x, 2) | BIT(x, 3)) << 2) | (BIT(x, 2) << 1) | (BIT(x, 0) & BIT(x, 3));
				return;
			}
			break;
	}

	logerror("%s: warning, unknown prot_w( %04x, %02x )\n", machine().describe_context(), offset, data);
}

u16 igs011_device::prot_r()
{
	// !(b1&b2) . 0 . 0 . (b0^b3) . 0 . 0
	u8 const x = m_prot;
	return (((BIT(x, 1) & BIT(x, 2)) ^ 1) << 5) | ((BIT(x, 0) ^ BIT(x, 3)) << 2);
}

void igs011_device::prot_addr_change_w(s32 data)
{
	if (m_prev_prot_addr != -1)
		m_restore_space_cb(m_prot_addr);
	if (data != -1)
	{
		m_prot_addr = (data << 4) ^ 0x8340;
		prot_mem_range_set();
	}
	m_prev_prot_addr = data;
}

void igs011_device::prot_addr_w(u16 data)
{
	m_prot = 0x00;
	m_prot_swap = 0x00;
	if (m_prev_prot_addr != data)
		prot_addr_change_w(data);
}

void igs011_device::prot_mem_range_set()
{
	// Add protection memory range
	m_host_space->install_write_handler(m_prot_addr + 0, m_prot_addr + 7, write8sm_delegate(*this, FUNC(igs011_device::prot_w)), 0xff00);
	m_host_space->install_read_handler (m_prot_addr + 8, m_prot_addr + 9, read16smo_delegate(*this, FUNC(igs011_device::prot_r)));
}
/*
u16 igs011_device::prot_fake_r(offs_t offset)
{
    switch (offset)
    {
        case 0: return m_prot;
        case 1: return m_prot_swap;
    }
    return 0;
}
*/
