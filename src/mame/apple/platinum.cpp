// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple Platinum memory controller and video
    Emulation by R. Belmont

    This is a memory controller for 60x PowerPC chips and a DAFB derived video controller
    all in one ASIC.
*/

#include "emu.h"
#include "platinum.h"

#include "endianness.h"

#define LOG_MONSENSE    (1U << 1)
#define LOG_RAMDAC      (1U << 2)
#define LOG_ACCEL       (1U << 3)

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PLATINUM, platinum_device, "macplatinum", "Apple Platinum memory/video")

static constexpr u32 ACCEL_CMD_ROP_MASK     = 0x0000'0007;
static constexpr u32 ACCEL_CMD_PATTERN      = 0x0000'0008;
static constexpr u32 ACCEL_CMD_FILL         = 0x0001'0000;
static constexpr u32 ACCEL_CMD_COLORIZE     = 0x0002'0000;
static constexpr u32 ACCEL_CMD_EXPAND       = 0x0004'0000;
static constexpr u32 ACCEL_CMD_INT_ENABLE   = 0x4000'0000;

static constexpr u32 ACCEL_STATUS_GO        = 0x0000'0001;
static constexpr u32 ACCEL_STATUS_GO_MULTI  = 0x0000'0002;
static constexpr u32 ACCEL_STATUS_INT       = 0x4000'0000;
static constexpr u32 ACCEL_STATUS_BUSY      = 0x8000'0000;

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void platinum_device::map(address_map &map)
{
	map(0x00000000, 0x000000ff).rw(FUNC(platinum_device::platinum_r), FUNC(platinum_device::platinum_w));
	map(0x00000100, 0x000001ff).rw(FUNC(platinum_device::dafb_r), FUNC(platinum_device::dafb_w));
	map(0x00000200, 0x000003ff).rw(FUNC(platinum_device::swatch_r), FUNC(platinum_device::swatch_w));
	map(0x00000400, 0x000004ff).rw(FUNC(platinum_device::accel_r), FUNC(platinum_device::accel_w));
}

platinum_device::platinum_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	dafb_base(mconfig, PLATINUM, tag, owner, clock),
	m_maincpu_space(nullptr),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_accel_status(0)
{
	std::fill(std::begin(m_accel_regs), std::end(m_accel_regs), 0);
	m_accel_regs[ACCEL_SYSCONFIG] = 1;  // big-endian bus
	std::fill(std::begin(m_cursor_clut), std::end(m_cursor_clut), 0);
	std::fill(std::begin(m_cursor_rgb), std::end(m_cursor_rgb), 0);
	std::fill(std::begin(m_mem_regs), std::end(m_mem_regs), 0);
	std::fill(std::begin(m_dacula_regs), std::end(m_dacula_regs), 0);
}

void platinum_device::device_start()
{
	m_vram_size = 0x400000;     // Platinum supports up to 4 MB of VRAM
	m_dafb_version = 3;
	m_reg_shift = 2;            // the register file is spaced 16 bytes apart on the PowerPC bus

	dafb_base::device_start();

	m_accel_timer = timer_alloc(FUNC(platinum_device::accel_done), this);
	m_accel_timer->adjust(attotime::never);

	save_item(NAME(m_mem_regs));
	save_item(NAME(m_dacula_regs));
	save_item(NAME(m_accel_regs));
	save_item(NAME(m_accel_status));
	save_item(NAME(m_cursor_clut));
	save_item(NAME(m_cursor_rgb));
}

int platinum_device::clock_divider() const
{
	return 1 << (((m_dacula_regs[DAC_CONTROL] >> 6) + 1) & 3);
}

