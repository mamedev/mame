// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Williams first-generation blitter

    Register Description
    --------------------

    CA00 start_blitter    Control bits
    CA01 mask             Solid color to optionally use instead of source
    CA02 source           Source address MSB
    CA03 source           Source address LSB
    CA04 dest             Destination address MSB
    CA05 dest             Destination address LSB
    CA06 w_h              Blit width (XOR with 4 to have the real value on first-rev blitter)
    CA07 w_h              Blit height (XOR with 4 to have the real value on first-rev blitter)

***************************************************************************/

#include "emu.h"
#include "williamsblitter.h"

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(WILLIAMS_BLITTER_SC1, williams_blitter_sc1_device, "williams_blitter_sc1_device", "Williams Blitter (SC1)")
DEFINE_DEVICE_TYPE(WILLIAMS_BLITTER_SC2, williams_blitter_sc2_device, "williams_blitter_sc2_device", "Williams Blitter (SC2)")

williams_blitter_device::williams_blitter_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_vram(*this, finder_base::DUMMY_TAG),
	m_proms(*this, finder_base::DUMMY_TAG)
{
}

williams_blitter_sc1_device::williams_blitter_sc1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	williams_blitter_device(mconfig, WILLIAMS_BLITTER_SC1, tag, owner, clock)
{
}

williams_blitter_sc2_device::williams_blitter_sc2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	williams_blitter_device(mconfig, WILLIAMS_BLITTER_SC2, tag, owner, clock)
{
}

void williams_blitter_device::device_start()
{
	static const u8 dummy_table[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	m_window_enable = 0;

	// create the remap table; if no PROM, make an identity remap table
	m_remap_lookup = std::make_unique<uint8_t[]>(256 * 256);
	m_remap = m_remap_lookup.get();
	for (int i = 0; i < 256; i++)
	{
		const u8 *table = m_proms.found() ? &m_proms[(i & 0x7f) * 16] : dummy_table;
		for (int j = 0; j < 256; j++)
		{
			m_remap_lookup[i * 256 + j] = (table[j >> 4] << 4) | table[j & 0x0f];
		}
	}

	save_item(NAME(m_window_enable));
	save_item(NAME(m_control));
	save_item(NAME(m_no_even));
	save_item(NAME(m_no_odd));
	save_item(NAME(m_solid));
	save_item(NAME(m_fg_only));
	save_item(NAME(m_solid_color));
	save_item(NAME(m_sstart));
	save_item(NAME(m_dstart));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_remap_index));
}

void williams_blitter_device::device_reset()
{
	m_control = 0;
	m_no_even = false;
	m_no_odd = false;
	m_solid = false;
	m_fg_only = false;
	m_solid_color = 0;
	m_sstart = 0;
	m_dstart = 0;
	m_width = 0;
	m_height = 0;
	m_remap_index = 0;
}

void williams_blitter_device::map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(williams_blitter_device::control_w));
	map(0x1, 0x1).lw8(NAME([this](u8 data) { m_solid_color = data; }));
	map(0x2, 0x2).lw8(NAME([this](u8 data) { m_sstart = (m_sstart & 0x00ff) | ((u16)data << 8); }));
	map(0x3, 0x3).lw8(NAME([this](u8 data) { m_sstart = (m_sstart & 0xff00) | data; }));
	map(0x4, 0x4).lw8(NAME([this](u8 data) { m_dstart = (m_dstart & 0x00ff) | ((u16)data << 8); }));
	map(0x5, 0x5).lw8(NAME([this](u8 data) { m_dstart = (m_dstart & 0xff00) | data; }));
	map(0x6, 0x6).lw8(NAME([this](u8 data) { m_width = data; }));
	map(0x7, 0x7).lw8(NAME([this](u8 data) { m_height = data; }));
}

