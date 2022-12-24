// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Epson SED1375 LCD Controller emulation skeleton

    - Currently hard-coded for use with the Palm IIIc driver.
    - TODO: Find more test-case systems.

**********************************************************************/

#include "emu.h"
#include "sed1375.h"
#include "screen.h"

#define LOG_REG_READ    (1 << 1)
#define LOG_REG_WRITE   (1 << 2)
#define LOG_VRAM_READ   (1 << 3)
#define LOG_VRAM_WRITE  (1 << 4)
#define LOG_LUT_READ    (1 << 5)
#define LOG_LUT_WRITE   (1 << 6)
#define LOG_VBL_READ    (1 << 7)
#define LOG_ALL         (LOG_REG_READ | LOG_REG_WRITE | LOG_VRAM_READ | LOG_VRAM_WRITE | LOG_LUT_READ | LOG_LUT_WRITE | LOG_VBL_READ)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SED1375, sed1375_device, "sed1375", "Epson SED1375")

sed1375_device::sed1375_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SED1375, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_vram(*this, "vram", 80 * 1024, ENDIANNESS_BIG)
{
}

void sed1375_device::device_start()
{
	save_item(NAME(m_revision));
	save_item(NAME(m_mode));
	save_item(NAME(m_panel_hsize));
	save_item(NAME(m_panel_vsize));
	save_item(NAME(m_fpline_start));
	save_item(NAME(m_hblank_period));
	save_item(NAME(m_fpframe_start));
	save_item(NAME(m_vblank_period));
	save_item(NAME(m_mod_rate));
	save_item(NAME(m_screen_start));
	save_item(NAME(m_screen_start_ovf));
	save_item(NAME(m_mem_addr_offset));
	save_item(NAME(m_screen1_vsize));
	save_item(NAME(m_lut_addr));
	save_item(NAME(m_lut_index));
	save_item(NAME(m_red_lut));
	save_item(NAME(m_green_lut));
	save_item(NAME(m_blue_lut));
	save_item(NAME(m_gpio_config));
	save_item(NAME(m_gpio_status));
	save_item(NAME(m_scratch));
	save_item(NAME(m_swivel_mode));
	save_item(NAME(m_swivel_bytecnt));
}

void sed1375_device::device_reset()
{
	m_revision = 0x24;
	std::fill(std::begin(m_mode), std::end(m_mode), 0);
	m_panel_hsize = 0x03;
	m_panel_vsize = 0;
	m_fpline_start = 0;
	m_hblank_period = 0;
	m_fpframe_start = 0;
	m_vblank_period = 0;
	m_mod_rate = 0;
	std::fill(std::begin(m_screen_start), std::end(m_screen_start), 0);
	m_screen_start_ovf = 0;
	m_mem_addr_offset = 0;
	m_screen1_vsize = 0;
	m_lut_addr = 0;
	m_lut_index = 0;
	std::fill(std::begin(m_red_lut), std::end(m_red_lut), 0);
	std::fill(std::begin(m_green_lut), std::end(m_green_lut), 0);
	std::fill(std::begin(m_blue_lut), std::end(m_blue_lut), 0);
	m_gpio_config = 0;
	m_gpio_status = 0;
	m_scratch = 0;
	m_swivel_mode = 0;
	m_swivel_bytecnt = 0;
}