u32 platinum_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[8]) + (m_base & 0x3f'ffe0);
	const pen_t *pens = m_palette->pens();

	if (BIT(m_swatch_mode, 0))
	{
		return 0;
	}

	// if interlace is enabled, the stride is double it's actual value
	const u32 stride = BIT(m_config, 2) ? (m_stride >> 1) : m_stride;

	switch ((m_dacula_regs[DAC_CONTROL] >> 2) & 3)
	{
		case 0: // 8bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];
					*scanline++ = pens[pixels];
				}
			}
		}
		break;

		case 1: // 16bpp x555
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres; x++)
				{
					u16 const pixels = (vram8[(y * stride) + (x << 1)] << 8) | vram8[(y * stride) + (x << 1) + 1];
					*scanline++ = rgb_t(((pixels >> 10) & 0x1f) << 3, ((pixels >> 5) & 0x1f) << 3, (pixels & 0x1f) << 3);
				}
			}
			break;

		case 2: // 24 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				u32 const *base = &m_vram[(y * (stride/4)) + (m_base/4) + 4];
				for (int x = 0; x < m_hres; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}

	// overlay the hardware cursor if DACula has it enabled
	if (BIT(m_dacula_regs[DAC_CONTROL], 1))
	{
		draw_hw_cursor(bitmap, m_base + 0x10, stride);
	}

	return 0;
}

u32 platinum_device::platinum_r(offs_t offset)
{
	u32 retval = m_mem_regs[offset >> 2];

	switch (offset >> 2)
	{
		case 0: // ID register
			retval = 0x3001'8b00; // PowerMac 7200 ID
			break;

		case 1: // Platinum version and vendor
			retval = 0x3000'0000; // Version 3, vendor 0 is VLSI Technology
			break;
	}
//  printf("Platinum read %08x @ %x (reg %x)\n", retval, offset << 2, offset >> 2);
	return retval;
}

void platinum_device::platinum_w(offs_t offset, u32 data)
{
//  printf("Platinum write %08x @ %x (reg %x)\n", data, offset << 2, offset >> 2);
	m_mem_regs[offset >> 2] = data;
}

u32 platinum_device::dafb_r(offs_t offset)
{
//  printf("DAFB read %08x @ %x (reg %x) (%s)\n", m_mem_regs[offset >> 2], offset << 2, offset >> 2, machine().describe_context().c_str());
	switch (offset >> 2)
	{
		case 0: // framebuffer base
			return m_base;

		case 2: // framebuffer stride, in 32-bit words
			return m_stride;

		case 3: // timing control
			return m_timing_control;

		case 4: // DAFB config
			return m_config;

		// 5 = config 2
		// 6 = page control

		case 7:  // inverse of monitor sense
			return read_monitor_sense();

		// 8 = reset
		// 9 = double buffer control
		// A = test
		// B = VRAM refresh timing
	}

	return 0;
}

void platinum_device::dafb_w(offs_t offset, u32 data)
{
//  printf("DAFB write %08x @ %x (reg %x) (%s)\n", data, offset << 2, offset >> 2, machine().describe_context().c_str());
	switch (offset >> 2)
	{
		case 0: // base
			m_base = data & 0x003f'ffff;
			LOG("base: wrote %08x => %08x\n", data, m_base);
			break;

		case 2:
			m_stride = data;    // stride in 32-bit words
			LOG("Stride = %d\n", m_stride);
			break;

		case 3:   // clock configuration
			m_timing_control = data;
			LOG("Clock configuration = %08x\n", data);
			recalc_mode();
			break;

		case 4: // configuration
			LOG("DAFB config = %08x\n", data);
			m_config = data;
			break;

		case 7: // drive monitor sense lines. 0=drive to value in bit 0 of TEST, 1=tri-state
			m_monitor_id = (data & 0x7) ^ 7;
			LOGMASKED(LOG_MONSENSE, "%x to sense drive\n", data & 0xf);
			break;
	}
}

TIMER_CALLBACK_MEMBER(platinum_device::accel_done)
{
	m_accel_status = ACCEL_STATUS_INT;
}

