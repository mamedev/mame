// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Epson SED1356 LCD Controller emulation skeleton

    Status:
    - Currently hard-coded for use with the Jornada 720 driver.
    - Register contents are correctly stored, logged and masked, but
      register handling is otherwise largely non-present.
    - There is the potential for endian issues, and it will be
      dealt with once the Jornada 720 driver makes heavier use of
      BitBLT operations or other operations relevant to endianness.

**********************************************************************/

#include "emu.h"
#include "sed1356.h"
#include "screen.h"

#define LOG_MISC_RD     (1U << 1)
#define LOG_MISC_WR     (1U << 2)
#define LOG_LCD_RD      (1U << 3)
#define LOG_LCD_WR      (1U << 4)
#define LOG_CRT_RD      (1U << 5)
#define LOG_CRT_WR      (1U << 6)
#define LOG_BITBLT_RD   (1U << 7)
#define LOG_BITBLT_WR   (1U << 8)
#define LOG_BITBLT_OP   (1U << 9)
#define LOG_LUT_RD      (1U << 10)
#define LOG_LUT_WR      (1U << 11)
#define LOG_MPLUG_RD    (1U << 12)
#define LOG_MPLUG_WR    (1U << 13)
#define LOG_LCD_RD_HF   (1U << 14)
#define LOG_ALL         (LOG_MISC_RD | LOG_MISC_WR | LOG_LCD_RD | LOG_LCD_WR | LOG_CRT_RD | LOG_CRT_WR | LOG_BITBLT_RD | LOG_BITBLT_WR | LOG_BITBLT_OP | LOG_LUT_RD \
						| LOG_LUT_WR | LOG_MPLUG_RD | LOG_MPLUG_WR)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SED1356, sed1356_device, "sed1356", "Epson SED1356")

sed1356_device::sed1356_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SED1356, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_vram(*this, "vram")
{
}

void sed1356_device::map(address_map &map)
{
	map(0x000000, 0x000000).r(FUNC(sed1356_device::revision_r));
	map(0x000001, 0x000001).rw(FUNC(sed1356_device::misc_r), FUNC(sed1356_device::misc_w));
	map(0x000004, 0x000004).rw(FUNC(sed1356_device::gpio_config_r), FUNC(sed1356_device::gpio_config_w));
	map(0x000008, 0x000008).rw(FUNC(sed1356_device::gpio_ctrl_r), FUNC(sed1356_device::gpio_ctrl_w));
	map(0x00000c, 0x00000d).rw(FUNC(sed1356_device::md_config_readback_r), FUNC(sed1356_device::md_config_readback_w));
	map(0x000010, 0x000010).rw(FUNC(sed1356_device::mem_clk_config_r), FUNC(sed1356_device::mem_clk_config_w));
	map(0x000014, 0x000014).rw(FUNC(sed1356_device::lcd_pclk_config_r), FUNC(sed1356_device::lcd_pclk_config_w));
	map(0x000018, 0x000018).rw(FUNC(sed1356_device::crt_pclk_config_r), FUNC(sed1356_device::crt_pclk_config_w));
	map(0x00001c, 0x00001c).rw(FUNC(sed1356_device::mplug_clk_config_r), FUNC(sed1356_device::mplug_clk_config_w));
	map(0x00001e, 0x00001e).rw(FUNC(sed1356_device::mem_wait_select_r), FUNC(sed1356_device::mem_wait_select_w));
	map(0x000020, 0x000020).rw(FUNC(sed1356_device::mem_config_r), FUNC(sed1356_device::mem_config_w));
	map(0x000021, 0x000021).rw(FUNC(sed1356_device::dram_refresh_select_r), FUNC(sed1356_device::dram_refresh_select_w));
	map(0x00002a, 0x00002b).r(FUNC(sed1356_device::dram_timing_ctrl_r));
	map(0x00002a, 0x00002a).w(FUNC(sed1356_device::dram_timing_ctrl0_w));
	map(0x00002b, 0x00002b).w(FUNC(sed1356_device::dram_timing_ctrl1_w));
	map(0x000030, 0x000030).rw(FUNC(sed1356_device::panel_type_r), FUNC(sed1356_device::panel_type_w));
	map(0x000031, 0x000031).rw(FUNC(sed1356_device::mod_rate_r), FUNC(sed1356_device::mod_rate_w));
	map(0x000032, 0x000032).rw(FUNC(sed1356_device::lcd_display_width_r), FUNC(sed1356_device::lcd_display_width_w));
	map(0x000034, 0x000034).rw(FUNC(sed1356_device::lcd_hblank_period_r), FUNC(sed1356_device::lcd_hblank_period_w));
	map(0x000035, 0x000035).rw(FUNC(sed1356_device::tft_fpline_start_pos_r), FUNC(sed1356_device::tft_fpline_start_pos_w));
	map(0x000036, 0x000036).rw(FUNC(sed1356_device::tft_fpline_pulse_width_r), FUNC(sed1356_device::tft_fpline_pulse_width_w));
	map(0x000038, 0x000039).rw(FUNC(sed1356_device::lcd_display_height_r), FUNC(sed1356_device::lcd_display_height_w));
	map(0x00003a, 0x00003a).rw(FUNC(sed1356_device::lcd_vblank_period_r), FUNC(sed1356_device::lcd_vblank_period_w));
	map(0x00003b, 0x00003b).rw(FUNC(sed1356_device::tft_fpframe_start_pos_r), FUNC(sed1356_device::tft_fpframe_start_pos_w));
	map(0x00003c, 0x00003c).rw(FUNC(sed1356_device::tft_fpframe_pulse_width_r), FUNC(sed1356_device::tft_fpframe_pulse_width_w));
	map(0x000040, 0x000040).rw(FUNC(sed1356_device::lcd_display_mode_r), FUNC(sed1356_device::lcd_display_mode_w));
	map(0x000041, 0x000041).rw(FUNC(sed1356_device::lcd_misc_r), FUNC(sed1356_device::lcd_misc_w));
	map(0x000042, 0x000044).rw(FUNC(sed1356_device::lcd_display_start_addr_r), FUNC(sed1356_device::lcd_display_start_addr_w));
	map(0x000046, 0x000047).rw(FUNC(sed1356_device::lcd_mem_addr_offset_r), FUNC(sed1356_device::lcd_mem_addr_offset_w));
	map(0x000048, 0x000048).rw(FUNC(sed1356_device::lcd_pixel_pan_r), FUNC(sed1356_device::lcd_pixel_pan_w));
	map(0x00004a, 0x00004a).rw(FUNC(sed1356_device::lcd_display_fifo_hi_thresh_r), FUNC(sed1356_device::lcd_display_fifo_hi_thresh_w));
	map(0x00004b, 0x00004b).rw(FUNC(sed1356_device::lcd_display_fifo_lo_thresh_r), FUNC(sed1356_device::lcd_display_fifo_lo_thresh_w));
	map(0x000050, 0x000050).rw(FUNC(sed1356_device::crt_display_width_r), FUNC(sed1356_device::crt_display_width_w));
	map(0x000052, 0x000052).rw(FUNC(sed1356_device::crt_hblank_period_r), FUNC(sed1356_device::crt_hblank_period_w));
	map(0x000053, 0x000053).rw(FUNC(sed1356_device::crt_hrtc_start_pos_r), FUNC(sed1356_device::crt_hrtc_start_pos_w));
	map(0x000054, 0x000054).rw(FUNC(sed1356_device::crt_hrtc_pulse_width_r), FUNC(sed1356_device::crt_hrtc_pulse_width_w));
	map(0x000056, 0x000057).rw(FUNC(sed1356_device::crt_display_height_r), FUNC(sed1356_device::crt_display_height_w));
	map(0x000058, 0x000058).rw(FUNC(sed1356_device::crt_vblank_period_r), FUNC(sed1356_device::crt_vblank_period_w));
	map(0x000059, 0x000059).rw(FUNC(sed1356_device::crt_vrtc_start_pos_r), FUNC(sed1356_device::crt_vrtc_start_pos_w));
	map(0x00005a, 0x00005a).rw(FUNC(sed1356_device::crt_vrtc_pulse_width_r), FUNC(sed1356_device::crt_vrtc_pulse_width_w));
	map(0x00005b, 0x00005b).rw(FUNC(sed1356_device::crt_output_ctrl_r), FUNC(sed1356_device::crt_output_ctrl_w));
	map(0x000060, 0x000060).rw(FUNC(sed1356_device::crt_display_mode_r), FUNC(sed1356_device::crt_display_mode_w));
	map(0x000062, 0x000064).rw(FUNC(sed1356_device::crt_display_start_addr_r), FUNC(sed1356_device::crt_display_start_addr_w));
	map(0x000066, 0x000067).rw(FUNC(sed1356_device::crt_mem_addr_offset_r), FUNC(sed1356_device::crt_mem_addr_offset_w));
	map(0x000068, 0x000068).rw(FUNC(sed1356_device::crt_pixel_pan_r), FUNC(sed1356_device::crt_pixel_pan_w));
	map(0x00006a, 0x00006a).rw(FUNC(sed1356_device::crt_display_fifo_hi_thresh_r), FUNC(sed1356_device::crt_display_fifo_hi_thresh_w));
	map(0x00006b, 0x00006b).rw(FUNC(sed1356_device::crt_display_fifo_lo_thresh_r), FUNC(sed1356_device::crt_display_fifo_lo_thresh_w));
	map(0x000070, 0x000070).rw(FUNC(sed1356_device::lcd_cursor_ctrl_r), FUNC(sed1356_device::lcd_cursor_ctrl_w));
	map(0x000071, 0x000071).rw(FUNC(sed1356_device::lcd_cursor_start_addr_r), FUNC(sed1356_device::lcd_cursor_start_addr_w));
	map(0x000072, 0x000073).rw(FUNC(sed1356_device::lcd_cursor_x_r), FUNC(sed1356_device::lcd_cursor_x_w));
	map(0x000074, 0x000075).rw(FUNC(sed1356_device::lcd_cursor_y_r), FUNC(sed1356_device::lcd_cursor_y_w));
	map(0x000076, 0x000078).r(FUNC(sed1356_device::lcd_cursor_color0_r));
	map(0x000076, 0x000076).w(FUNC(sed1356_device::lcd_cursor_color0_blu_w));
	map(0x000077, 0x000077).w(FUNC(sed1356_device::lcd_cursor_color0_grn_w));
	map(0x000078, 0x000078).w(FUNC(sed1356_device::lcd_cursor_color0_red_w));
	map(0x00007a, 0x00007b).r(FUNC(sed1356_device::lcd_cursor_color1_r));
	map(0x00007a, 0x00007a).w(FUNC(sed1356_device::lcd_cursor_color1_blu_w));
	map(0x00007b, 0x00007b).w(FUNC(sed1356_device::lcd_cursor_color1_grn_w));
	map(0x00007c, 0x00007c).w(FUNC(sed1356_device::lcd_cursor_color1_red_w));
	map(0x00007e, 0x00007e).rw(FUNC(sed1356_device::lcd_cursor_fifo_thresh_r), FUNC(sed1356_device::lcd_cursor_fifo_thresh_w));
	map(0x000080, 0x000080).rw(FUNC(sed1356_device::crt_cursor_ctrl_r), FUNC(sed1356_device::crt_cursor_ctrl_w));
	map(0x000081, 0x000081).rw(FUNC(sed1356_device::crt_cursor_start_addr_r), FUNC(sed1356_device::crt_cursor_start_addr_w));
	map(0x000082, 0x000083).rw(FUNC(sed1356_device::crt_cursor_x_r), FUNC(sed1356_device::crt_cursor_x_w));
	map(0x000084, 0x000085).rw(FUNC(sed1356_device::crt_cursor_y_r), FUNC(sed1356_device::crt_cursor_y_w));
	map(0x000086, 0x000088).r(FUNC(sed1356_device::crt_cursor_color0_r));
	map(0x000086, 0x000086).w(FUNC(sed1356_device::crt_cursor_color0_blu_w));
	map(0x000087, 0x000087).w(FUNC(sed1356_device::crt_cursor_color0_grn_w));
	map(0x000088, 0x000088).w(FUNC(sed1356_device::crt_cursor_color0_red_w));
	map(0x00008a, 0x00008b).r(FUNC(sed1356_device::crt_cursor_color1_r));
	map(0x00008a, 0x00008a).w(FUNC(sed1356_device::crt_cursor_color1_blu_w));
	map(0x00008b, 0x00008b).w(FUNC(sed1356_device::crt_cursor_color1_grn_w));
	map(0x00008c, 0x00008c).w(FUNC(sed1356_device::crt_cursor_color1_red_w));
	map(0x00008e, 0x00008e).rw(FUNC(sed1356_device::crt_cursor_fifo_thresh_r), FUNC(sed1356_device::crt_cursor_fifo_thresh_w));
	map(0x000100, 0x000100).rw(FUNC(sed1356_device::bitblt_ctrl0_r), FUNC(sed1356_device::bitblt_ctrl0_w));
	map(0x000101, 0x000101).rw(FUNC(sed1356_device::bitblt_ctrl1_r), FUNC(sed1356_device::bitblt_ctrl1_w));
	map(0x000102, 0x000102).rw(FUNC(sed1356_device::bitblt_rop_code_r), FUNC(sed1356_device::bitblt_rop_code_w));
	map(0x000103, 0x000103).rw(FUNC(sed1356_device::bitblt_operation_r), FUNC(sed1356_device::bitblt_operation_w));
	map(0x000104, 0x000106).rw(FUNC(sed1356_device::bitblt_src_start_addr_r), FUNC(sed1356_device::bitblt_src_start_addr_w));
	map(0x000108, 0x00010a).rw(FUNC(sed1356_device::bitblt_dst_start_addr_r), FUNC(sed1356_device::bitblt_dst_start_addr_w));
	map(0x00010c, 0x00010d).rw(FUNC(sed1356_device::bitblt_mem_addr_offset_r), FUNC(sed1356_device::bitblt_mem_addr_offset_w));
	map(0x000110, 0x000111).rw(FUNC(sed1356_device::bitblt_width_r), FUNC(sed1356_device::bitblt_width_w));
	map(0x000112, 0x000113).rw(FUNC(sed1356_device::bitblt_height_r), FUNC(sed1356_device::bitblt_height_w));
	map(0x000114, 0x000115).rw(FUNC(sed1356_device::bitblt_bgcolor_r), FUNC(sed1356_device::bitblt_bgcolor_w));
	map(0x000118, 0x000119).rw(FUNC(sed1356_device::bitblt_fgcolor_r), FUNC(sed1356_device::bitblt_fgcolor_w));
	map(0x0001e0, 0x0001e0).rw(FUNC(sed1356_device::lut_mode_r), FUNC(sed1356_device::lut_mode_w));
	map(0x0001e2, 0x0001e2).rw(FUNC(sed1356_device::lut_addr_r), FUNC(sed1356_device::lut_addr_w));
	map(0x0001e4, 0x0001e4).rw(FUNC(sed1356_device::lut_data_r), FUNC(sed1356_device::lut_data_w));
	map(0x0001f0, 0x0001f0).rw(FUNC(sed1356_device::power_config_r), FUNC(sed1356_device::power_config_w));
	map(0x0001f1, 0x0001f1).r(FUNC(sed1356_device::power_status_r));
	map(0x0001f4, 0x0001f4).rw(FUNC(sed1356_device::cpu_mem_watchdog_r), FUNC(sed1356_device::cpu_mem_watchdog_w));
	map(0x0001fc, 0x0001fc).rw(FUNC(sed1356_device::display_mode_r), FUNC(sed1356_device::display_mode_w));
	map(0x001000, 0x001001).rw(FUNC(sed1356_device::mplug_lcmd_r), FUNC(sed1356_device::mplug_lcmd_w));
	map(0x001002, 0x001003).rw(FUNC(sed1356_device::mplug_resv_lcmd_r), FUNC(sed1356_device::mplug_resv_lcmd_w));
	map(0x001004, 0x001005).rw(FUNC(sed1356_device::mplug_cmd_r), FUNC(sed1356_device::mplug_cmd_w));
	map(0x001006, 0x001007).rw(FUNC(sed1356_device::mplug_resv_cmd_r), FUNC(sed1356_device::mplug_resv_cmd_w));
	map(0x001008, 0x001fff).rw(FUNC(sed1356_device::mplug_data_r), FUNC(sed1356_device::mplug_data_w));
	map(0x100000, 0x1fffff).rw(FUNC(sed1356_device::bitblt_data_r), FUNC(sed1356_device::bitblt_data_w));
}