void sed1375_device::map(address_map &map)
{
	map(0x00000, 0x13fff).rw(FUNC(sed1375_device::vram_r), FUNC(sed1375_device::vram_w));
	map(0x1ffe0, 0x1ffe0).r(FUNC(sed1375_device::revision_r));
	map(0x1ffe1, 0x1ffe1).rw(FUNC(sed1375_device::mode0_r), FUNC(sed1375_device::mode0_w));
	map(0x1ffe2, 0x1ffe2).rw(FUNC(sed1375_device::mode1_r), FUNC(sed1375_device::mode1_w));
	map(0x1ffe3, 0x1ffe3).rw(FUNC(sed1375_device::mode2_r), FUNC(sed1375_device::mode2_w));
	map(0x1ffe4, 0x1ffe4).rw(FUNC(sed1375_device::panel_hsize_r), FUNC(sed1375_device::panel_hsize_w));
	map(0x1ffe5, 0x1ffe5).rw(FUNC(sed1375_device::panel_vsize_lsb_r), FUNC(sed1375_device::panel_vsize_lsb_w));
	map(0x1ffe6, 0x1ffe6).rw(FUNC(sed1375_device::panel_vsize_msb_r), FUNC(sed1375_device::panel_vsize_msb_w));
	map(0x1ffe7, 0x1ffe7).rw(FUNC(sed1375_device::fpline_start_r), FUNC(sed1375_device::fpline_start_w));
	map(0x1ffe8, 0x1ffe8).rw(FUNC(sed1375_device::hblank_r), FUNC(sed1375_device::hblank_w));
	map(0x1ffe9, 0x1ffe9).rw(FUNC(sed1375_device::fpframe_start_r), FUNC(sed1375_device::fpframe_start_w));
	map(0x1ffea, 0x1ffea).rw(FUNC(sed1375_device::vblank_r), FUNC(sed1375_device::vblank_w));
	map(0x1ffeb, 0x1ffeb).rw(FUNC(sed1375_device::mod_rate_r), FUNC(sed1375_device::mod_rate_w));
	map(0x1ffec, 0x1ffec).rw(FUNC(sed1375_device::screen1_start_lsb_r), FUNC(sed1375_device::screen1_start_lsb_w));
	map(0x1ffed, 0x1ffed).rw(FUNC(sed1375_device::screen1_start_msb_r), FUNC(sed1375_device::screen1_start_msb_w));
	map(0x1ffee, 0x1ffee).rw(FUNC(sed1375_device::screen2_start_lsb_r), FUNC(sed1375_device::screen2_start_lsb_w));
	map(0x1ffef, 0x1ffef).rw(FUNC(sed1375_device::screen2_start_msb_r), FUNC(sed1375_device::screen2_start_msb_w));
	map(0x1fff0, 0x1fff0).rw(FUNC(sed1375_device::screen1_start_ovf_r), FUNC(sed1375_device::screen1_start_ovf_w));
	map(0x1fff1, 0x1fff1).rw(FUNC(sed1375_device::mem_addr_offset_r), FUNC(sed1375_device::mem_addr_offset_w));
	map(0x1fff2, 0x1fff2).rw(FUNC(sed1375_device::screen1_vsize_lsb_r), FUNC(sed1375_device::screen1_vsize_lsb_w));
	map(0x1fff3, 0x1fff3).rw(FUNC(sed1375_device::screen1_vsize_msb_r), FUNC(sed1375_device::screen1_vsize_msb_w));
	map(0x1fff5, 0x1fff5).rw(FUNC(sed1375_device::lut_addr_r), FUNC(sed1375_device::lut_addr_w));
	map(0x1fff7, 0x1fff7).rw(FUNC(sed1375_device::lut_data_r), FUNC(sed1375_device::lut_data_w));
	map(0x1fff8, 0x1fff8).rw(FUNC(sed1375_device::gpio_config_r), FUNC(sed1375_device::gpio_config_w));
	map(0x1fff9, 0x1fff9).rw(FUNC(sed1375_device::gpio_r), FUNC(sed1375_device::gpio_w));
	map(0x1fffa, 0x1fffa).rw(FUNC(sed1375_device::scratch_r), FUNC(sed1375_device::scratch_w));
	map(0x1fffb, 0x1fffb).rw(FUNC(sed1375_device::swivel_mode_r), FUNC(sed1375_device::swivel_mode_w));
	map(0x1fffc, 0x1fffc).rw(FUNC(sed1375_device::swivel_bytecnt_r), FUNC(sed1375_device::swivel_bytecnt_w));
}