// The dreaded QuickDraw accelerator
u32 platinum_device::accel_r(offs_t offset, u32 mem_mask)
{
	const u32 reg = offset >> 2;

	switch (reg)
	{
		case ACCEL_STATUS:
			return m_accel_status;

		case ACCEL_SYSCONFIG:
			return m_accel_regs[ACCEL_SYSCONFIG];
	}

	if (reg < ACCEL_MAX_REG)
	{
		return m_accel_regs[reg];
	}
	return 0;
}

void platinum_device::accel_w(offs_t offset, u32 data, u32 mem_mask)
{
	const u32 reg = offset >> 2;
	LOGMASKED(LOG_ACCEL, "accel: %08x to reg %x (mask %08x)\n", data, reg, mem_mask);

	switch (reg)
	{
		case ACCEL_STATUS:
			if (data & ACCEL_STATUS_GO_MULTI)
			{
				logerror("%s: accelerator Go Multiple is not implemented in Platinum\n", tag());
			}
			if (data & ACCEL_STATUS_GO)
			{
				// the memory effects happen immediately, but busy is held
				// for an arbitrary plausible duration so polling works.
				accel_execute();
				m_accel_status = ACCEL_STATUS_BUSY;
				const u32 pixels = (m_accel_regs[ACCEL_DSTSIZE] >> 16) * (m_accel_regs[ACCEL_DSTSIZE] & 0xffff);
				m_accel_timer->adjust(attotime::from_nsec(500 + (pixels * 20)));
			}
			else
			{
				m_accel_status = data & ACCEL_STATUS_INT;
			}
			break;

		default:
			if (reg < ACCEL_MAX_REG)
			{
				m_accel_regs[reg] = data;
			}
			break;
	}
}

//  Read/write a pixel by byte address.  Can be VRAM or system RAM and may not be aligned.
u32 platinum_device::accel_read_pixel(u32 addr, int bpp)
{
	// is it VRAM?
	if (BIT(addr, 31))
	{
		auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
		addr &= (m_vram_size - 1);
		u32 data = 0;
		for (int i = 0; i < bpp; i++)
		{
			data = (data << 8) | vram8[addr + i];
		}
		return data;
	}

	u32 data = 0;
	for (int i = 0; i < bpp; i++)
	{
		data = (data << 8) | m_maincpu_space->read_byte(addr + i);
	}
	return data;
}

void platinum_device::accel_write_pixel(u32 addr, int bpp, u32 data)
{
	if (BIT(addr, 31))
	{
		auto const vram8 = util::big_endian_cast<u8>(&m_vram[0]);
		addr &= (m_vram_size - 1);
		for (int i = bpp - 1; i >= 0; i--)
		{
			vram8[addr + i] = data & 0xff;
			data >>= 8;
		}
		return;
	}

	for (int i = bpp - 1; i >= 0; i--)
	{
		m_maincpu_space->write_byte(addr + i, data & 0xff);
		data >>= 8;
	}
}