void sed1356_device::vram_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram().share(m_vram);
}

uint8_t sed1356_device::revision_r(offs_t offset)
{
	const uint8_t data = 0x11;
	LOGMASKED(LOG_MISC_RD, "%s: revision_r: %02x\n", machine().describe_context(), data);
	return data;
}


uint8_t sed1356_device::misc_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: misc_r: %02x\n", machine().describe_context(), m_misc);
	return m_misc;
}

void sed1356_device::misc_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: misc_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:          Register/Memory Select: %d\n", machine().describe_context(), BIT(data, MISC_RMS_BIT));
	m_misc = data & MISC_MASK;
}


uint8_t sed1356_device::gpio_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: gpio_config_r: %02x\n", machine().describe_context(), m_gpio_config);
	return m_gpio_config;
}

void sed1356_device::gpio_config_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: gpio_config_w = %02x\n", machine().describe_context(), data);
	m_gpio_config = data & GPCFG_MASK;
}

uint8_t sed1356_device::gpio_ctrl_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: gpio_ctrl_r: %02x\n", machine().describe_context(), m_gpio_ctrl);
	return m_gpio_ctrl;
}

void sed1356_device::gpio_ctrl_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: gpio_ctrl_w = %02x\n", machine().describe_context(), data);
	m_gpio_ctrl = data & GPCTRL_MASK;
}


uint8_t sed1356_device::md_config_readback_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: md_config_readback_r[%d]: %02x\n", machine().describe_context(), offset, m_md_config[offset]);
	return m_md_config[offset];
}

void sed1356_device::md_config_readback_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: md_config_readback_w[%d] = %02x\n", machine().describe_context(), offset, data);
	m_md_config[offset] = data;
}


uint8_t sed1356_device::mem_clk_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: mem_clk_config_r: %02x\n", machine().describe_context(), m_memclk_config);
	return m_memclk_config;
}

uint8_t sed1356_device::lcd_pclk_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: lcd_pclk_config_r: %02x\n", machine().describe_context(), m_lcd_pclk_config);
	return m_lcd_pclk_config;
}

uint8_t sed1356_device::crt_pclk_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: crt_pclk_config_r: %02x\n", machine().describe_context(), m_crt_pclk_config);
	return m_crt_pclk_config;
}

uint8_t sed1356_device::mplug_clk_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: mplug_clk_config_r: %02x\n", machine().describe_context(), m_mplug_clk_config);
	return m_mplug_clk_config;
}