u32 sed1375_device::get_pixel(int screen_idx, int x, int y)
{
	const u32 pixels_per_line = (m_panel_hsize + 1) * 8;
	const u32 bpp = 1 << ((m_mode[1] & MODE1_BPP_MASK) >> MODE1_BPP_SHIFT);
	const u32 pixel_index = y * pixels_per_line + x;
	const u32 byte_index = (pixel_index * bpp) / 8;
	const u32 bit_index = x % (8 / bpp);
	const u8 bpp_mask = (1 << bpp) - 1;
	return (m_vram[(m_screen_start[screen_idx] << 1) + byte_index] >> (bit_index * bpp)) & bpp_mask;
}

u32 sed1375_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_mode[1], MODE1_BLANK_BIT))
	{
		bitmap.fill(0xff000000, cliprect);
		return 0;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u32 *dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			u32 pix = get_pixel(0, x, y);
			if (BIT(m_mode[0], MODE0_TFT_BIT) || BIT(m_mode[0], MODE0_COLOR_BIT))
			{
				const u8 r = m_red_lut[pix];
				const u8 g = m_green_lut[pix];
				const u8 b = m_blue_lut[pix];
				*dst++ = 0xff000000 | (((r << 4) | r) << 16) | (((g << 4) | g) << 8) | ((b << 4) | b);
			}
			else
			{
				switch ((m_mode[1] & MODE1_BPP_MASK) >> MODE1_BPP_SHIFT)
				{
					case 0: // 1bpp
						*dst++ = 0xff000000 | (pix * 0xffffff);
						break;
					case 1: // 2bpp
						*dst++ = 0xff000000 | (pix * 0x555555);
						break;
					case 2: // 4bpp
						*dst++ = 0xff000000 | (pix * 0x111111);
						break;
					case 3: // 8bpp
						*dst++ = 0xff000000 | (pix * 0x010101);
						break;
				}
			}
		}
	}
	return 0;
}

void sed1375_device::vram_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_VRAM_WRITE, "%s: vram_w: VRAM Write[%05x] = %02x\n", machine().describe_context(), offset, data);
	m_vram[offset] = data;
}

u8 sed1375_device::vram_r(offs_t offset)
{
	LOGMASKED(LOG_VRAM_READ, "%s: vram_r: VRAM Read[%05x]: %02x\n", machine().describe_context(), offset, m_vram[offset]);
	return m_vram[offset];
}

u8 sed1375_device::revision_r()
{
	LOGMASKED(LOG_REG_READ, "%s: revision_r: %02x\n", machine().describe_context(), m_revision);
	return m_revision;
}