void williams_blitter_device::control_w(address_space &space, offs_t offset, u8 data)
{
	m_control = data;
	m_no_even = BIT(data, CONTROLBYTE_NO_EVEN);
	m_no_odd = BIT(data, CONTROLBYTE_NO_ODD);
	m_solid = BIT(data, CONTROLBYTE_SOLID);
	m_fg_only = BIT(data, CONTROLBYTE_FOREGROUND_ONLY);

	int w = m_width ^ m_size_xor;
	int h = m_height ^ m_size_xor;

	if (w == 0)
		w = 1;
	if (h == 0)
		h = 1;

	const int accesses = blit_core(space, w, h);

	// based on the number of memory accesses needed to do the blit, compute how long the blit will take
	int estimated_clocks_at_4MHz = 4;
	if (BIT(m_control, CONTROLBYTE_SLOW))
		estimated_clocks_at_4MHz += 4 * (accesses + 2);
	else
		estimated_clocks_at_4MHz += 2 * (accesses + 3);

	m_cpu->adjust_icount(-((estimated_clocks_at_4MHz + 3) / 4));

	// log blits
	LOG("%04X:Blit : %04X -> %04X, %3dx%3d, mask=%02X, flags=%02X, icount=%d, win=%d\n",
		m_cpu->pc(), m_sstart, m_dstart, m_width, m_height, m_solid_color, m_control,
		((estimated_clocks_at_4MHz + 3) / 4), m_window_enable);
}

void williams_blitter_device::remap_select_w(u8 data)
{
	m_remap_index = data;
	if (m_proms != nullptr)
		m_remap = m_remap_lookup.get() + data * 256;
}

void williams_blitter_device::window_enable_w(u8 data)
{
	m_window_enable = BIT(data, 0);
}

inline void williams_blitter_device::blit_pixel(address_space &space, int dstaddr, int srcdata)
{
	// always read from video RAM regardless of the bank setting
	int curpix = (dstaddr < 0xc000) ? m_vram[dstaddr] : space.read_byte(dstaddr); // current pixel values at dest

	u8 keepmask = 0xff; // what part of original dst byte should be kept, based on NO_EVEN and NO_ODD flags

	// even pixel (D7-D4)
	if (m_fg_only && !(srcdata & 0xf0)) // FG only and src even pixel=0
	{
		if (m_no_even)
			keepmask &= 0x0f;
	}
	else
	{
		if (!m_no_even)
			keepmask &= 0x0f;
	}

	// odd pixel (D3-D0)
	if (m_fg_only && !(srcdata & 0x0f)) // FG only and src odd pixel=0
	{
		if (m_no_odd)
			keepmask &= 0xf0;
	}
	else
	{
		if (!m_no_odd)
			keepmask &= 0xf0;
	}

	curpix &= keepmask;
	if (m_solid)
		curpix |= m_solid_color & ~keepmask;
	else
		curpix |= srcdata & ~keepmask;

	/* if the window is enabled, only blit to videoram below the clipping address */
	/* note that we have to allow blits to non-video RAM (e.g. tileram, Sinistar $DXXX SRAM) because those */
	/* are not blocked by the window enable */
	if (!m_window_enable || dstaddr < m_clip_address || dstaddr >= 0xc000)
		space.write_byte(dstaddr, curpix);
}

int williams_blitter_device::blit_core(address_space &space, int w, int h)
{
	// compute how much to advance in the x and y loops
	const bool dst_stride_256 = BIT(m_control, CONTROLBYTE_DST_STRIDE_256);
	const bool src_stride_256 = BIT(m_control, CONTROLBYTE_SRC_STRIDE_256);
	const int sxadv = src_stride_256 ? 0x100 : 1;
	const int syadv = src_stride_256 ? 1 : w;
	const int dxadv = dst_stride_256 ? 0x100 : 1;
	const int dyadv = dst_stride_256 ? 1 : w;

	int accesses = 0;
	int pixdata = 0;

	// loop over the height
	const bool shift = BIT(m_control, CONTROLBYTE_SHIFT);
	u16 sstart = m_sstart;
	u16 dstart = m_dstart;
	for (int y = 0; y < h; y++)
	{
		u16 source = sstart;
		u16 dest = dstart;

		// loop over the width
		for (int x = 0; x < w; x++)
		{
			const u8 rawval = m_remap[space.read_byte(source)];
			if (shift)
			{
				// shift one pixel right
				pixdata = (pixdata << 8) | rawval;
				blit_pixel(space, dest, (pixdata >> 4) & 0xff);
			}
			else
			{
				blit_pixel(space, dest, rawval);
			}
			accesses += 2;

			// advance src and dst pointers
			source += sxadv;
			dest   += dxadv;
		}

		// note that PlayBall! indicates the X coordinate doesn't wrap
		if (dst_stride_256)
			dstart = (dstart & 0xff00) | ((dstart + dyadv) & 0xff);
		else
			dstart += dyadv;

		if (src_stride_256)
			sstart = (sstart & 0xff00) | ((sstart + syadv) & 0xff);
		else
			sstart += syadv;
	}
	return accesses;
}
