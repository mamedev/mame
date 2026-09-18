// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple CIVIC (Cyclone Integrated Video Interface Controller) 343S1096 (for 25/33 MHz bus), 343S1103 (for 40 MHz bus)
    Emulation by R. Belmont

    CIVIC manages the video input and output of the Macintosh Quadra 660AV and 840AV, and it was also used with
    a bus translator in the Power Macintosh 6100AV/7100AV/8100AV.

    TODO:
    - 16 and 24 bpp modes have issues when bit 4 of DAC mode is set
    - convolution
*/

#include "emu.h"
#include "civic.h"

#include "endianness.h"

#define LOG_CRTC        (1U << 1)
#define LOG_CLOCKGEN    (1U << 2)
#define LOG_MONSENSE    (1U << 3)
#define LOG_RAMDAC      (1U << 4)
#define LOG_REGISTERS   (1U << 5)

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(CIVIC, civic_device, "civic", "Apple CIVIC video")

static constexpr u32 register_map[60*2] =
{
	0x000, 1, 0x004, 1, 0x008, 1, 0x00C, 1, 0x010, 1, 0x014, 1, 0x018, 1, 0x01C, 1,
	0x020, 3, 0x02C, 1, 0x040, 1, 0x044, 2, 0x04C, 1, 0x050, 1, 0x054, 1, 0x058, 1,
	0x05C, 1, 0x060, 1, 0x064, 1, 0x068, 1, 0x06C, 1, 0x080, 3, 0x08C, 8, 0x0C0, 9,
	0x100, 1, 0x104, 1, 0x108, 1, 0x10C, 1, 0x110, 1, 0x114, 1, 0x118, 1, 0x11C, 1,
	0x120, 1, 0x124, 1, 0x128, 1, 0x12C, 1, 0x140, 12, 0x180, 12, 0x1C0, 12, 0x200, 2,
	0x240, 12, 0x280, 12, 0x2C0, 8, 0x300, 12, 0x340, 12, 0x380, 12, 0x3C0, 12, 0x400, 12,
	0x440, 10, 0x480, 12, 0x4C0, 12, 0x500, 12, 0x540, 12, 0x580, 12, 0x5C0, 12, 0x600, 12,
	0x640, 12, 0x680, 12, 0x6C0, 12, 0x208, 1
};

static const char *register_names[60] =
{
	"VBLInt", "Enable", "VDCInt", "VDCClr", "VDCEnb", "VidInSize", "VDCClk", "ScanCtl",
	"GSCDivide", "VSCDivide", "VRAMSize", "RefreshCtl", "BusSize", "SpeedCtl", "ConvEnb", "SenseCtl",
	"Sense0", "Sense1", "Sense2", "Tristate", "SyncClr", "ReadSense", "RowWords", "BaseAddr",
	"VLDB", "VHLTB", "VActHi", "Reset", "VBLEnb", "HLDB", "HHLTB", "HActHi", "VBLClr",
	"AdjF2", "AdjF1", "TestEnb", "CntTest", "HSerr", "VInHAL", "VInHFPD", "VInHFP",
	"HlfLn", "HEq", "HSP", "HBWay", "HAL", "HFP", "HPix", "PipeD",
	"VHLine", "VSync", "VBPEq", "VBP", "VAL", "VInVAL", "VInVFP", "VFP",
	"VFPEq", "CurLine", "VInDoubleLine"
};