void sed1375_device::mode0_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: mode0_w: Mode Register 0 = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s           Data Width Bits: %d\n", machine().describe_context(), data & MODE0_WIDTH_MASK);
	LOGMASKED(LOG_REG_WRITE, "%s           Mask FPSHIFT: %d\n", machine().describe_context(), BIT(data, MODE0_FPSHIFT_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           FPFRAME Polarity: %s\n", machine().describe_context(), BIT(data, MODE0_FPFRAME_POL_BIT) ? "Active-High" : "Active-Low");
	LOGMASKED(LOG_REG_WRITE, "%s           FPLINE Polarity: %s\n", machine().describe_context(), BIT(data, MODE0_FPLINE_POL_BIT) ? "Active-High" : "Active-Low");
	LOGMASKED(LOG_REG_WRITE, "%s           Color/Mono: %s\n", machine().describe_context(), BIT(data, MODE0_COLOR_BIT) ? "Color" : "Mono");
	LOGMASKED(LOG_REG_WRITE, "%s           Dual/Single: %s\n", machine().describe_context(), BIT(data, MODE0_DUAL_BIT) ? "Dual" : "Single");
	LOGMASKED(LOG_REG_WRITE, "%s           TFT/STN: %s\n", machine().describe_context(), BIT(data, MODE0_TFT_BIT) ? "TFT" : "STN");
	m_mode[0] = data & MODE0_MASK;
}

u8 sed1375_device::mode0_r()
{
	LOGMASKED(LOG_REG_READ, "%s: mode0_r: Mode Register 0: %02x\n", machine().describe_context(), m_mode[0]);
	return m_mode[0];
}

void sed1375_device::mode1_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: mode1_w: Mode Register 1 = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s           Software Video Invert: %d\n", machine().describe_context(), BIT(data, MODE1_SWINVERT_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           Hardware Video Invert: %d\n", machine().describe_context(), BIT(data, MODE1_HWINVERT_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           Frame Repeat: %d\n", machine().describe_context(), BIT(data, MODE1_FRREPEAT_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           Display Blank: %d\n", machine().describe_context(), BIT(data, MODE1_BLANK_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           Input Clock Divider: %d\n", machine().describe_context(), BIT(data, MODE1_CLKDIV_BIT) + 1);
	LOGMASKED(LOG_REG_WRITE, "%s           High Performance: %d\n", machine().describe_context(), BIT(data, MODE1_HIPERF_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           Bits Per Pixel: %d\n", machine().describe_context(), 1 << ((data & MODE1_BPP_MASK) >> MODE1_BPP_SHIFT));
	m_mode[1] = data & MODE1_MASK;
}

u8 sed1375_device::mode1_r()
{
	LOGMASKED(LOG_REG_READ, "%s: mode1_r: Mode Register 1: %02x\n", machine().describe_context(), m_mode[1]);
	return m_mode[1];
}

void sed1375_device::mode2_w(u8 data)
{
	static const char *const SWPWRSAVE_NAMES[4] = { "Software Power Save", "Reserved (1)", "Reserved (2)", "Normal Operation" };
	LOGMASKED(LOG_REG_WRITE, "%s: mode2_w: Mode Register 2 = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s           Software Power Save Mode: %s\n", machine().describe_context(), SWPWRSAVE_NAMES[data & MODE2_SWPWRSAVE_MASK]);
	LOGMASKED(LOG_REG_WRITE, "%s           Hardware Power Save: %d\n", machine().describe_context(), BIT(data, MODE2_HWPWRSAVE_BIT));
	LOGMASKED(LOG_REG_WRITE, "%s           LCDPWR Override: %d\n", machine().describe_context(), BIT(data, MODE2_LCDPWR_OVR_BIT));
	m_mode[2] = data & MODE2_MASK;
}

u8 sed1375_device::mode2_r()
{
	LOGMASKED(LOG_REG_READ, "%s: mode2_r: Mode Register 2: %02x\n", machine().describe_context(), m_mode[2]);
	return m_mode[2];
}

void sed1375_device::panel_hsize_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: panel_hsize_w: Panel Horizontal Size = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s:                In Pixels: %d\n", machine().describe_context(), ((data & PANEL_HSIZE_MASK) + 1) * 8);
	m_panel_hsize = data & PANEL_HSIZE_MASK;
}

u8 sed1375_device::panel_hsize_r()
{
	LOGMASKED(LOG_REG_READ, "%s: panel_hsize_r: Panel Horizontal Size: %02x\n", machine().describe_context(), m_panel_hsize);
	return m_panel_hsize;
}

void sed1375_device::panel_vsize_lsb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: panel_vsize_lsb_w: Panel Vertical Size (LSB) = %02x\n", machine().describe_context(), data);
	m_panel_vsize = ((m_panel_vsize & 0xff00) | data) & PANEL_VSIZE_MASK;
}

u8 sed1375_device::panel_vsize_lsb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: panel_vsize_lsb_r: Panel Vertical Size (LSB): %02x\n", machine().describe_context(), m_panel_vsize & 0x00ff);
	return m_panel_vsize;
}

void sed1375_device::panel_vsize_msb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: panel_vsize_msb_w: Panel Vertical Size (MSB) = %02x\n", machine().describe_context(), data);
	m_panel_vsize = ((m_panel_vsize & 0x00ff) | (data << 8)) & PANEL_VSIZE_MASK;
}