// QuickDraw ROPs
u32 platinum_device::accel_apply_rop(u32 rop, bool colorize, int depth_sel, u32 src, u32 dst, u32 fg, u32 bg)
{
	if (depth_sel == 0)     // indexed 8 bpp destination
	{
		if (!colorize)
		{
			switch (rop)
			{
				case 0: return src;                 // srcCopy
				case 1: return src | dst;           // srcOr
				case 2: return src ^ dst;           // srcXor
				case 3: return ~src & dst;          // srcBic
				case 4: return ~src;                // notSrcCopy (unsupported by hw; approximated)
				case 5: return ~src | dst;          // notSrcOr
				case 6: return ~src ^ dst;          // notSrcXor
				case 7: return src & dst;           // notSrcBic
			}
		}
		else
		{
			switch (rop)
			{
				case 0: return src;                             // srcCopy (unsupported by hw)
				case 1: return (~src & (dst ^ fg)) ^ fg;        // srcOr
				case 2: return src ^ dst;                       // srcXor
				case 3: return (~src & (dst ^ bg)) ^ bg;        // srcBic
				case 4: return ~src;                            // notSrcCopy (unsupported by hw)
				case 5: return (src & (dst ^ fg)) ^ fg;         // notSrcOr
				case 6: return ~src ^ dst;                      // notSrcXor
				case 7: return (src & (dst ^ bg)) ^ bg;         // notSrcBic
			}
		}
	}
	else                    // direct 16/32 bpp destination
	{
		const u32 invert_mask = (depth_sel == 1) ? 0x7fff : 0x00ffffff;
		const u32 inv_src = src ^ invert_mask;

		if (!colorize)
		{
			switch (rop)
			{
				case 0: return src;                 // srcCopy
				case 1: return src & dst;           // srcOr
				case 2: return inv_src ^ dst;       // srcXor
				case 3: return inv_src | dst;       // srcBic
				case 4: return inv_src;             // notSrcCopy
				case 5: return inv_src & dst;       // notSrcOr
				case 6: return src ^ dst;           // notSrcXor
				case 7: return src | dst;           // notSrcBic
			}
		}
		else
		{
			switch (rop)
			{
				case 0: return (inv_src & fg) | (src & bg);     // srcCopy
				case 1: return (src & (dst ^ fg)) ^ fg;         // srcOr
				case 2: return inv_src ^ dst;                   // srcXor
				case 3: return (src & (dst ^ bg)) ^ bg;         // srcBic
				case 4: return (src & fg) | (inv_src & bg);     // notSrcCopy
				case 5: return (inv_src & (dst ^ fg)) ^ fg;     // notSrcOr
				case 6: return src ^ dst;                       // notSrcXor
				case 7: return (inv_src & (dst ^ bg)) ^ bg;     // notSrcBic
			}
		}
	}
	return dst;
}

// ROPs for expanding a 1bpp bitmap
u32 platinum_device::accel_apply_rop_expand(u32 rop, u32 src, u32 dst, u32 fg, u32 bg)
{
	switch (rop)
	{
		case 0: return (src & fg) | (~src & bg);    // srcCopy
		case 1: return (src & fg) | (~src & dst);   // srcOr
		case 2: return src ^ dst;                   // srcXor
		case 3: return (src & bg) | (~src & dst);   // srcBic
		case 4: return (~src & fg) | (src & bg);    // notSrcCopy
		case 5: return (~src & fg) | (src & dst);   // notSrcOr
		case 6: return ~src ^ dst;                  // notSrcXor
		case 7: return (~src & bg) | (src & dst);   // notSrcBic
	}
	return dst;
}

//  Decode a pattern dimension field
static int accel_pattern_size(u32 field)
{
	for (int i = 0; i < 6; i++)
	{
		if (BIT(field, i))
		{
			return 8 << i;
		}
	}
	return 8;
}