enum
{
	CIVIC_VBLInt = 0, CIVIC_Enable, CIVIC_VDCInt, CIVIC_VDCClr, CIVIC_VDCEnb, CIVIC_VidInSize, CIVIC_VDCClk,
	CIVIC_ScanCtl, CIVIC_GSCDivide, CIVIC_VSCDivide, CIVIC_VRAMSize, CIVIC_RefreshCtl, CIVIC_BusSize,
	CIVIC_SpeedCtl, CIVIC_ConvEnb, CIVIC_SenseCtl, CIVIC_Sense0, CIVIC_Sense1, CIVIC_Sense2, CIVIC_Tristate,
	CIVIC_SyncClr, CIVIC_ReadSense, CIVIC_RowWords, CIVIC_BaseAddr, CIVIC_VLDB, CIVIC_VHLTB, CIVIC_VActHi,
	CIVIC_Reset, CIVIC_VBLEnb, CIVIC_HLDB, CIVIC_HHLTB, CIVIC_HActHi, CIVIC_VBLClr, CIVIC_AdjF2, CIVIC_AdjF1,
	CIVIC_TestEnb, CIVIC_CntTest, CIVIC_HSerr, CIVIC_VInHAL, CIVIC_VInHFPD, CIVIC_VInHFP, CIVIC_HlfLn,
	CIVIC_HEq, CIVIC_HSP, CIVIC_HBWay, CIVIC_HAL, CIVIC_HFP, CIVIC_HPix, CIVIC_PipeD, CIVIC_VHLine, CIVIC_VSync,
	CIVIC_VBPEq, CIVIC_VBP, CIVIC_VAL, CIVIC_VInVAL, CIVIC_VInVFP, CIVIC_VFP, CIVIC_VFPEq, CIVIC_CurLine,
	CIVIC_VInDoubleLine,

	CIVIC_LastReg = CIVIC_VInDoubleLine
};

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void civic_device::map(address_map &map)
{
	map(0x0003'6000, 0x0003'66ff).rw(FUNC(civic_device::civic_r), FUNC(civic_device::civic_w));
	map(0x0010'0000, 0x002f'ffff).rw(FUNC(civic_device::vram_r), FUNC(civic_device::vram_w));
	map(0x00f2'e000, 0x00f2'ffff).rw(FUNC(civic_device::clockgen_r), FUNC(civic_device::clockgen_w));
	map(0x00f3'0800, 0x00f3'083f).rw(FUNC(civic_device::ramdac_r), FUNC(civic_device::ramdac_w));
}

civic_device::civic_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_vram_size(0x200000),
	m_pixel_clock(31334400),
	m_is_clifton(false),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_clockgen(*this, "clifton"),
	m_monitor_config(*this, "monitor"),
	m_irq(*this),
	m_monitor_id(0),
	m_pal_address(0),
	m_pal_idx(0),
	m_sebastian_ctrl(0),
	m_base(0x800), m_stride(1024),
	m_int_status(0), m_hres(640), m_vres(480), m_htotal(896), m_vtotal(525),
	m_M(0), m_N(0), m_CLK(1),
	m_clocksel(0)
{
}

civic_device::civic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	civic_device(mconfig, CIVIC, tag, owner, clock)
{
}

void civic_device::device_start()
{
	m_vram = std::make_unique<u32[]>(m_vram_size);

	m_vbl_timer = timer_alloc(FUNC(civic_device::vbl_tick), this);
	m_vbl_timer->adjust(attotime::never);

	save_item(NAME(m_monitor_id));
	save_item(NAME(m_base));
	save_item(NAME(m_stride));
	save_item(NAME(m_pal_address));
	save_item(NAME(m_pal_idx));
	save_item(NAME(m_sebastian_ctrl));
	save_item(NAME(m_hres));
	save_item(NAME(m_vres));
	save_item(NAME(m_M));
	save_item(NAME(m_N));
	save_item(NAME(m_CLK));
	save_item(NAME(m_pixel_clock));

	machine().save().register_postload(save_prepost_delegate(FUNC(civic_device::recalc_mode), this));

	std::fill_n(&m_regs[0], std::size(m_regs), 0);

	std::fill_n(&m_register_base[0], std::size(m_register_base), 0xffffffff);
	std::fill_n(&m_register_shift[0], std::size(m_register_shift), 0xffffffff);

	for (int entry = 0; entry < 60*2; entry += 2)
	{
		u32 location = register_map[entry] >> 2;
		const int bits = register_map[entry + 1];

		for (int bit = 0; bit < bits; bit++)
		{
			m_register_base[location] = entry/2;
			m_register_shift[location] = bit;
			location++;
		}
	}
}