u8 sed1375_device::panel_vsize_msb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: panel_vsize_msb_r: Panel Vertical Size (MSB): %02x\n", machine().describe_context(), m_panel_vsize >> 8);
	return m_panel_vsize >> 8;
}

void sed1375_device::fpline_start_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: fpline_start_w: FPLINE Start Position = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s:                 In Pixels: %d\n", machine().describe_context(), ((data & FPLINE_START_MASK) + 2) * 8);
	m_fpline_start = data & FPLINE_START_MASK;
}

u8 sed1375_device::fpline_start_r()
{
	LOGMASKED(LOG_REG_READ, "%s: fpline_start_r: FPLINE Start Position: %02x\n", machine().describe_context(), m_fpline_start);
	return m_fpline_start;
}

void sed1375_device::hblank_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: hblank_w: Horizontal Blanking Period = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s:           In Pixels: %d\n", machine().describe_context(), ((data & HBLANK_MASK) + 4) * 8);
	m_hblank_period = data & HBLANK_MASK;
}

u8 sed1375_device::hblank_r()
{
	LOGMASKED(LOG_REG_READ, "%s: hblank_r: Horizontal Blanking Period: %02x\n", machine().describe_context(), m_hblank_period);
	return m_hblank_period;
}

void sed1375_device::fpframe_start_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: fpline_start_w: FPFRAME Start Position = %02x\n", machine().describe_context(), data);
	m_fpframe_start = data & FPFRAME_START_MASK;
}

u8 sed1375_device::fpframe_start_r()
{
	LOGMASKED(LOG_REG_READ, "%s: fpframe_start_r: FPFRAME Start Position: %02x\n", machine().describe_context(), m_fpframe_start);
	return m_fpframe_start;
}

void sed1375_device::vblank_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: vblank_w: Vertical Blanking Period = %02x\n", machine().describe_context(), data);
	m_vblank_period = data & VBLANK_MASK;
}

u8 sed1375_device::vblank_r()
{
	const u8 data = m_vblank_period | (screen().vblank() ? (1 << VBLANK_VBL_BIT) : 0);
	LOGMASKED(LOG_VBL_READ, "%s: vblank_r: Vertical Blanking Period/Status: %02x\n", machine().describe_context(), data);
	return data;
}

void sed1375_device::mod_rate_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: mod_rate_w: MOD Rate Register = %02x\n", machine().describe_context(), data);
	m_mod_rate = data & MODRATE_MASK;
}

u8 sed1375_device::mod_rate_r()
{
	LOGMASKED(LOG_REG_READ, "%s: mod_rate_r: MOD Rate Register: %02x\n", machine().describe_context(), m_mod_rate);
	return m_mod_rate;
}

void sed1375_device::screen2_start_lsb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen2_start_w: Screen 2 Start Address (LSB) = %02x\n", machine().describe_context(), data);
	m_screen_start[1] = ((m_screen_start[1] & 0xff00) | data) & SCREEN2_START_MASK;
}

u8 sed1375_device::screen2_start_lsb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen2_start_r: Screen 2 Start Address (LSB): %02x\n", machine().describe_context(), (u8)m_screen_start[1]);
	return (u8)m_screen_start[1];
}

void sed1375_device::screen1_start_msb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen1_start_w: Screen 1 Start Address (MSB) = %02x\n", machine().describe_context(), data);
	m_screen_start[0] = ((m_screen_start[0] & 0x00ff) | (data << 8)) & SCREEN1_START_MASK;
}

u8 sed1375_device::screen1_start_msb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen1_start_r: Screen 1 Start Address (MSB): %02x\n", machine().describe_context(), (u8)(m_screen_start[0] >> 8));
	return (u8)(m_screen_start[0] >> 8);
}

void sed1375_device::screen1_start_lsb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen1_start_w: Screen 1 Start Address (LSB) = %02x\n", machine().describe_context(), data);
	m_screen_start[0] = ((m_screen_start[0] & 0xff00) | data) & SCREEN1_START_MASK;
}