void sed1356_device::mem_clk_config_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: mem_clk_config_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                    MCLK Source: %s\n", machine().describe_context(), BIT(data, MEMCLK_SRC_BIT) ? "BUSCLK" : "CLKI");
	LOGMASKED(LOG_MISC_WR, "%s:                    MCLK Divider: %d:1\n", machine().describe_context(), BIT(data, MEMCLK_DIV_BIT) + 1);
	m_memclk_config = data & MEMCLK_MASK;
}

void sed1356_device::lcd_pclk_config_w(offs_t offset, uint8_t data)
{
	static const char *const s_source_names[4] = { "CLKI", "BUSCLK", "CLKI2", "MCLK" };
	LOGMASKED(LOG_MISC_WR, "%s: lcd_pclk_config_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                     LCD PCLK Source: %s\n", machine().describe_context(), s_source_names[(data & LCDCLK_SRC_MASK) >> LCDCLK_SRC_BIT]);
	LOGMASKED(LOG_MISC_WR, "%s:                     LCD PCLK Divider: %d:1\n", machine().describe_context(), ((data & LCDCLK_DIV_MASK) >> LCDCLK_DIV_BIT) + 1);
	m_lcd_pclk_config = data & LCDCLK_MASK;
}

void sed1356_device::crt_pclk_config_w(offs_t offset, uint8_t data)
{
	static const char *const s_source_names[4] = { "CLKI", "BUSCLK", "CLKI2", "MCLK" };
	LOGMASKED(LOG_MISC_WR, "%s: crt_pclk_config_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                     CRT PCLK Source: %s\n", machine().describe_context(), s_source_names[(data & CRTCLK_SRC_MASK) >> CRTCLK_SRC_BIT]);
	LOGMASKED(LOG_MISC_WR, "%s:                     CRT PCLK Divider: %d:1\n", machine().describe_context(), ((data & CRTCLK_DIV_MASK) >> CRTCLK_DIV_BIT) + 1);
	LOGMASKED(LOG_MISC_WR, "%s:                     CRT PCLK 2x Enable: %d\n", machine().describe_context(), BIT(data, CRTCLK_2X_BIT));
	m_crt_pclk_config = data & CRTCLK_MASK;
}

void sed1356_device::mplug_clk_config_w(offs_t offset, uint8_t data)
{
	static const char *const s_source_names[4] = { "CLKI", "BUSCLK", "CLKI2", "MCLK" };
	LOGMASKED(LOG_MISC_WR, "%s: mplug_clk_config_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                      MediaPlug CLK Source: %s\n", machine().describe_context(), s_source_names[(data & PLUGCLK_SRC_MASK) >> PLUGCLK_SRC_BIT]);
	LOGMASKED(LOG_MISC_WR, "%s:                      MediaPlug CLK Divider: %d:1\n", machine().describe_context(), ((data & PLUGCLK_DIV_MASK) >> PLUGCLK_DIV_BIT) + 1);
	m_mplug_clk_config = data & PLUGCLK_MASK;
}


uint8_t sed1356_device::mem_wait_select_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: mem_wait_select_r: %02x\n", machine().describe_context(), m_wait_state_sel);
	return m_wait_state_sel;
}

uint8_t sed1356_device::mem_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: mem_config_r: %02x\n", machine().describe_context(), m_mem_config);
	return m_mem_config;
}

uint8_t sed1356_device::dram_refresh_select_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: dram_refresh_select_r: %02x\n", machine().describe_context(), m_dram_refresh);
	return m_dram_refresh;
}

uint8_t sed1356_device::dram_timing_ctrl_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: dram_timing_ctrl_r[%d]: %02x\n", machine().describe_context(), offset, m_dram_timing);
	return m_dram_timing[offset];
}

void sed1356_device::mem_wait_select_w(offs_t offset, uint8_t data)
{
	static const char *const s_sel_names[4] = { "no restrictions", "2 * MCLK - 4ns", "MCLK - 4ns", "Reserved" };
	LOGMASKED(LOG_MISC_WR, "%s: mem_wait_select_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                     Mode: %s\n", machine().describe_context(), s_sel_names[(data & CPUWAIT_SEL_MASK) >> CPUWAIT_SEL_BIT]);
	m_wait_state_sel = data & CPUWAIT_MASK;
}

void sed1356_device::mem_config_w(offs_t offset, uint8_t data)
{
	static const char *const s_type_names[4] = { "EDO-DRAM w/ 2-CAS#", "FPM-DRAM w/ 2-CAS#", "EDO-DRAM w/ 2-WE#", "FPM-DRAM w/ 2-WE#" };
	LOGMASKED(LOG_MISC_WR, "%s: mem_config_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                Type: %s\n", machine().describe_context(), s_type_names[(data & MEMCFG_TYPE_MASK) >> MEMCFG_TYPE_BIT]);
	m_mem_config = data & MEMCFG_MASK;
}

void sed1356_device::dram_refresh_select_w(offs_t offset, uint8_t data)
{
	static const char *const s_sel_names[4] = { "CBR Refresh", "Self-Refresh", "No Refresh [2]", "No Refresh [3]" };
	LOGMASKED(LOG_MISC_WR, "%s: dram_refresh_select_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                         Rate: MCLK/%d\n", machine().describe_context(), 64 << ((data & MEMRFSH_RATE_MASK) >> MEMRFSH_RATE_BIT));
	LOGMASKED(LOG_MISC_WR, "%s:                         Type: %s\n", machine().describe_context(), s_sel_names[(data & MEMRFSH_SEL_MASK) >> MEMRFSH_SEL_BIT]);
	m_dram_refresh = data & MEMRFSH_MASK;
}

void sed1356_device::dram_timing_ctrl0_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: dram_timing_ctrl_w[0] = %02x\n", machine().describe_context(), data);
	m_dram_timing[0] = data;
}

void sed1356_device::dram_timing_ctrl1_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: dram_timing_ctrl_w[1] = %02x\n", machine().describe_context(), data);
	m_dram_timing[1] = data & DRAMTIME1_MASK;
}

uint8_t sed1356_device::panel_type_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: panel_type_r: %02x\n", machine().describe_context(), m_panel_type);
	return m_panel_type;
}

uint8_t sed1356_device::mod_rate_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: mod_rate_r: %02x\n", machine().describe_context(), m_mod_rate);
	return m_mod_rate;
}

void sed1356_device::panel_type_w(offs_t offset, uint8_t data)
{
	static const char *const s_data_names[2][4] =
	{
		{ "4-bit", "8-bit", "16-bit", "Reserved" },
		{ "9-bit", "12-bit", "18-bit", "Reserved" }
	};
	LOGMASKED(LOG_MISC_WR, "%s: panel_type_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                Panel Type: %s\n", machine().describe_context(), BIT(data, PTYPE_TFT_PASS_BIT) ? "TFT" : "Passive");
	LOGMASKED(LOG_MISC_WR, "%s:                Panel Count: %d\n", machine().describe_context(), 1 + BIT(data, PTYPE_COUNT_BIT));
	LOGMASKED(LOG_MISC_WR, "%s:                Color Panel: %s\n", machine().describe_context(), BIT(data, PTYPE_COLOR_BIT) ? "Yes" : "No");
	LOGMASKED(LOG_MISC_WR, "%s:                Format Select: %d\n", machine().describe_context(), BIT(data, PTYPE_FORMAT_BIT));
	LOGMASKED(LOG_MISC_WR, "%s:                Data Width: %s\n", machine().describe_context(), s_data_names[BIT(data, PTYPE_TFT_PASS_BIT)][(data & PTYPE_WIDTH_MASK) >> PTYPE_WIDTH_BIT]);
	LOGMASKED(LOG_MISC_WR, "%s:                EL Mode Enable: %d\n", machine().describe_context(), BIT(data, PTYPE_EL_BIT));
	m_panel_type = data & PTYPE_MASK;
}

void sed1356_device::mod_rate_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: mod_rate_w = %02x\n", machine().describe_context(), data);
	m_mod_rate = data & MODRATE_MASK;
}


uint8_t sed1356_device::lcd_display_width_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_width_r: %02x\n", machine().describe_context(), m_lcd_width);
	return m_lcd_width;
}

uint8_t sed1356_device::lcd_hblank_period_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_hblank_period_r: %02x\n", machine().describe_context(), m_lcd_hblank_period);
	return m_lcd_hblank_period;
}

uint8_t sed1356_device::tft_fpline_start_pos_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: tft_fpline_start_pos_r: %02x\n", machine().describe_context(), m_tft_fpline_start);
	return m_tft_fpline_start;
}

uint8_t sed1356_device::tft_fpline_pulse_width_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: tft_fpline_pulse_width_r: %02x\n", machine().describe_context(), m_tft_fpline_width);
	return m_tft_fpline_width;
}

uint8_t sed1356_device::lcd_display_height_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_lcd_height >> (8 * offset));
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_height_r[%d]: %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t sed1356_device::lcd_vblank_period_r(offs_t offset)
{
	const uint8_t vblank = screen().vblank() ? (1 << LCDVBL_STATUS_BIT) : 0x00;
	const uint8_t data = m_lcd_vblank_period | vblank;
	LOGMASKED(LOG_LCD_RD, "%s: lcd_vblank_period_r: %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t sed1356_device::tft_fpframe_start_pos_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: tft_fpframe_start_pos_r: %02x\n", machine().describe_context(), m_tft_fpframe_start);
	return m_tft_fpframe_start;
}

uint8_t sed1356_device::tft_fpframe_pulse_width_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: tft_fpframe_pulse_width_r: %02x\n", machine().describe_context(), m_tft_fpframe_width);
	return m_tft_fpframe_width;
}

uint8_t sed1356_device::lcd_display_mode_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_mode_r: %02x\n", machine().describe_context(), m_lcd_mode);
	return m_lcd_mode;
}