void civic_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen);
	// dot clock, htotal, hstart, hend, vtotal, vstart, vend
	m_screen->set_raw(31334400, 896, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(FUNC(civic_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	ICD2053B(config, m_clockgen, 17.734475_MHz_XTAL);
	m_clockgen->clkout_changed().set(FUNC(civic_device::pclock_w));
}

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(monitor_config)
	PORT_START("monitor")
	PORT_CONFNAME(0x7f, 6, "Monitor type")
	PORT_CONFSETTING(0x00, u8"Mac 21\" Color Display (1152\u00d7870)")          // "RGB 2 Page" or "Kong"
	PORT_CONFSETTING(0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")   // "2 Page"
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(0x07, u8"No monitor, disable internal video")              // No monitor connected
	PORT_CONFSETTING(ext(0, 0, 0), "PAL Encoder (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 0), "NTSC Encoder (512\u00d7384, 640\u00d7480)")
	PORT_CONFSETTING(ext(3, 0, 0), "PAL (640\u00d7480, 768\u00d7576)")
	PORT_CONFSETTING(ext(1, 1, 3), "640x480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), "832x624 16\" RGB")                          // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 2, 2), "1024x\u00d7768 19\" RGB")
INPUT_PORTS_END

ioport_constructor civic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(monitor_config);
}

u32 civic_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!(m_regs[CIVIC_Enable] & 1))
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]) + m_base;
	const pen_t *pens = m_palette->pens();
	const int stride = m_stride;

	switch ((m_sebastian_ctrl & 7))
	{
		case 0: // 1bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/8; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(pixels & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 1) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 2) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 3) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 4) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 5) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 6) & 0x80) | 0x7f];
					*scanline++ = pens[((pixels << 7) & 0x80) | 0x7f];
				}
			}
		}
		break;

		case 1: // 2bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/4; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[(((pixels >> 6) & 3) << 6) | 0x3f];
					*scanline++ = pens[(((pixels >> 4) & 3) << 6) | 0x3f];
					*scanline++ = pens[(((pixels >> 2) & 3) << 6) | 0x3f];
					*scanline++ = pens[((pixels & 3) << 6) | 0x3f];
				}
			}
		}
		break;

		case 2: // 4bpp
		{
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				for (int x = 0; x < m_hres/2; x++)
				{
					u8 const pixels = vram8[(y * stride) + x];

					*scanline++ = pens[0x0f | (pixels & 0xf0)];
					*scanline++ = pens[0x0f | ((pixels << 4) & 0xf0)];
				}
			}
		}
		break;

		case 3: // 8bpp
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

		case 4: // 16bpp x555
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

		case 5: // 24 bpp
			for (int y = 0; y < m_vres; y++)
			{
				u32 *scanline = &bitmap.pix(y);
				u32 const *base = &m_vram[(y * (stride/4)) + (m_base/4)];
				for (int x = 0; x < m_hres; x++)
				{
					*scanline++ = *base++;
				}
			}
			break;
	}
	return 0;
}

void civic_device::clock_select_w(int state)
{
	m_clocksel = state;

	if (m_is_clifton)
	{
		m_clockgen->set_clock(state ? 14.318181_MHz_XTAL : 17.734475_MHz_XTAL);
	}
}

u32 civic_device::civic_r(offs_t offset)
{
	if (offset != 0)
	{
		LOGMASKED(LOG_REGISTERS, "civic_r: offset %08x => reg %02x shift %d (%s)\n", offset, m_register_base[offset], m_register_shift[offset], register_names[m_register_base[offset]]);
	}

	const int regnum = m_register_base[offset];
	const int shift = m_register_shift[offset];
	switch (regnum)
	{
		case CIVIC_VBLInt:
			return m_int_status & 1;

		case CIVIC_ReadSense:
			{
				u8 mon = m_monitor_config->read();
				u8 monitor_id;
				u8 res;

				monitor_id = m_regs[CIVIC_Sense2] << 2 | m_regs[CIVIC_Sense1] << 1 | m_regs[CIVIC_Sense0];

				if (mon & 0x40)
				{
					res = 7;
					if (monitor_id == 0x5)
					{
						res = (mon >> 4) & 3;
					}
					else if (monitor_id == 0x3)
					{
						res = (BIT(mon, 3) << 2) | BIT(mon, 2);
					}
					else if (monitor_id == 0x0)
					{
						res = (mon & 3) << 1;
					}
				}
				else
				{
					res = mon;
				}

				m_regs[CIVIC_ReadSense] = res;

				LOGMASKED(LOG_MONSENSE, "Sense result = %x (monitor_id %x)\n", res, monitor_id);
			}
			break;

		case CIVIC_CurLine:
			m_regs[CIVIC_CurLine] = m_screen->vpos();
			break;
	}

	return (m_regs[regnum] & (1 << shift)) ? 1 : 0;
}