u8 sed1375_device::screen1_start_lsb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen1_start_r: Screen 1 Start Address (LSB): %02x\n", machine().describe_context(), (u8)m_screen_start[0]);
	return (u8)m_screen_start[0];
}

void sed1375_device::screen2_start_msb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen2_start_w: Screen 2 Start Address (MSB) = %02x\n", machine().describe_context(), data);
	m_screen_start[1] = ((m_screen_start[1] & 0x00ff) | (data << 8)) & SCREEN2_START_MASK;
}

u8 sed1375_device::screen2_start_msb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen2_start_r: Screen 2 Start Address (MSB): %02x\n", machine().describe_context(), (u8)(m_screen_start[1] >> 8));
	return (u8)(m_screen_start[1] >> 8);
}

void sed1375_device::screen1_start_ovf_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen1_start_ovf_w: Screen Start Address Overflow Register = %02x\n", machine().describe_context(), data);
	m_screen_start_ovf = data & START_OVF_MASK;
}

u8 sed1375_device::screen1_start_ovf_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen1_start_ovf_r: Screen Start Address Overflow Register: %02x\n", machine().describe_context(), m_screen_start_ovf);
	return m_screen_start_ovf;
}

void sed1375_device::mem_addr_offset_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: mem_addr_offset_w: Memory Address Offset Register = %02x\n", machine().describe_context(), data);
	m_mem_addr_offset = data & MEM_ADDR_OFFSET_MASK;
}

u8 sed1375_device::mem_addr_offset_r()
{
	LOGMASKED(LOG_REG_READ, "%s: mem_addr_offset_r: Memory Address Offset Register: %02x\n", machine().describe_context(), m_mem_addr_offset);
	return m_mem_addr_offset;
}

void sed1375_device::screen1_vsize_lsb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen1_vsize_lsb_w: Screen 1 Vertical Size Register (LSB) = %02x\n", machine().describe_context(), data);
	m_screen1_vsize = ((m_screen1_vsize & 0xff00) | data) & SCREEN1_VSIZE_MASK;
}

u8 sed1375_device::screen1_vsize_lsb_r()
{
	LOGMASKED(LOG_REG_READ, "%s: screen1_vsize_lsb_r: Screen 1 Vertical Size Register (LSB): %02x\n", machine().describe_context(), (u8)m_screen1_vsize);
	return (u8)m_screen1_vsize;
}

void sed1375_device::screen1_vsize_msb_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: screen1_vsize_msb_w: Screen 1 Vertical Size Register (MSB) = %02x\n", machine().describe_context(), data);
	m_screen1_vsize = ((m_screen1_vsize & 0x00ff) | (data << 8)) & SCREEN1_VSIZE_MASK;
}

u8 sed1375_device::screen1_vsize_msb_r()
{
	LOGMASKED(LOG_LUT_READ, "%s: screen1_vsize_msb_r: Screen 1 Vertical Size Register (MSB): %02x\n", machine().describe_context(), (u8)(m_screen1_vsize >> 8));
	return (u8)(m_screen1_vsize >> 8);
}

void sed1375_device::lut_addr_w(u8 data)
{
	LOGMASKED(LOG_LUT_WRITE, "%s: lut_addr_w: LUT Address Register = %02x\n", machine().describe_context(), data);
	m_lut_addr = data & LUT_ADDR_MASK;
	m_lut_index = 0;
}

u8 sed1375_device::lut_addr_r()
{
	LOGMASKED(LOG_LUT_READ, "%s: lut_addr_r: LUT Address Register: %02x\n", machine().describe_context(), m_lut_addr);
	return m_lut_addr;
}

void sed1375_device::lut_data_w(u8 data)
{
	static const char *const LUT_INDEX_NAMES[3] = { "Red", "Green", "Blue" };
	LOGMASKED(LOG_LUT_WRITE, "%s: lut_data_w: LUT Data[%d] = %02x (%s)\n", machine().describe_context(), (u32)m_lut_addr, data, LUT_INDEX_NAMES[m_lut_index]);

	u8 *lut_data[3] = { m_red_lut, m_green_lut, m_blue_lut };
	lut_data[m_lut_index++][m_lut_addr] = (data & LUT_DATA_MASK) >> LUT_DATA_SHIFT;
	if (m_lut_index >= 3)
	{
		m_lut_addr++;
		m_lut_index = 0;
	}
}