uint8_t sed1356_device::lcd_misc_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_misc_r: %02x\n", machine().describe_context(), m_lcd_misc);
	return m_lcd_misc;
}

uint8_t sed1356_device::lcd_display_start_addr_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_lcd_addr >> (8 * offset));
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_start_addr_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::lcd_mem_addr_offset_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_lcd_mem_offset >> (8 * offset));
	LOGMASKED(LOG_LCD_RD, "%s: lcd_mem_addr_offset_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::lcd_pixel_pan_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_pixel_pan_r: %02x\n", machine().describe_context(), m_lcd_pan);
	return m_lcd_pan;
}

uint8_t sed1356_device::lcd_display_fifo_hi_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_fifo_hi_thresh_r: %02x\n", machine().describe_context(), m_lcd_fifo_hi_thresh);
	return m_lcd_fifo_hi_thresh;
}

uint8_t sed1356_device::lcd_display_fifo_lo_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_display_fifo_lo_thresh_r: %02x\n", machine().describe_context(), m_lcd_fifo_lo_thresh);
	return m_lcd_fifo_lo_thresh;
}

void sed1356_device::lcd_display_width_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_width_w = %02x\n", machine().describe_context(), data);
	m_lcd_width = data & LCDW_MASK;
}

void sed1356_device::lcd_hblank_period_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_hblank_period_w = %02x\n", machine().describe_context(), data);
	m_lcd_hblank_period = data & LCDHBL_MASK;
}

void sed1356_device::tft_fpline_start_pos_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: tft_fpline_start_pos_w = %02x\n", machine().describe_context(), data);
	m_tft_fpline_start = data & TFTFPLS_MASK;
}

void sed1356_device::tft_fpline_pulse_width_w(offs_t offset, uint8_t data)
{
	const int tft = BIT(data, PTYPE_TFT_PASS_BIT);
	static const char *const s_polarity_names[2][2] =
	{
		{ "active-high", "active-low" },
		{ "active-low", "active-high" }
	};
	LOGMASKED(LOG_LCD_WR, "%s: tft_fpline_pulse_width_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD_WR, "%s:                            Pulse Width: %d\n", machine().describe_context(), (data & TFTFPLW_WIDTH_MASK) >> TFTFPLW_WIDTH_BIT);
	LOGMASKED(LOG_LCD_WR, "%s:                            Polarity: %s\n", machine().describe_context(), s_polarity_names[tft][BIT(data, TFTFPLW_POL_BIT)]);
	m_tft_fpline_width = data & TFTFPLW_MASK;
}

void sed1356_device::lcd_display_height_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_height_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_lcd_height &= ~(0x00ff << shift);
	m_lcd_height = (m_lcd_height | (data << shift)) & LCDH_MASK;
}

void sed1356_device::lcd_vblank_period_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_vblank_period_w = %02x\n", machine().describe_context(), data);
	m_lcd_vblank_period &= ~LCDVBL_VALUE_MASK;
	m_lcd_vblank_period |= (data & LCDVBL_VALUE_MASK) << LCDVBL_VALUE_BIT;
}

void sed1356_device::tft_fpframe_start_pos_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: tft_fpframe_start_pos_w = %02x\n", machine().describe_context(), data);
	m_tft_fpframe_start = data & TFTFPLS_MASK;
}

void sed1356_device::tft_fpframe_pulse_width_w(offs_t offset, uint8_t data)
{
	const int tft = BIT(data, PTYPE_TFT_PASS_BIT);
	static const char *const s_polarity_names[2][2] =
	{
		{ "active-high", "active-low" },
		{ "active-low", "active-high" }
	};
	LOGMASKED(LOG_LCD_WR, "%s: tft_fpframe_pulse_width_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD_WR, "%s:                             Pulse Width: %d\n", machine().describe_context(), (data & TFTFPFW_WIDTH_MASK) >> TFTFPFW_WIDTH_BIT);
	LOGMASKED(LOG_LCD_WR, "%s:                             Polarity: %s\n", machine().describe_context(), s_polarity_names[tft][BIT(data, TFTFPFW_POL_BIT)]);
	m_tft_fpframe_width = data & TFTFPFW_MASK;
}

void sed1356_device::lcd_display_mode_w(offs_t offset, uint8_t data)
{
	static const char *const s_bpp_names[8] = { "Reserved", "Reserved", "4bpp", "8bpp", "15bpp", "16bpp", "Reserved", "Reserved" };
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_mode_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD_WR, "%s:                      Bit Depth: %s\n", machine().describe_context(), s_bpp_names[(data & LCDMODE_BPP_MASK) >> LCDMODE_BPP_BIT]);
	LOGMASKED(LOG_LCD_WR, "%s:                      SwivelView Enable Bit 1: %d\n", machine().describe_context(), BIT(data, LCDMODE_SWIVEN1_BIT));
	LOGMASKED(LOG_LCD_WR, "%s:                      LCD Display Blank: %d\n", machine().describe_context(), BIT(data, LCDMODE_BLANK_BIT));
	m_lcd_mode = data & LCDMODE_MASK;
}

void sed1356_device::lcd_misc_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_misc_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD_WR, "%s:              Dual-Panel Buffer Disable: %d\n", machine().describe_context(), BIT(data, LCDMISC_DPDIS_BIT));
	LOGMASKED(LOG_LCD_WR, "%s:              Dithering Disable: %d\n", machine().describe_context(), BIT(data, LCDMISC_DITHDIS_BIT));
	m_lcd_misc = data & LCDMISC_MASK;
}

void sed1356_device::lcd_display_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_start_addr_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint32_t shift = 8 * offset;
	m_lcd_addr &= ~(0x00ff << shift);
	m_lcd_addr = (m_lcd_addr | (data << shift)) & LCDDS_MASK;
}

void sed1356_device::lcd_mem_addr_offset_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_mem_addr_offset_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_lcd_mem_offset &= ~(0x00ff << shift);
	m_lcd_mem_offset = (m_lcd_mem_offset | (data << shift)) & LCDMOFS_MASK;
}

void sed1356_device::lcd_pixel_pan_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_pixel_pan_w = %02x\n", machine().describe_context(), data);
	m_lcd_pan = data & LCDPAN_MASK;
}

void sed1356_device::lcd_display_fifo_hi_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_fifo_hi_thresh_w = %02x\n", machine().describe_context(), data);
	m_lcd_fifo_hi_thresh = data & LCDFIFO_MASK;
}

void sed1356_device::lcd_display_fifo_lo_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_display_fifo_lo_thresh_w = %02x\n", machine().describe_context(), data);
	m_lcd_fifo_lo_thresh = data & LCDFIFO_MASK;
}


uint8_t sed1356_device::crt_display_width_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_width_r: %02x\n", machine().describe_context(), m_crt_width);
	return m_crt_width;
}

uint8_t sed1356_device::crt_hblank_period_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_hblank_period_r: %02x\n", machine().describe_context(), m_crt_hblank_period);
	return m_crt_hblank_period;
}

uint8_t sed1356_device::crt_hrtc_start_pos_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_hrtc_start_pos_r: %02x\n", machine().describe_context(), m_crt_hrtc_start);
	return m_crt_hrtc_start;
}

uint8_t sed1356_device::crt_hrtc_pulse_width_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_hrtc_pulse_width_r: %02x\n", machine().describe_context(), m_crt_hrtc_width);
	return m_crt_hrtc_width;
}

uint8_t sed1356_device::crt_display_height_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_crt_height >> (8 * offset));
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_height_r[%d]: %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t sed1356_device::crt_vblank_period_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_vblank_period_r: %02x\n", machine().describe_context(), m_crt_vblank_period);
	LOGMASKED(LOG_CRT_RD, "%s:                      VBlank Active: %d\n", machine().describe_context(), BIT(m_crt_vblank_period, CRTVBL_STATUS_BIT));
	return m_crt_vblank_period;
}

uint8_t sed1356_device::crt_vrtc_start_pos_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_vrtc_start_pos_r: %02x\n", machine().describe_context(), m_crt_vrtc_start);
	return m_crt_vrtc_start;
}

uint8_t sed1356_device::crt_vrtc_pulse_width_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_vrtc_pulse_width_r: %02x\n", machine().describe_context(), m_crt_vrtc_width);
	return m_crt_vrtc_width;
}

uint8_t sed1356_device::crt_output_ctrl_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_output_ctrl_r: %02x\n", machine().describe_context(), m_crt_output_ctrl);
	return m_crt_output_ctrl;
}

uint8_t sed1356_device::crt_display_mode_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_mode_r: %02x\n", machine().describe_context(), m_crt_mode);
	return m_crt_mode;
}

uint8_t sed1356_device::crt_display_start_addr_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_crt_addr >> (8 * offset));
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_start_addr_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::crt_mem_addr_offset_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_crt_mem_offset >> (8 * offset));
	LOGMASKED(LOG_CRT_RD, "%s: crt_mem_addr_offset_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::crt_pixel_pan_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: crt_pixel_pan_r: %02x\n", machine().describe_context(), m_crt_pan);
	return m_crt_pan;
}

uint8_t sed1356_device::crt_display_fifo_hi_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_fifo_hi_thresh_r: %02x\n", machine().describe_context(), m_crt_fifo_hi_thresh);
	return m_crt_fifo_hi_thresh;
}

uint8_t sed1356_device::crt_display_fifo_lo_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_display_fifo_lo_thresh_r: %02x\n", machine().describe_context(), m_crt_fifo_lo_thresh);
	return m_crt_fifo_lo_thresh;
}