void platinum_device::accel_execute()
{
	const u32 cmd = m_accel_regs[ACCEL_CMD];
	const u32 cntl = m_accel_regs[ACCEL_CNTL];
	const int depth_sel = (cntl >> 19) & 3;         // 0 = 8bpp, 1 = 16bpp, 2 = 32bpp
	const int bpp = 1 << depth_sel;                 // bytes per pixel
	const bool dir_inc = BIT(cntl, 21);
	const int bit_offset = (cntl >> 16) & 7;
	const int width = m_accel_regs[ACCEL_DSTSIZE] & 0xffff;
	const int height = m_accel_regs[ACCEL_DSTSIZE] >> 16;
	const int dst_rowbytes = m_accel_regs[ACCEL_ROWBYTES] >> 16;
	const int src_rowbytes = m_accel_regs[ACCEL_ROWBYTES] & 0xffff;
	const u32 rop = cmd & ACCEL_CMD_ROP_MASK;
	const bool pattern = (cmd & ACCEL_CMD_PATTERN) != 0;
	const bool fill = (cmd & ACCEL_CMD_FILL) != 0;
	const bool colorize = (cmd & ACCEL_CMD_COLORIZE) != 0;
	const bool expand = (cmd & ACCEL_CMD_EXPAND) != 0;

	LOGMASKED(LOG_ACCEL, "accel: go cmd %08x cntl %08x src %08x dst %08x rb %08x size %dx%d\n",
			  cmd, cntl, m_accel_regs[ACCEL_SRCBASE], m_accel_regs[ACCEL_DSTBASE],
			  m_accel_regs[ACCEL_ROWBYTES], width, height);

	if (cmd & ACCEL_CMD_INT_ENABLE)
	{
		logerror("%s: accelerator completion interrupt requested but not implemented\n", tag());
	}

	if ((width == 0) || (height == 0))
	{
		return;
	}

	// with the direction bit clear the base addresses point at the
	// bottom-right pixel and the image is flipped
	const u32 dst_tl = dir_inc ? m_accel_regs[ACCEL_DSTBASE]
							   : m_accel_regs[ACCEL_DSTBASE] - ((height - 1) * dst_rowbytes) - ((width - 1) * bpp);

	if (fill)
	{
		const u32 fg = m_accel_regs[ACCEL_FGCOLOR];
		for (int y = 0; y < height; y++)
		{
			u32 addr = dst_tl + (y * dst_rowbytes);
			for (int x = 0; x < width; x++)
			{
				accel_write_pixel(addr, bpp, fg);
				addr += bpp;
			}
		}
		return;
	}

	u32 fg = m_accel_regs[ACCEL_FGCOLOR];
	u32 bg = m_accel_regs[ACCEL_BGCOLOR];
	if (expand && !colorize)
	{
		// expansion without colorize implies black foreground/white background
		fg = (depth_sel == 0) ? 0xff : 0;
		bg = (depth_sel == 0) ? 0 : ((depth_sel == 1) ? 0x7fff : 0x00ffffff);
	}

	// pattern geometry: patterns are aligned on their natural boundary and
	// the source address selects the starting pixel within the pattern
	const u32 src_base = m_accel_regs[ACCEL_SRCBASE];
	int pat_w = 0, pat_h = 0, pat_rowbytes = 0, pat_x0 = 0, pat_y0 = 0;
	u32 pat_base = 0;
	if (pattern)
	{
		pat_w = accel_pattern_size(cntl & 0x3f);
		pat_h = accel_pattern_size((cntl >> 8) & 0x3f);
		pat_rowbytes = expand ? (pat_w / 8) : (pat_w * bpp);

		const u32 pat_bytes = pat_rowbytes * pat_h;
		pat_base = src_base & ~(pat_bytes - 1);
		const u32 pat_off = src_base - pat_base;
		pat_y0 = pat_off / pat_rowbytes;
		pat_x0 = expand ? 0 : ((pat_off % pat_rowbytes) / bpp);

		if (!dir_inc)
		{
			// the base selected the bottom-right pixel of the pattern phase
			pat_y0 = ((pat_y0 - (height - 1)) % pat_h + pat_h) % pat_h;
			pat_x0 = ((pat_x0 - (width - 1)) % pat_w + pat_w) % pat_w;
		}
	}

	const u32 src_tl = dir_inc ? src_base
							   : src_base - ((height - 1) * src_rowbytes) - ((width - 1) * (expand ? 0 : bpp));

	for (int ly = 0; ly < height; ly++)
	{
		// order matters because overlapping source and destination do happen!
		const int y = dir_inc ? ly : (height - 1) - ly;
		const u32 dst_line = dst_tl + (y * dst_rowbytes);

		for (int lx = 0; lx < width; lx++)
		{
			const int x = dir_inc ? lx : (width - 1) - lx;
			const u32 dst_addr = dst_line + (x * bpp);
			const u32 dst = accel_read_pixel(dst_addr, bpp);
			u32 out;

			if (expand)
			{
				u32 bit;
				if (pattern)
				{
					const int py = (pat_y0 + y) % pat_h;
					const int px = (bit_offset + x) % pat_w;
					bit = BIT(accel_read_pixel(pat_base + (py * pat_rowbytes) + (px / 8), 1), 7 - (px & 7));
				}
				else
				{
					const int bitpos = bit_offset + x;
					bit = BIT(accel_read_pixel(src_tl + (y * src_rowbytes) + (bitpos / 8), 1), 7 - (bitpos & 7));
				}
				const u32 src = bit ? 0xffffffff : 0;
				out = accel_apply_rop_expand(rop, src, dst, fg, bg);
			}
			else
			{
				u32 src_addr;
				if (pattern)
				{
					const int py = (pat_y0 + y) % pat_h;
					const int px = (pat_x0 + x) % pat_w;
					src_addr = pat_base + (py * pat_rowbytes) + (px * bpp);
				}
				else
				{
					src_addr = src_tl + (y * src_rowbytes) + (x * bpp);
				}
				const u32 src = accel_read_pixel(src_addr, bpp);
				out = accel_apply_rop(rop, colorize, depth_sel, src, dst, fg, bg);
			}

			const u32 pixel_mask = (depth_sel == 0) ? 0xff : ((depth_sel == 1) ? 0xffff : 0xffffffff);
			accel_write_pixel(dst_addr, bpp, out & pixel_mask);
		}
	}
}