u8 sed1375_device::lut_data_r()
{
	static const char *const LUT_INDEX_NAMES[3] = { "Red", "Green", "Blue" };
	u8 *lut_data[3] = { m_red_lut, m_green_lut, m_blue_lut };
	const u8 data = lut_data[m_lut_index++][m_lut_addr];
	LOGMASKED(LOG_REG_READ, "%s: lut_data_r: LUT Data[%d]: %02x (%s)\n", machine().describe_context(), (u32)m_lut_addr, data, LUT_INDEX_NAMES[m_lut_index]);
	if (m_lut_index >= 3)
	{
		m_lut_addr++;
		m_lut_index = 0;
	}
	return data;
}

void sed1375_device::gpio_config_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: gpio_config_w: GPIO Config Register = %02x\n", machine().describe_context(), data);
	m_gpio_config = data & GPIO_CONFIG_MASK;
}

u8 sed1375_device::gpio_config_r()
{
	LOGMASKED(LOG_REG_READ, "%s: gpio_config_r: GPIO Config Register: %02x\n", machine().describe_context(), m_gpio_config);
	return m_gpio_config;
}

void sed1375_device::gpio_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: gpio_w: GPIO Output = %02x\n", machine().describe_context(), data);
	m_gpio_status = data & GPIO_STATUS_MASK;
}

u8 sed1375_device::gpio_r()
{
	LOGMASKED(LOG_REG_READ, "%s: gpio_r: GPIO Input: %02x\n", machine().describe_context(), m_gpio_status);
	return m_gpio_status;
}

void sed1375_device::scratch_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: scratch_w: Scratch Pad Register = %02x\n", machine().describe_context(), data);
	m_scratch = data & SCRATCH_MASK;
}

u8 sed1375_device::scratch_r()
{
	LOGMASKED(LOG_REG_READ, "%s: scratch_r: Scratch Pad Register: %02x\n", machine().describe_context(), m_scratch);
	return m_scratch;
}

void sed1375_device::swivel_mode_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: swivel_mode_w: SwivelView Mode Register = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_REG_WRITE, "%s:                Pixel Clock Select Bits: %d\n", machine().describe_context(), data & SWIVEL_CLKSEL_MASK);
	LOGMASKED(LOG_REG_WRITE, "%s:                SwivelView Mode Select: %d\n", machine().describe_context(),
		!BIT(data, SWIVEL_ENABLE_BIT) ? "Landscape" : (BIT(data, SWIVEL_MODE_BIT) ? "Alternate" : "Default"));
	LOGMASKED(LOG_REG_WRITE, "%s:                SwivelView Mode Enable: %d\n", machine().describe_context(), BIT(data, SWIVEL_ENABLE_BIT));
	m_swivel_mode = data & SWIVEL_MODE_MASK;
}

u8 sed1375_device::swivel_mode_r()
{
	LOGMASKED(LOG_REG_READ, "%s: swivel_mode_r: SwivelView Mode Register: %02x\n", machine().describe_context(), m_swivel_mode);
	return m_swivel_mode;
}

void sed1375_device::swivel_bytecnt_w(u8 data)
{
	LOGMASKED(LOG_REG_WRITE, "%s: swivel_bytecnt_w: SwivelView Line Byte Count = %02x\n", machine().describe_context(), data);
	m_swivel_bytecnt = data & SWIVEL_BYTECNT_MASK;
}

u8 sed1375_device::swivel_bytecnt_r()
{
	LOGMASKED(LOG_REG_READ, "%s: swivel_bytecnt_r: SwivelView Line Byte Count: %02x\n", machine().describe_context(), m_swivel_bytecnt);
	return m_swivel_bytecnt;
}