void civic_device::civic_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_REGISTERS, "civic_w: %d @ offset %08x => reg %02x shift %d (%s)\n", data & 1, offset << 4, m_register_base[offset], m_register_shift[offset], register_names[m_register_base[offset]]);

	// and cook the data into a more conventionally usable form
	const int regnum = m_register_base[offset];
	const int shift = m_register_shift[offset];
	m_regs[regnum] &= ~(1 << shift);
	m_regs[regnum] |= (data & 1) << shift;
	if ((regnum == CIVIC_LastReg) || (m_register_shift[offset + 1] == 0) || (m_register_shift[offset + 1] == 0xffffffff))
	{
		if (regnum != CIVIC_VBLClr)
		{
			LOGMASKED(LOG_REGISTERS, "CIVIC reg %02x (%s) now %08x\n", regnum, register_names[regnum], m_regs[regnum]);
		}
	}
	switch (regnum)
	{
		case CIVIC_Enable: // Enable
			if (data & 1)
			{
				recalc_mode();
			}
			break;

		case CIVIC_VBLEnb: // VBL interrupt enable
			if (data & 1)
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
			}
			else
			{
				// stop generating VBLs and drop any pending one, otherwise the OS sees a
				// slot interrupt it has no handler for once it removes the VBL task
				m_vbl_timer->adjust(attotime::never);
				m_int_status &= ~1;
				recalc_ints();
			}
			break;

		case CIVIC_VBLClr: // VBLClr
			// write 1 to enable, 0 to clear
			if (data & 1)
			{
				if (m_regs[CIVIC_VBLEnb] & 1)
				{
					m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
				}
			}
			else
			{
				m_int_status &= ~1;
				recalc_ints();
			}
			break;

		case CIVIC_VDCEnb: // VDCEnb
			if (data & 1)
			{
				m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
			}
			else
			{
				m_vbl_timer->adjust(attotime::never);
			}
			break;
	}
}

u32 civic_device::ramdac_r(offs_t offset)
{
	switch (offset << 2)
	{
		case 0:
			if (!machine().side_effects_disabled())
			{
				m_pal_idx = 0;
			}
			return m_pal_address << 24;

		case 0x10:
			{
				pen_t const entry = m_palette->pen(m_pal_address);
				u8 const idx = m_pal_idx;
				if (!machine().side_effects_disabled())
				{
					m_pal_idx++;
				}
				switch (idx)
				{
					case 0:
						return ((entry >> 16) & 0xff) << 24;
					case 1:
						return ((entry >> 8) & 0xff) << 24;
					case 2:
						return (entry & 0xff) << 24;
				}
			}
			break;

		case 0x20:
				LOGMASKED(LOG_RAMDAC, "Read %02x from DAC control\n", m_sebastian_ctrl);
				return m_sebastian_ctrl << 24;
	}
	return 0;
}