/*
    The cursor image is fetched from the 16 bytes immediately preceding each
    visible scanline in the frame buffer.  If the data is non-zero, a cursor
    shows on that scanline.
*/
void platinum_device::draw_hw_cursor(bitmap_rgb32 &bitmap, u32 fb_base, u32 stride)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	const u16 cursor_x = ((m_dacula_regs[DAC_CURSOR_POS_H] & 0xff) << 8) | (m_dacula_regs[DAC_CURSOR_POS_L] & 0xff);

	if (fb_base < 16)
	{
		return;
	}

	for (int y = 0; y < m_vres; y++)
	{
		const u32 band = fb_base + (y * stride) - 16;

		for (int i = 0; i < 32; i++)
		{
			const u8 pix = (vram8[band + (i >> 1)] >> ((i & 1) ? 0 : 4)) & 0xf;
			const int x = cursor_x + i;

			if ((pix != 0) && (x < m_hres))
			{
				if (pix & 8)
				{
					bitmap.pix(y, x) = m_cursor_clut[pix & 7];
				}
				else if (pix & 1)
				{
					// threshold-invert the underlying pixel
					const u32 under = bitmap.pix(y, x);
					bitmap.pix(y, x) = ((((under >> 7) & 0x010101) * 0xff) ^ 0xffffff) | 0xff000000;
				}
			}
		}
	}
}

u32 platinum_device::dacula_r(offs_t offset, u32 mem_mask)
{
	offset >>= 2;
	LOGMASKED(LOG_RAMDAC, "Read DAC @ %x (%s)\n", offset, machine().describe_context());

	if (offset == DAC_CONTROL_LATCH)
	{
		switch (m_dacula_regs[DAC_ADDR_LATCH])
		{
			case DAC_CONTROL:
				return m_dacula_regs[DAC_CONTROL];

			case DAC_PLL_CONTROL:
				return m_dacula_regs[DAC_PLL_CONTROL];

			case DAC_ID:
				return 0x3c; // ID register, 0x3c for Platinum
		}
	}

	return m_dacula_regs[offset];
}