void sed1356_device::crt_display_width_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_width_w = %02x\n", machine().describe_context(), data);
	m_crt_width = data & CRTW_MASK;
}

void sed1356_device::crt_hblank_period_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_hblank_period_w = %02x\n", machine().describe_context(), data);
	m_crt_hblank_period = data & CRTHBL_MASK;
}

void sed1356_device::crt_hrtc_start_pos_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_hrtc_start_pos_w = %02x\n", machine().describe_context(), data);
	m_crt_hrtc_start = data & CRTHRTS_MASK;
}

void sed1356_device::crt_hrtc_pulse_width_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_hrtc_pulse_width_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CRT_WR, "%s:                          Pulse Width: %d\n", machine().describe_context(), (data & CRTHRTW_WIDTH_MASK) >> CRTHRTW_WIDTH_BIT);
	LOGMASKED(LOG_CRT_WR, "%s:                          Polarity: %s\n", machine().describe_context(), BIT(data, CRTHRTW_POL_BIT) ? "active-high" : "active-low");
	m_crt_hrtc_width = data & CRTHRTW_MASK;
}

void sed1356_device::crt_display_height_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_height_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_crt_height &= ~(0x00ff << shift);
	m_crt_height = (m_crt_height | (data << shift)) & CRTH_MASK;
}

void sed1356_device::crt_vblank_period_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_vblank_period_w = %02x\n", machine().describe_context(), data);
	m_crt_vblank_period &= ~CRTVBL_VALUE_MASK;
	m_crt_vblank_period |= (data & CRTVBL_VALUE_MASK) << CRTVBL_VALUE_BIT;
}

void sed1356_device::crt_vrtc_start_pos_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_vrtc_start_pos_w = %02x\n", machine().describe_context(), data);
	m_crt_vrtc_start = data & CRTVRTS_MASK;
}

void sed1356_device::crt_vrtc_pulse_width_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_vrtc_pulse_width_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CRT_WR, "%s:                          Pulse Width: %d\n", machine().describe_context(), (data & CRTVRTW_WIDTH_MASK) >> CRTVRTW_WIDTH_BIT);
	LOGMASKED(LOG_CRT_WR, "%s:                          Polarity: %s\n", machine().describe_context(), BIT(data, CRTVRTW_POL_BIT) ? "active-high" : "active-low");
	m_crt_vrtc_width = data & CRTVRTW_MASK;
}

void sed1356_device::crt_output_ctrl_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_output_ctrl_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CRT_WR, "%s:                     Output Format: %s\n", machine().describe_context(), BIT(data, CRTCTRL_PAL_BIT) ? "PAL" : "NTSC");
	LOGMASKED(LOG_CRT_WR, "%s:                     Output Signal: %s\n", machine().describe_context(), BIT(data, CRTCTRL_SVID_BIT) ? "S-Video" : "Composite");
	LOGMASKED(LOG_CRT_WR, "%s:                     DAC Output Level Select: %d\n", machine().describe_context(), BIT(data, CRTCTRL_DACLVL_BIT));
	LOGMASKED(LOG_CRT_WR, "%s:                     Luma Filter Enable: %d\n", machine().describe_context(), BIT(data, CRTCTRL_LUMF_BIT));
	LOGMASKED(LOG_CRT_WR, "%s:                     Chroma Filter Enable: %d\n", machine().describe_context(), BIT(data, CRTCTRL_CHRF_BIT));
	m_crt_output_ctrl = data & CRTCTRL_MASK;
}

void sed1356_device::crt_display_mode_w(offs_t offset, uint8_t data)
{
	static const char *const s_bpp_names[8] = { "Reserved", "Reserved", "4bpp", "8bpp", "15bpp", "16bpp", "Reserved", "Reserved" };
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_mode_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CRT_WR, "%s:                      Bit Depth: %s\n", machine().describe_context(), s_bpp_names[(data & CRTMODE_BPP_MASK) >> CRTMODE_BPP_BIT]);
	LOGMASKED(LOG_CRT_WR, "%s:                      CRT Display Blank: %d\n", machine().describe_context(), BIT(data, CRTMODE_BLANK_BIT));
	m_crt_mode = data & CRTMODE_MASK;
}

void sed1356_device::crt_display_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_start_addr_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint32_t shift = 8 * offset;
	m_crt_addr &= ~(0x00ff << shift);
	m_crt_addr = (m_crt_addr | (data << shift)) & CRTDS_MASK;
}

void sed1356_device::crt_mem_addr_offset_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_mem_addr_offset_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_crt_mem_offset &= ~(0x00ff << shift);
	m_crt_mem_offset = (m_crt_mem_offset | (data << shift)) & CRTMOFS_MASK;
}

void sed1356_device::crt_pixel_pan_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_pixel_pan_w = %02x\n", machine().describe_context(), data);
	m_crt_pan = data & CRTPAN_MASK;
}

void sed1356_device::crt_display_fifo_hi_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_fifo_hi_thresh_w = %02x\n", machine().describe_context(), data);
	m_crt_fifo_hi_thresh = data & CRTFIFO_MASK;
}

void sed1356_device::crt_display_fifo_lo_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_display_fifo_lo_thresh_w = %02x\n", machine().describe_context(), data);
	m_crt_fifo_lo_thresh = data & CRTFIFO_MASK;
}


uint8_t sed1356_device::lcd_cursor_ctrl_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_ctrl_r: %02x\n", machine().describe_context(), m_lcd_cursor_ctrl);
	return m_lcd_cursor_ctrl;
}

uint8_t sed1356_device::lcd_cursor_start_addr_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_start_addr_r: %02x\n", machine().describe_context(), m_lcd_cursor_addr);
	return m_lcd_cursor_addr;
}

uint8_t sed1356_device::lcd_cursor_x_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_lcd_cursor_x >> (8 * offset));
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_x_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::lcd_cursor_y_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_lcd_cursor_y >> (8 * offset));
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_y_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::lcd_cursor_color0_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_color0_r[%d]: %02x\n", machine().describe_context(), offset, m_lcd_cursor_color0[offset]);
	return m_lcd_cursor_color0[offset];
}

uint8_t sed1356_device::lcd_cursor_color1_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_color1_r[%d]: %02x\n", machine().describe_context(), offset, m_lcd_cursor_color1[offset]);
	return m_lcd_cursor_color1[offset];
}

uint8_t sed1356_device::lcd_cursor_fifo_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_LCD_RD, "%s: lcd_cursor_fifo_thresh_r: %02x\n", machine().describe_context(), offset, m_lcd_cursor_fifo_thresh);
	return m_lcd_cursor_fifo_thresh;
}

void sed1356_device::lcd_cursor_ctrl_w(offs_t offset, uint8_t data)
{
	static const char *const s_mode_names[4] = { "Inactive", "Cursor", "Ink", "Reserved" };
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_ctrl_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_LCD_WR, "%s:                     Cursor Mode: %s\n", machine().describe_context(), s_mode_names[(data & CURCTRL_MODE_MASK) >> CURCTRL_MODE_BIT]);
	m_lcd_cursor_ctrl = data & CURCTRL_MASK;
}

void sed1356_device::lcd_cursor_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_start_addr_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_addr = data;
}

void sed1356_device::lcd_cursor_x_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_x_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_lcd_cursor_x &= ~(0x00ff << shift);
	m_lcd_cursor_x = (m_lcd_cursor_x | (data << shift)) & CURX_MASK;
}

void sed1356_device::lcd_cursor_y_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_y_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_lcd_cursor_y &= ~(0x00ff << shift);
	m_lcd_cursor_y = (m_lcd_cursor_y | (data << shift)) & CURY_MASK;
}

void sed1356_device::lcd_cursor_color0_blu_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color0_blu_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color0[0] = data & CURB_MASK;
}

void sed1356_device::lcd_cursor_color0_grn_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color0_grn_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color0[1] = data & CURG_MASK;
}

void sed1356_device::lcd_cursor_color0_red_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color0_red_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color0[2] = data & CURR_MASK;
}

void sed1356_device::lcd_cursor_color1_blu_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color1_blu_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color1[0] = data & CURB_MASK;
}

void sed1356_device::lcd_cursor_color1_grn_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color1_grn_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color1[1] = data & CURG_MASK;
}

void sed1356_device::lcd_cursor_color1_red_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_color1_red_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_color1[2] = data & CURR_MASK;
}

void sed1356_device::lcd_cursor_fifo_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: lcd_cursor_fifo_thresh_w = %02x\n", machine().describe_context(), data);
	m_lcd_cursor_fifo_thresh = data & CUR_FIFO_MASK;
}


uint8_t sed1356_device::crt_cursor_ctrl_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_ctrl_r: %02x\n", machine().describe_context(), m_crt_cursor_ctrl);
	return m_crt_cursor_ctrl;
}

uint8_t sed1356_device::crt_cursor_start_addr_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_start_addr_r: %02x\n", machine().describe_context(), m_crt_cursor_addr);
	return m_crt_cursor_addr;
}

uint8_t sed1356_device::crt_cursor_x_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_crt_cursor_x >> (8 * offset));
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_x_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::crt_cursor_y_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_crt_cursor_y >> (8 * offset));
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_y_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::crt_cursor_color0_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_color0_r[%d]: %02x\n", machine().describe_context(), offset, m_crt_cursor_color0[offset]);
	return m_crt_cursor_color0[offset];
}

uint8_t sed1356_device::crt_cursor_color1_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_color1_r[%d]: %02x\n", machine().describe_context(), offset, m_crt_cursor_color1[offset]);
	return m_crt_cursor_color1[offset];
}