void civic_device::ramdac_w(offs_t offset, u32 data)
{
	switch (offset << 2)
	{
	case 0:
		m_pal_address = data >> 24;
		m_pal_idx = 0;
		break;

	case 0x10:
		if (!(m_sebastian_ctrl & 0x40))
		{
			if ((m_monitor_config->read() == 1) || (m_monitor_config->read() == 3))
			{
				// monochrome monitors put info only on the blue channel
				if (m_pal_idx == 2)
				{
					m_palette->set_pen_red_level(m_pal_address, data >> 24);
					m_palette->set_pen_green_level(m_pal_address, data >> 24);
					m_palette->set_pen_blue_level(m_pal_address, data >> 24);
				}
			}
			else
			{
				switch (m_pal_idx)
				{
					case 0:
						m_palette->set_pen_red_level(m_pal_address, data >> 24);
						break;
					case 1:
						m_palette->set_pen_green_level(m_pal_address, data >> 24);
						break;
					case 2:
						m_palette->set_pen_blue_level(m_pal_address, data >> 24);
						break;
				}
			}

			m_pal_idx++;
			if (m_pal_idx == 4)
			{
				m_pal_idx = 0;
				m_pal_address++;
			}
		}
		break;

	case 0x20:
		LOGMASKED(LOG_RAMDAC, "%08x to Sebastian pixel bus control, & 0x7 = %02x\n", data, data & 7);
		m_sebastian_ctrl = data >> 24;
		recalc_mode();
		break;
	}
}

void civic_device::recalc_mode()
{
	m_vtotal = m_regs[CIVIC_VHLine] / 2;
	m_vres = (m_regs[CIVIC_VFP] -  m_regs[CIVIC_VAL]) / 2;

	m_htotal = m_regs[CIVIC_HlfLn] * 4;
	m_hres = (m_regs[CIVIC_HFP] - m_regs[CIVIC_HAL]) * 2;

	m_stride = m_regs[CIVIC_RowWords] * 32;
	m_base = m_regs[CIVIC_BaseAddr] * 32;

	const double refresh = (double)m_pixel_clock / (double)(m_htotal * m_vtotal);
	LOGMASKED(LOG_CRTC, "hres %d vres %d htotal %d vtotal %d refresh %f stride %d mode %d pclk %d\n", m_hres, m_vres, m_htotal, m_vtotal, refresh, m_stride, m_sebastian_ctrl & 7, m_pixel_clock);
	if ((m_hres != 0) && (m_vres != 0))
	{
		rectangle visarea(0, m_hres - 1, 0, m_vres - 1);
		m_screen->configure(m_htotal, m_vtotal, visarea, attotime::from_ticks(m_htotal * m_vtotal, m_pixel_clock));
	}
}

void civic_device::pclock_w(u32 new_clock)
{
	LOGMASKED(LOG_CLOCKGEN, "Got new pixel clock %d\n", new_clock);
	m_pixel_clock = new_clock;
}

u8 civic_device::clockgen_r(offs_t offset)
{
	return 0;
}

void civic_device::clockgen_w(offs_t offset, u8 data)
{
	if (m_is_clifton)
	{
		// Clock is connected to chip enable, so each write pulses the clock line
		m_clockgen->data_w(data & 1);
		m_clockgen->clk_w(CLEAR_LINE);
		m_clockgen->clk_w(ASSERT_LINE);
	}
	else
	{
		switch (offset)
		{
			case 0:
				m_M = data;
				break;

			case 0x10:
				m_N = data;

				{
					const double ratio = ((double)m_M * 4.0f) / (double)m_N;
					const double pclk = 2.0f * (m_clocksel ? 14.3181818f : 17.734475f) * ratio;
					m_pixel_clock = (u32)(pclk * 1000000 + .5);
					LOGMASKED(LOG_CLOCKGEN, "M %d N %d Clock select %d => ratio %f, pixel clock %d\n", m_M, m_N, m_clocksel, ratio, m_pixel_clock);
				}
				break;

			case 0x20:
				m_CLK = data;
				break;
		}
	}
}

u32 civic_device::vram_r(offs_t offset)
{
	if (offset >= (m_vram_size>>2))
	{
		return 0;
	}

	return m_vram[offset];
}

void civic_device::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= (m_vram_size >> 2))
	{
		return;
	}

	COMBINE_DATA(&m_vram[offset]);
}

void civic_device::recalc_ints()
{
	if (m_int_status != 0)
	{
		m_irq(ASSERT_LINE);
	}
	else
	{
		m_irq(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(civic_device::vbl_tick)
{
	m_int_status |= 1;
	recalc_ints();

	if (m_regs[CIVIC_VBLEnb] & 1)
	{
		m_vbl_timer->adjust(m_screen->time_until_pos(480, 0), 0);
	}
}