void platinum_device::dacula_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset >>= 2;
	LOGMASKED(LOG_RAMDAC, "%08x to DAC reg %x mask %08x (%s)\n", data, offset, mem_mask, machine().describe_context());
	m_dacula_regs[offset] = data;

	switch (offset)
	{
		case DAC_CONTROL_LATCH:
			dacula_w(m_dacula_regs[DAC_ADDR_LATCH] << 2, data, mem_mask);
			break;

		case DAC_ADDR_LATCH:
			m_pal_idx = 0;
			break;

		case DAC_CURSOR_CLUT:
			m_cursor_rgb[m_pal_idx++] = data & 0xff;
			if (m_pal_idx >= 3)
			{
				m_cursor_clut[m_dacula_regs[DAC_ADDR_LATCH] & 7] = rgb_t(m_cursor_rgb[0], m_cursor_rgb[1], m_cursor_rgb[2]);
				m_dacula_regs[DAC_ADDR_LATCH]++;
				m_pal_idx = 0;
			}
			break;

		case DAC_DATA_LATCH:
			switch (m_pal_idx)
			{
			case 0:
				m_palette->set_pen_red_level(m_dacula_regs[0] & 0xff, data & 0xff);
				break;
			case 1:
				m_palette->set_pen_green_level(m_dacula_regs[0] & 0xff, data & 0xff);
				break;
			case 2:
				m_palette->set_pen_blue_level(m_dacula_regs[0] & 0xff, data & 0xff);
				break;
			}
			m_pal_idx++;
			if (m_pal_idx >= 3)
			{
				m_pal_idx = 0;
				m_dacula_regs[DAC_ADDR_LATCH] = (m_dacula_regs[DAC_ADDR_LATCH] + 1) & 0xff;
			}
			break;

		case DAC_PLL_CONTROL:
		case DAC_VCLK_M_A:
		case DAC_VCLK_PN_A:
		case DAC_VCLK_M_B:
		case DAC_VCLK_PN_B:
			recalc_clock();
			break;
	}
}

// seems to be similar to the MEMCjr "Gazelle" PLL, which in turn is based on the
// Sierra SC11412.
void platinum_device::recalc_clock()
{
	double m = 0, n = 0;
	int p = 0;

	if (!BIT(m_dacula_regs[DAC_PLL_CONTROL], 0))
	{
		m = m_dacula_regs[DAC_VCLK_M_A];
		n = m_dacula_regs[DAC_VCLK_PN_A] & 0x3f;
		p = m_dacula_regs[DAC_VCLK_PN_A] >> 5;
	}
	else
	{
		m = m_dacula_regs[DAC_VCLK_M_B];
		n = m_dacula_regs[DAC_VCLK_PN_B] & 0x3f;
		p = m_dacula_regs[DAC_VCLK_PN_B] >> 5;
	}

	if (n == 0)
	{
		return;
	}

	double new_clock = 14318180.0f * (m / (n * double(1 << p)));
	LOGMASKED(LOG_RAMDAC, "Recalc clock: m=%f n=%f p=%d => new clock %f\n", m, n, p, new_clock);
	m_pixel_clock = std::round(new_clock);
	recalc_mode();
}

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config_platinum)
	PORT_START("monitor")
	PORT_CONFNAME(0x7f, 6, "Monitor type")
	PORT_CONFSETTING(0x00,         u8"Mac 21\" Color Display (1152\u00d7870)")          // "RGB 2 Page" or "Kong"
	PORT_CONFSETTING(0x01,         u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02,         u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x03,         u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")   // "2 Page"
	PORT_CONFSETTING(0x06,         u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(0x07,         u8"No monitor, disable internal video")              // No monitor connected
	PORT_CONFSETTING(ext(0, 0, 0), u8"PAL Encoder (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 0), u8"NTSC Encoder (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(0x04,         u8"NTSC Monitor (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(ext(3, 0, 0), u8"PAL Monitor (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 3), u8"640x480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), u8"832x624 16\" RGB")                                // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 2, 2), u8"1024\u00d7768 19\" RGB")
INPUT_PORTS_END

ioport_constructor platinum_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config_platinum);
}