uint8_t sed1356_device::crt_cursor_fifo_thresh_r(offs_t offset)
{
	LOGMASKED(LOG_CRT_RD, "%s: crt_cursor_fifo_thresh_r: %02x\n", machine().describe_context(), offset, m_crt_cursor_fifo_thresh);
	return m_crt_cursor_fifo_thresh;
}

void sed1356_device::crt_cursor_ctrl_w(offs_t offset, uint8_t data)
{
	static const char *const s_mode_names[4] = { "Inactive", "Cursor", "Ink", "Reserved" };
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_ctrl_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_CRT_WR, "%s:                     Cursor Mode: %s\n", machine().describe_context(), s_mode_names[(data & CURCTRL_MODE_MASK) >> CURCTRL_MODE_BIT]);
	m_crt_cursor_ctrl = data & CURCTRL_MASK;
}

void sed1356_device::crt_cursor_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_start_addr_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_addr = data;
}

void sed1356_device::crt_cursor_x_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_x_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_crt_cursor_x &= ~(0x00ff << shift);
	m_crt_cursor_x = (m_crt_cursor_x | (data << shift)) & CURX_MASK;
}

void sed1356_device::crt_cursor_y_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_y_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_crt_cursor_y &= ~(0x00ff << shift);
	m_crt_cursor_y = (m_crt_cursor_y | (data << shift)) & CURY_MASK;
}

void sed1356_device::crt_cursor_color0_blu_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_color0_blu_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color0[0] = data & CURB_MASK;
}

void sed1356_device::crt_cursor_color0_grn_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_color0_grn_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color0[1] = data & CURG_MASK;
}

void sed1356_device::crt_cursor_color0_red_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LCD_WR, "%s: crt_cursor_color0_red_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color0[2] = data & CURR_MASK;
}

void sed1356_device::crt_cursor_color1_blu_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_color1_blu_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color1[0] = data & CURB_MASK;
}

void sed1356_device::crt_cursor_color1_grn_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_color1_grn_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color1[1] = data & CURG_MASK;
}

void sed1356_device::crt_cursor_color1_red_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_color1_red_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_color1[2] = data & CURR_MASK;
}

void sed1356_device::crt_cursor_fifo_thresh_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CRT_WR, "%s: crt_cursor_fifo_thresh_w = %02x\n", machine().describe_context(), data);
	m_crt_cursor_fifo_thresh = data & CUR_FIFO_MASK;
}


template <bool Linear>
void sed1356_device::bitblt_solid_fill()
{
	uint16_t *dst = (uint16_t*)&m_vram[m_bitblt_dst_addr >> 2];
	if (m_bitblt_dst_addr & 2)
		dst++;

	for (uint32_t y = 0; y <= m_bitblt_height; y++)
	{
		for (uint32_t x = 0; x <= m_bitblt_width; x++)
		{
			if (Linear)
				*dst++ = m_bitblt_fgcolor;
			else
				dst[x] = m_bitblt_fgcolor;
		}

		if (!Linear)
			dst += m_bitblt_mem_offset;
	}
}

uint16_t sed1356_device::bitblt_rop(const uint8_t rop, const uint16_t s, const uint16_t d)
{
	switch (rop & 0x0f)
	{
	case 0:
		return 0;
	case 1:
		return ~(s | d);
	case 2:
		return ~s & d;
	case 3:
		return ~s;
	case 4:
		return s | ~d;
	case 5:
		return ~d;
	case 6:
		return s ^d;
	case 7:
		return ~s | ~d;
	case 8:
		return s & d;
	case 9:
		return ~(s ^ d);
	case 10:
		return d;
	case 11:
		return ~s | d;
	case 12:
		return s;
	case 13:
		return s | ~d;
	case 14:
		return s | d;
	case 15:
		return 0xffff;
	}
	return 0;
}

void sed1356_device::bitblt_async_advance(uint32_t &addr)
{
	m_bitblt_x++;
	if (m_bitblt_x > m_bitblt_width)
	{
		m_bitblt_x = 0;
		m_bitblt_y++;
		if (!BIT(m_bitblt_ctrl[0], BBCTRL0_DSTLIN_BIT))
		{
			addr -= (m_bitblt_width << 1);
			addr += (m_bitblt_mem_offset << 1);
		}
		if (m_bitblt_y > m_bitblt_height)
		{
			m_bitblt_ctrl[0] &= ~(1 << BBCTRL0_ACTIVE_BIT);
			m_bitblt_ctrl[0] &= ~(1 << BBCTRL0_ANY_BIT);
		}
	}
	else
	{
		addr += 2;
	}
}

void sed1356_device::bitblt_write_continue(const uint16_t s)
{
	uint16_t *dstp = (uint16_t*)&m_vram[m_bitblt_curr_dst_addr >> 2];
	if (m_bitblt_curr_dst_addr & 2)
		dstp++;

	*dstp = bitblt_rop(m_bitblt_rop, s, *dstp);

	bitblt_async_advance(m_bitblt_curr_dst_addr);
}

void sed1356_device::bitblt_write()
{
	m_bitblt_ctrl[0] |= (1 << BBCTRL0_ACTIVE_BIT);
	m_bitblt_x = 0;
	m_bitblt_y = 0;
	m_bitblt_curr_dst_addr = m_bitblt_dst_addr;
}

uint16_t sed1356_device::bitblt_read_continue()
{
	const uint16_t *srcp = (uint16_t*)&m_vram[m_bitblt_curr_src_addr >> 2];
	if (m_bitblt_curr_src_addr & 2)
		srcp++;

	const uint16_t data = *srcp;

	bitblt_async_advance(m_bitblt_curr_src_addr);

	return data;
}

void sed1356_device::bitblt_read()
{
	m_bitblt_ctrl[0] |= (1 << BBCTRL0_ACTIVE_BIT);
	m_bitblt_ctrl[0] |= (1 << BBCTRL0_ANY_BIT);
	m_bitblt_x = 0;
	m_bitblt_y = 0;
	m_bitblt_curr_src_addr = m_bitblt_src_addr;
}

template <bool SrcLinear, bool DstLinear>
void sed1356_device::bitblt_move_negative()
{
	uint16_t *srcp = (uint16_t*)&m_vram[m_bitblt_src_addr >> 2];
	if (m_bitblt_src_addr & 2)
		srcp++;

	uint16_t *dstp = (uint16_t*)&m_vram[m_bitblt_dst_addr >> 2];
	if (m_bitblt_dst_addr & 2)
		dstp++;

	for (int32_t y = (int32_t)m_bitblt_height; y >= 0; y--)
	{
		for (int32_t x = (int32_t)m_bitblt_width; x >= 0; x--)
		{
			const uint16_t s = (SrcLinear ? *srcp : srcp[x]);
			const uint16_t d = (DstLinear ? *dstp : dstp[x]);

			*dstp = bitblt_rop(m_bitblt_rop, s, d);

			if (SrcLinear)
				srcp++;
			if (DstLinear)
				dstp++;
		}

		if (!SrcLinear)
			srcp += m_bitblt_mem_offset;
		if (!DstLinear)
			dstp += m_bitblt_mem_offset;
	}
}

void sed1356_device::bitblt_execute_command()
{
	switch (m_bitblt_op)
	{
	case 0:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Write BitBLT with ROP\n");
		bitblt_write();
		return;
	case 1:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Read BitBLT\n");
		bitblt_read();
		return;
	case 2:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Move BitBLT in + direction with ROP\n");
		return;
	case 3:
	{
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Move BitBLT in - direction with ROP\n");
		const bool srclin = BIT(m_bitblt_ctrl[0], BBCTRL0_SRCLIN_BIT);
		const bool dstlin = BIT(m_bitblt_ctrl[0], BBCTRL0_DSTLIN_BIT);
		if (srclin && dstlin)
			bitblt_move_negative<true, true>();
		else if (srclin)
			bitblt_move_negative<true, false>();
		else if (dstlin)
			bitblt_move_negative<false, true>();
		else
			bitblt_move_negative<false, false>();
		return;
	}
	case 4:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Transparent Write BitBLT\n");
		return;
	case 5:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Transparent Move BitBLT in + direction\n");
		return;
	case 6:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Pattern Fill with ROP\n");
		return;
	case 7:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Pattern Fill with transparency\n");
		return;
	case 8:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Color Expansion\n");
		return;
	case 9:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Color Expansion with transparency\n");
		return;
	case 10:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Move BitBLT with Color Expansion\n");
		return;
	case 11:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Command not yet implemented: Move BitBLT with Color Expansion and transparency\n");
		return;
	case 12:
	{
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Solid Fill\n");
		if (BIT(m_bitblt_ctrl[0], BBCTRL0_DSTLIN_BIT))
			bitblt_solid_fill<true>();
		else
			bitblt_solid_fill<false>();
		return;
	}
	default:
		LOGMASKED(LOG_BITBLT_OP, "bitblt: Unsupported command type: %02x\n", m_bitblt_op);
		return;
	}
}

uint8_t sed1356_device::bitblt_ctrl0_r(offs_t offset)
{
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_ctrl0_r: %02x\n", machine().describe_context(), m_bitblt_ctrl[0]);
	LOGMASKED(LOG_BITBLT_RD, "%s:                 FIFO Full: %d\n", machine().describe_context(), BIT(m_bitblt_ctrl[0], BBCTRL0_FULL_BIT));
	LOGMASKED(LOG_BITBLT_RD, "%s:                 FIFO Half-Full: %d\n", machine().describe_context(), BIT(m_bitblt_ctrl[0], BBCTRL0_HALF_BIT));
	LOGMASKED(LOG_BITBLT_RD, "%s:                 FIFO Not Empty: %d\n", machine().describe_context(), BIT(m_bitblt_ctrl[0], BBCTRL0_ANY_BIT));
	LOGMASKED(LOG_BITBLT_RD, "%s:                 Active: %d\n", machine().describe_context(), BIT(m_bitblt_ctrl[0], BBCTRL0_ACTIVE_BIT));
	return m_bitblt_ctrl[0];
}

uint8_t sed1356_device::bitblt_ctrl1_r(offs_t offset)
{
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_ctrl1_r: %02x\n", machine().describe_context(), m_bitblt_ctrl[1]);
	return m_bitblt_ctrl[1];
}

uint8_t sed1356_device::bitblt_rop_code_r(offs_t offset)
{
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_rop_code_r: %02x\n", machine().describe_context(), m_bitblt_rop);
	return m_bitblt_rop;
}

uint8_t sed1356_device::bitblt_operation_r(offs_t offset)
{
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_operation_r: %02x\n", machine().describe_context(), m_bitblt_op);
	return m_bitblt_op;
}

uint8_t sed1356_device::bitblt_src_start_addr_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_src_addr >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_src_start_addr_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_dst_start_addr_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_dst_addr >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_dst_start_addr_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_mem_addr_offset_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_mem_offset >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_mem_addr_offset_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_width_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_width >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_width_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_height_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_height >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_height_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_bgcolor_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_bgcolor >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_bgcolor_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::bitblt_fgcolor_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_bitblt_fgcolor >> (8 * offset));
	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_fgcolor_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

void sed1356_device::bitblt_ctrl0_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_ctrl0_w = %02x\n", machine().describe_context(), data);
	m_bitblt_ctrl[0] &= ~BBCTRL0_WR_MASK;
	m_bitblt_ctrl[0] |= data & BBCTRL0_WR_MASK;
	if (BIT(data, BBCTRL0_ACTIVE_BIT))
	{
		bitblt_execute_command();
	}
}

void sed1356_device::bitblt_ctrl1_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_ctrl1_w = %02x\n", machine().describe_context(), data);
	m_bitblt_ctrl[1] &= ~BBCTRL1_WR_MASK;
	m_bitblt_ctrl[1] |= data & BBCTRL1_WR_MASK;
}

void sed1356_device::bitblt_rop_code_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_rop_code_w = %02x\n", machine().describe_context(), data);
	m_bitblt_rop = data & BBCODE_MASK;
}

void sed1356_device::bitblt_operation_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_operation_w = %02x\n", machine().describe_context(), data);
	m_bitblt_op = data & BBOP_MASK;
}

void sed1356_device::bitblt_src_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_src_start_addr_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_src_addr &= ~(0x00ff << shift);
	m_bitblt_src_addr = (m_bitblt_src_addr | (data << shift)) & BBSRC_MASK;
}

void sed1356_device::bitblt_dst_start_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_dst_start_addr_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_dst_addr &= ~(0x00ff << shift);
	m_bitblt_dst_addr = (m_bitblt_dst_addr | (data << shift)) & BBDST_MASK;
}

void sed1356_device::bitblt_mem_addr_offset_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_mem_addr_offset_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_mem_offset &= ~(0x00ff << shift);
	m_bitblt_mem_offset = (m_bitblt_mem_offset | (data << shift)) & BBMADR_MASK;
}

void sed1356_device::bitblt_width_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_width_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_width &= ~(0x00ff << shift);
	m_bitblt_width = (m_bitblt_width | (data << shift)) & BBWIDTH_MASK;
}

void sed1356_device::bitblt_height_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_height_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_height &= ~(0x00ff << shift);
	m_bitblt_height = (m_bitblt_height | (data << shift)) & BBHEIGHT_MASK;
}

void sed1356_device::bitblt_bgcolor_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_bgcolor_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_bgcolor &= ~(0x00ff << shift);
	m_bitblt_bgcolor |= (data << shift);
}

void sed1356_device::bitblt_fgcolor_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_fgcolor_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_bitblt_fgcolor &= ~(0x00ff << shift);
	m_bitblt_fgcolor |= (data << shift);
}


uint8_t sed1356_device::lut_mode_r(offs_t offset)
{
	LOGMASKED(LOG_LUT_RD, "%s: lut_mode_r: %02x\n", machine().describe_context(), m_lut_mode);
	return m_lut_mode;
}

uint8_t sed1356_device::lut_addr_r(offs_t offset)
{
	LOGMASKED(LOG_LUT_RD, "%s: lut_addr_r: %02x\n", machine().describe_context(), m_lut_addr);
	return m_lut_addr;
}

uint8_t sed1356_device::lut_data_r(offs_t offset)
{
	LOGMASKED(LOG_LUT_RD, "%s: lut_data_r: %02x\n", machine().describe_context(), m_lut_data);
	return m_lut_data;
}

void sed1356_device::lut_mode_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LUT_WR, "%s: lut_mode_w = %02x\n", machine().describe_context(), data);
	m_lut_mode = data & LUTMODE_MASK;
}

void sed1356_device::lut_addr_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LUT_WR, "%s: lut_addr_w = %02x\n", machine().describe_context(), data);
	m_lut_addr = data;
}

void sed1356_device::lut_data_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_LUT_WR, "%s: lut_data_w = %02x\n", machine().describe_context(), data);
	m_lut_data = data & LUTDATA_MASK;
}


uint8_t sed1356_device::power_config_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: power_config_r: %02x\n", machine().describe_context(), m_power_config);
	return m_power_config;
}

uint8_t sed1356_device::power_status_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: power_status_r: %02x\n", machine().describe_context(), m_power_status);
	return m_power_status;
}

void sed1356_device::power_config_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: power_config_w = %02x\n", machine().describe_context(), data);
	m_power_config = data & PWRCFG_MASK;
}


uint8_t sed1356_device::cpu_mem_watchdog_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: cpu_mem_watchdog_r: %02x\n", machine().describe_context(), m_watchdog);
	return m_watchdog;
}

void sed1356_device::cpu_mem_watchdog_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MISC_WR, "%s: cpu_mem_watchdog_w = %02x\n", machine().describe_context(), data);
	m_watchdog = data & WATCHDOG_MASK;
}


uint8_t sed1356_device::display_mode_r(offs_t offset)
{
	LOGMASKED(LOG_MISC_RD, "%s: display_mode_r: %02x\n", machine().describe_context(), m_display_mode);
	return m_display_mode;
}

void sed1356_device::display_mode_w(offs_t offset, uint8_t data)
{
	static const char *const s_mode_names[8] =
	{
		"No Display",
		"LCD only",
		"CRT only",
		"Double, CRT and LCD",
		"TV w/ no flicker filter",
		"Double, TV w/ no flicker filter and LCD",
		"TV w/ flicker filter",
		"Double, TV w/ flicker filter and LCD"
	};
	LOGMASKED(LOG_MISC_WR, "%s: display_mode_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_MISC_WR, "%s:                  Display Mode: %s\n", machine().describe_context(), s_mode_names[(data & DISPMODE_MODE_MASK) >> DISPMODE_MODE_BIT]);
	LOGMASKED(LOG_MISC_WR, "%s:                  SwivelView Enable Bit 0: %d\n", machine().describe_context(), BIT(data, DISPMODE_SWIVEN0_BIT));
	m_display_mode = data & DISPMODE_MASK;
}


uint8_t sed1356_device::mplug_lcmd_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_mplug_lcmd >> (8 * offset));
	LOGMASKED(LOG_MPLUG_RD, "%s: mplug_lcmd_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::mplug_resv_lcmd_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_mplug_resv_lcmd >> (8 * offset));
	LOGMASKED(LOG_MPLUG_RD, "%s: mplug_resv_lcmd_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::mplug_cmd_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_mplug_cmd >> (8 * offset));
	LOGMASKED(LOG_MPLUG_RD, "%s: mplug_cmd_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::mplug_resv_cmd_r(offs_t offset)
{
	const uint8_t data = (uint8_t)(m_mplug_resv_cmd >> (8 * offset));
	LOGMASKED(LOG_MPLUG_RD, "%s: mplug_resv_cmd_r[%d]: %02x\n", machine().describe_context(), offset, data);
	return data;
}

uint8_t sed1356_device::mplug_data_r(offs_t offset)
{
	LOGMASKED(LOG_MPLUG_RD, "%s: (not implemented) mplug_data_r[%04x]: %02x\n", machine().describe_context(), 0x1008 + offset, 0);
	return 0;
}

void sed1356_device::mplug_lcmd_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MPLUG_WR, "%s: mplug_lcmd_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_mplug_lcmd &= ~(0x00ff << shift);
	m_mplug_lcmd |= (data << shift);
}

void sed1356_device::mplug_resv_lcmd_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MPLUG_WR, "%s: mplug_resv_lcmd_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_mplug_resv_lcmd &= ~(0x00ff << shift);
	m_mplug_resv_lcmd |= (data << shift);
}

void sed1356_device::mplug_cmd_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MPLUG_WR, "%s: mplug_cmd_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_mplug_cmd &= ~(0x00ff << shift);
	m_mplug_cmd |= (data << shift);
}

void sed1356_device::mplug_resv_cmd_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MPLUG_WR, "%s: mplug_resv_cmd_w[%d] = %02x\n", machine().describe_context(), offset, data);
	const uint16_t shift = 8 * offset;
	m_mplug_resv_cmd &= ~(0x00ff << shift);
	m_mplug_resv_cmd |= (data << shift);
}

void sed1356_device::mplug_data_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MPLUG_WR, "%s: (not implemented) mplug_data_w[%04x]: %02x\n", machine().describe_context(), 0x1008 + offset, data);
}


uint16_t sed1356_device::bitblt_data_r(offs_t offset)
{
	uint16_t data = 0;
	if (BIT(m_bitblt_ctrl[0], BBCTRL0_ACTIVE_BIT) && m_bitblt_op == 1)
	{
		data = bitblt_read_continue();
	}

	LOGMASKED(LOG_BITBLT_RD, "%s: bitblt_data_r[%06x]: %04x\n", machine().describe_context(), 0x100000 + (offset << 1), data);
	return data;
}

void sed1356_device::bitblt_data_w(offs_t offset, uint16_t data)
{
	LOGMASKED(LOG_BITBLT_WR, "%s: bitblt_data_w[%06x]: %04x\n", machine().describe_context(), 0x100000 + (offset << 1), data);
	if (BIT(m_bitblt_ctrl[0], BBCTRL0_ACTIVE_BIT))
	{
		switch (m_bitblt_op)
		{
		case 0: // Write BitBLT with ROP
			bitblt_write_continue(data);
			return;
		case 4: // Transparent Write BitBLT
			return;
		default:
			return;
		}
	}
}

void sed1356_device::device_start()
{
	save_item(NAME(m_misc));
	save_item(NAME(m_gpio_config));
	save_item(NAME(m_gpio_ctrl));
	save_item(NAME(m_md_config));

	save_item(NAME(m_memclk_config));
	save_item(NAME(m_lcd_pclk_config));
	save_item(NAME(m_crt_pclk_config));
	save_item(NAME(m_mplug_clk_config));

	save_item(NAME(m_wait_state_sel));
	save_item(NAME(m_mem_config));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_dram_timing));

	save_item(NAME(m_panel_type));
	save_item(NAME(m_mod_rate));

	save_item(NAME(m_lcd_width));
	save_item(NAME(m_lcd_hblank_period));
	save_item(NAME(m_tft_fpline_start));
	save_item(NAME(m_tft_fpline_width));
	save_item(NAME(m_lcd_height));
	save_item(NAME(m_lcd_vblank_period));
	save_item(NAME(m_tft_fpframe_start));
	save_item(NAME(m_tft_fpframe_width));
	save_item(NAME(m_lcd_mode));
	save_item(NAME(m_lcd_misc));
	save_item(NAME(m_lcd_addr));
	save_item(NAME(m_lcd_mem_offset));
	save_item(NAME(m_lcd_pan));
	save_item(NAME(m_lcd_fifo_hi_thresh));
	save_item(NAME(m_lcd_fifo_lo_thresh));

	save_item(NAME(m_crt_width));
	save_item(NAME(m_crt_hblank_period));
	save_item(NAME(m_crt_hrtc_start));
	save_item(NAME(m_crt_hrtc_width));
	save_item(NAME(m_crt_height));
	save_item(NAME(m_crt_vblank_period));
	save_item(NAME(m_crt_vrtc_start));
	save_item(NAME(m_crt_vrtc_width));
	save_item(NAME(m_crt_output_ctrl));
	save_item(NAME(m_crt_mode));
	save_item(NAME(m_crt_addr));
	save_item(NAME(m_crt_mem_offset));
	save_item(NAME(m_crt_pan));
	save_item(NAME(m_crt_fifo_hi_thresh));
	save_item(NAME(m_crt_fifo_lo_thresh));

	save_item(NAME(m_lcd_cursor_ctrl));
	save_item(NAME(m_lcd_cursor_addr));
	save_item(NAME(m_lcd_cursor_x));
	save_item(NAME(m_lcd_cursor_y));
	save_item(NAME(m_lcd_cursor_color0));
	save_item(NAME(m_lcd_cursor_color1));
	save_item(NAME(m_lcd_cursor_fifo_thresh));

	save_item(NAME(m_crt_cursor_ctrl));
	save_item(NAME(m_crt_cursor_addr));
	save_item(NAME(m_crt_cursor_x));
	save_item(NAME(m_crt_cursor_y));
	save_item(NAME(m_crt_cursor_color0));
	save_item(NAME(m_crt_cursor_color1));
	save_item(NAME(m_crt_cursor_fifo_thresh));

	save_item(NAME(m_bitblt_ctrl));
	save_item(NAME(m_bitblt_rop));
	save_item(NAME(m_bitblt_op));
	save_item(NAME(m_bitblt_src_addr));
	save_item(NAME(m_bitblt_dst_addr));
	save_item(NAME(m_bitblt_mem_offset));
	save_item(NAME(m_bitblt_width));
	save_item(NAME(m_bitblt_height));
	save_item(NAME(m_bitblt_bgcolor));
	save_item(NAME(m_bitblt_fgcolor));

	save_item(NAME(m_lut_mode));
	save_item(NAME(m_lut_addr));
	save_item(NAME(m_lut_data));

	save_item(NAME(m_power_config));
	save_item(NAME(m_power_status));

	save_item(NAME(m_watchdog));

	save_item(NAME(m_display_mode));

	save_item(NAME(m_mplug_lcmd));
	save_item(NAME(m_mplug_resv_lcmd));
	save_item(NAME(m_mplug_cmd));
	save_item(NAME(m_mplug_resv_cmd));
}

void sed1356_device::device_reset()
{
	m_misc = 0;
	m_gpio_config = 0;
	m_gpio_ctrl = 0;
	m_md_config[0] = 0;
	m_md_config[1] = 0;

	m_memclk_config = 0;
	m_lcd_pclk_config = 0;
	m_crt_pclk_config = 0;
	m_mplug_clk_config = 0;

	m_wait_state_sel = 0;
	m_mem_config = 0;
	m_dram_refresh = 0;
	m_dram_timing[0] = 0;
	m_dram_timing[1] = 0;

	m_panel_type = 0;
	m_mod_rate = 0;

	m_lcd_width = 0;
	m_lcd_hblank_period = 0;
	m_tft_fpline_start = 0;
	m_tft_fpline_width = 0;
	m_lcd_height = 0;
	m_lcd_vblank_period = 0;
	m_tft_fpframe_start = 0;
	m_tft_fpframe_width = 0;
	m_lcd_mode = 0;
	m_lcd_misc = 0;
	m_lcd_addr = 0;
	m_lcd_mem_offset = 0;
	m_lcd_pan = 0;
	m_lcd_fifo_hi_thresh = 0;
	m_lcd_fifo_lo_thresh = 0;

	m_crt_width = 0;
	m_crt_hblank_period = 0;
	m_crt_hrtc_start = 0;
	m_crt_hrtc_width = 0;
	m_crt_height = 0;
	m_crt_vblank_period = 0;
	m_crt_vrtc_start = 0;
	m_crt_vrtc_width = 0;
	m_crt_output_ctrl = 0;
	m_crt_mode = 0;
	m_crt_addr = 0;
	m_crt_mem_offset = 0;
	m_crt_pan = 0;
	m_crt_fifo_hi_thresh = 0;
	m_crt_fifo_lo_thresh = 0;

	m_lcd_cursor_ctrl = 0;
	m_lcd_cursor_addr = 0;
	m_lcd_cursor_x = 0;
	m_lcd_cursor_y = 0;
	m_lcd_cursor_color0[0] = 0;
	m_lcd_cursor_color0[1] = 0;
	m_lcd_cursor_color0[2] = 0;
	m_lcd_cursor_color1[0] = 0;
	m_lcd_cursor_color1[1] = 0;
	m_lcd_cursor_color1[2] = 0;
	m_lcd_cursor_fifo_thresh = 0;

	m_crt_cursor_ctrl = 0;
	m_crt_cursor_addr = 0;
	m_crt_cursor_x = 0;
	m_crt_cursor_y = 0;
	m_crt_cursor_color0[0] = 0;
	m_crt_cursor_color0[1] = 0;
	m_crt_cursor_color0[2] = 0;
	m_crt_cursor_color1[0] = 0;
	m_crt_cursor_color1[1] = 0;
	m_crt_cursor_color1[2] = 0;
	m_crt_cursor_fifo_thresh = 0;

	m_bitblt_ctrl[0] = 0;
	m_bitblt_ctrl[1] = 0;
	m_bitblt_rop = 0;
	m_bitblt_op = 0;
	m_bitblt_src_addr = 0;
	m_bitblt_dst_addr = 0;
	m_bitblt_mem_offset = 0;
	m_bitblt_width = 0;
	m_bitblt_height = 0;
	m_bitblt_bgcolor = 0;
	m_bitblt_fgcolor = 0;

	m_lut_mode = 0;
	m_lut_addr = 0;
	m_lut_data = 0;

	m_power_config = 0;
	m_power_status = 0;

	m_watchdog = 0;

	m_display_mode = 0;

	m_mplug_lcmd = 0;
	m_mplug_resv_lcmd = 0;
	m_mplug_cmd = 0;
	m_mplug_resv_cmd = 0;
}

uint32_t sed1356_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const uint16_t *src = (uint16_t*)&m_vram[y * (0x500 >> 2)];
		uint32_t *dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const uint16_t pix = src[x];
			const uint8_t src_red = (uint8_t)((pix >> 11) & 0x1f);
			const uint8_t src_grn = (uint8_t)((pix >> 5) & 0x3f);
			const uint8_t src_blu = (uint8_t)(pix & 0x1f);
			const uint32_t dst_red = ((src_red << 3) | (src_red >> 2)) & 0x000000ff;
			const uint32_t dst_grn = ((src_grn << 2) | (src_grn >> 4)) & 0x000000ff;
			const uint32_t dst_blu = ((src_blu << 3) | (src_blu >> 2)) & 0x000000ff;
			dst[x] = 0xff000000 | (dst_red << 16) | (dst_grn << 8) | dst_blu;
		}
	}
	return 0;
}
