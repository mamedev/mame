// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Epson SED1356 LCD Controller emulation skeleton

    Status:
    - Currently hard-coded for use with the Jornada 720 driver.
    - Register contents are correctly stored, logged and masked, but
      register handling is otherwise non-present.

**********************************************************************/

#ifndef MAME_VIDEO_SED1356_H
#define MAME_VIDEO_SED1356_H

#pragma once

class sed1356_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	sed1356_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t revision_r(offs_t offset);

	uint8_t misc_r(offs_t offset);
	void misc_w(offs_t offset, uint8_t data);

	uint8_t gpio_config_r(offs_t offset);
	void gpio_config_w(offs_t offset, uint8_t data);
	uint8_t gpio_ctrl_r(offs_t offset);
	void gpio_ctrl_w(offs_t offset, uint8_t data);

	uint8_t md_config_readback_r(offs_t offset);
	void md_config_readback_w(offs_t offset, uint8_t data);

	uint8_t mem_clk_config_r(offs_t offset);
	uint8_t lcd_pclk_config_r(offs_t offset);
	uint8_t crt_pclk_config_r(offs_t offset);
	uint8_t mplug_clk_config_r(offs_t offset);
	void mem_clk_config_w(offs_t offset, uint8_t data);
	void lcd_pclk_config_w(offs_t offset, uint8_t data);
	void crt_pclk_config_w(offs_t offset, uint8_t data);
	void mplug_clk_config_w(offs_t offset, uint8_t data);

	uint8_t mem_wait_select_r(offs_t offset);
	uint8_t mem_config_r(offs_t offset);
	uint8_t dram_refresh_select_r(offs_t offset);
	uint8_t dram_timing_ctrl_r(offs_t offset);
	void mem_wait_select_w(offs_t offset, uint8_t data);
	void mem_config_w(offs_t offset, uint8_t data);
	void dram_refresh_select_w(offs_t offset, uint8_t data);
	void dram_timing_ctrl0_w(offs_t offset, uint8_t data);
	void dram_timing_ctrl1_w(offs_t offset, uint8_t data);

	uint8_t panel_type_r(offs_t offset);
	uint8_t mod_rate_r(offs_t offset);
	void panel_type_w(offs_t offset, uint8_t data);
	void mod_rate_w(offs_t offset, uint8_t data);

	uint8_t lcd_display_width_r(offs_t offset);
	uint8_t lcd_hblank_period_r(offs_t offset);
	uint8_t tft_fpline_start_pos_r(offs_t offset);
	uint8_t tft_fpline_pulse_width_r(offs_t offset);
	uint8_t lcd_display_height_r(offs_t offset);
	uint8_t lcd_vblank_period_r(offs_t offset);
	uint8_t tft_fpframe_start_pos_r(offs_t offset);
	uint8_t tft_fpframe_pulse_width_r(offs_t offset);
	uint8_t lcd_display_mode_r(offs_t offset);
	uint8_t lcd_misc_r(offs_t offset);
	uint8_t lcd_display_start_addr_r(offs_t offset);
	uint8_t lcd_mem_addr_offset_r(offs_t offset);
	uint8_t lcd_pixel_pan_r(offs_t offset);
	uint8_t lcd_display_fifo_hi_thresh_r(offs_t offset);
	uint8_t lcd_display_fifo_lo_thresh_r(offs_t offset);
	void lcd_display_width_w(offs_t offset, uint8_t data);
	void lcd_hblank_period_w(offs_t offset, uint8_t data);
	void tft_fpline_start_pos_w(offs_t offset, uint8_t data);
	void tft_fpline_pulse_width_w(offs_t offset, uint8_t data);
	void lcd_display_height_w(offs_t offset, uint8_t data);
	void lcd_vblank_period_w(offs_t offset, uint8_t data);
	void tft_fpframe_start_pos_w(offs_t offset, uint8_t data);
	void tft_fpframe_pulse_width_w(offs_t offset, uint8_t data);
	void lcd_display_mode_w(offs_t offset, uint8_t data);
	void lcd_misc_w(offs_t offset, uint8_t data);
	void lcd_display_start_addr_w(offs_t offset, uint8_t data);
	void lcd_mem_addr_offset_w(offs_t offset, uint8_t data);
	void lcd_pixel_pan_w(offs_t offset, uint8_t data);
	void lcd_display_fifo_hi_thresh_w(offs_t offset, uint8_t data);
	void lcd_display_fifo_lo_thresh_w(offs_t offset, uint8_t data);

	uint8_t crt_display_width_r(offs_t offset);
	uint8_t crt_hblank_period_r(offs_t offset);
	uint8_t crt_hrtc_start_pos_r(offs_t offset);
	uint8_t crt_hrtc_pulse_width_r(offs_t offset);
	uint8_t crt_display_height_r(offs_t offset);
	uint8_t crt_vblank_period_r(offs_t offset);
	uint8_t crt_vrtc_start_pos_r(offs_t offset);
	uint8_t crt_vrtc_pulse_width_r(offs_t offset);
	uint8_t crt_output_ctrl_r(offs_t offset);
	uint8_t crt_display_mode_r(offs_t offset);
	uint8_t crt_display_start_addr_r(offs_t offset);
	uint8_t crt_mem_addr_offset_r(offs_t offset);
	uint8_t crt_pixel_pan_r(offs_t offset);
	uint8_t crt_display_fifo_hi_thresh_r(offs_t offset);
	uint8_t crt_display_fifo_lo_thresh_r(offs_t offset);
	void crt_display_width_w(offs_t offset, uint8_t data);
	void crt_hblank_period_w(offs_t offset, uint8_t data);
	void crt_hrtc_start_pos_w(offs_t offset, uint8_t data);
	void crt_hrtc_pulse_width_w(offs_t offset, uint8_t data);
	void crt_display_height_w(offs_t offset, uint8_t data);
	void crt_vblank_period_w(offs_t offset, uint8_t data);
	void crt_vrtc_start_pos_w(offs_t offset, uint8_t data);
	void crt_vrtc_pulse_width_w(offs_t offset, uint8_t data);
	void crt_output_ctrl_w(offs_t offset, uint8_t data);
	void crt_display_mode_w(offs_t offset, uint8_t data);
	void crt_display_start_addr_w(offs_t offset, uint8_t data);
	void crt_mem_addr_offset_w(offs_t offset, uint8_t data);
	void crt_pixel_pan_w(offs_t offset, uint8_t data);
	void crt_display_fifo_hi_thresh_w(offs_t offset, uint8_t data);
	void crt_display_fifo_lo_thresh_w(offs_t offset, uint8_t data);

	uint8_t lcd_cursor_ctrl_r(offs_t offset);
	uint8_t lcd_cursor_start_addr_r(offs_t offset);
	uint8_t lcd_cursor_x_r(offs_t offset);
	uint8_t lcd_cursor_y_r(offs_t offset);
	uint8_t lcd_cursor_color0_r(offs_t offset);
	uint8_t lcd_cursor_color1_r(offs_t offset);
	uint8_t lcd_cursor_fifo_thresh_r(offs_t offset);
	void lcd_cursor_ctrl_w(offs_t offset, uint8_t data);
	void lcd_cursor_start_addr_w(offs_t offset, uint8_t data);
	void lcd_cursor_x_w(offs_t offset, uint8_t data);
	void lcd_cursor_y_w(offs_t offset, uint8_t data);
	void lcd_cursor_color0_blu_w(offs_t offset, uint8_t data);
	void lcd_cursor_color0_grn_w(offs_t offset, uint8_t data);
	void lcd_cursor_color0_red_w(offs_t offset, uint8_t data);
	void lcd_cursor_color1_blu_w(offs_t offset, uint8_t data);
	void lcd_cursor_color1_grn_w(offs_t offset, uint8_t data);
	void lcd_cursor_color1_red_w(offs_t offset, uint8_t data);
	void lcd_cursor_fifo_thresh_w(offs_t offset, uint8_t data);

	uint8_t crt_cursor_ctrl_r(offs_t offset);
	uint8_t crt_cursor_start_addr_r(offs_t offset);
	uint8_t crt_cursor_x_r(offs_t offset);
	uint8_t crt_cursor_y_r(offs_t offset);
	uint8_t crt_cursor_color0_r(offs_t offset);
	uint8_t crt_cursor_color1_r(offs_t offset);
	uint8_t crt_cursor_fifo_thresh_r(offs_t offset);
	void crt_cursor_ctrl_w(offs_t offset, uint8_t data);
	void crt_cursor_start_addr_w(offs_t offset, uint8_t data);
	void crt_cursor_x_w(offs_t offset, uint8_t data);
	void crt_cursor_y_w(offs_t offset, uint8_t data);
	void crt_cursor_color0_blu_w(offs_t offset, uint8_t data);
	void crt_cursor_color0_grn_w(offs_t offset, uint8_t data);
	void crt_cursor_color0_red_w(offs_t offset, uint8_t data);
	void crt_cursor_color1_blu_w(offs_t offset, uint8_t data);
	void crt_cursor_color1_grn_w(offs_t offset, uint8_t data);
	void crt_cursor_color1_red_w(offs_t offset, uint8_t data);
	void crt_cursor_fifo_thresh_w(offs_t offset, uint8_t data);

	template <bool Linear> void bitblt_solid_fill();
	uint16_t bitblt_rop(const uint8_t rop, const uint16_t s, const uint16_t d);
	void bitblt_async_advance(uint32_t &addr);
	void bitblt_write_continue(const uint16_t s);
	void bitblt_write();
	uint16_t bitblt_read_continue();
	void bitblt_read();
	template <bool SrcLinear, bool DstLinear> void bitblt_move_negative();
	void bitblt_execute_command();
	uint8_t bitblt_ctrl0_r(offs_t offset);
	uint8_t bitblt_ctrl1_r(offs_t offset);
	uint8_t bitblt_rop_code_r(offs_t offset);
	uint8_t bitblt_operation_r(offs_t offset);
	uint8_t bitblt_src_start_addr_r(offs_t offset);
	uint8_t bitblt_dst_start_addr_r(offs_t offset);
	uint8_t bitblt_mem_addr_offset_r(offs_t offset);
	uint8_t bitblt_width_r(offs_t offset);
	uint8_t bitblt_height_r(offs_t offset);
	uint8_t bitblt_bgcolor_r(offs_t offset);
	uint8_t bitblt_fgcolor_r(offs_t offset);
	void bitblt_ctrl0_w(offs_t offset, uint8_t data);
	void bitblt_ctrl1_w(offs_t offset, uint8_t data);
	void bitblt_rop_code_w(offs_t offset, uint8_t data);
	void bitblt_operation_w(offs_t offset, uint8_t data);
	void bitblt_src_start_addr_w(offs_t offset, uint8_t data);
	void bitblt_dst_start_addr_w(offs_t offset, uint8_t data);
	void bitblt_mem_addr_offset_w(offs_t offset, uint8_t data);
	void bitblt_width_w(offs_t offset, uint8_t data);
	void bitblt_height_w(offs_t offset, uint8_t data);
	void bitblt_bgcolor_w(offs_t offset, uint8_t data);
	void bitblt_fgcolor_w(offs_t offset, uint8_t data);

	uint8_t lut_mode_r(offs_t offset);
	uint8_t lut_addr_r(offs_t offset);
	uint8_t lut_data_r(offs_t offset);
	void lut_mode_w(offs_t offset, uint8_t data);
	void lut_addr_w(offs_t offset, uint8_t data);
	void lut_data_w(offs_t offset, uint8_t data);

	uint8_t power_config_r(offs_t offset);
	uint8_t power_status_r(offs_t offset);
	void power_config_w(offs_t offset, uint8_t data);

	uint8_t cpu_mem_watchdog_r(offs_t offset);
	void cpu_mem_watchdog_w(offs_t offset, uint8_t data);

	uint8_t display_mode_r(offs_t offset);
	void display_mode_w(offs_t offset, uint8_t data);

	uint8_t mplug_lcmd_r(offs_t offset);
	uint8_t mplug_resv_lcmd_r(offs_t offset);
	uint8_t mplug_cmd_r(offs_t offset);
	uint8_t mplug_resv_cmd_r(offs_t offset);
	uint8_t mplug_data_r(offs_t offset);
	void mplug_lcmd_w(offs_t offset, uint8_t data);
	void mplug_resv_lcmd_w(offs_t offset, uint8_t data);
	void mplug_cmd_w(offs_t offset, uint8_t data);
	void mplug_resv_cmd_w(offs_t offset, uint8_t data);
	void mplug_data_w(offs_t offset, uint8_t data);

	uint16_t bitblt_data_r(offs_t offset);
	void bitblt_data_w(offs_t offset, uint16_t data);

	enum
	{
		MISC_RMS_BIT            = 7,
		MISC_MASK               = 0x80,

		GPCFG_GPIO1_BIT         = 1,
		GPCFG_GPIO2_BIT         = 2,
		GPCFG_GPIO3_BIT         = 3,
		GPCFG_MASK              = 0x0e,

		GPCTRL_GPIO1_BIT        = 1,
		GPCTRL_GPIO2_BIT        = 2,
		GPCTRL_GPIO3_BIT        = 3,
		GPCTRL_MASK             = 0x0e,

		MEMCLK_SRC_BIT          = 0,
		MEMCLK_DIV_BIT          = 4,
		MEMCLK_MASK             = 0x11,

		LCDCLK_SRC_BIT          = 0,
		LCDCLK_SRC_MASK         = 0x03,
		LCDCLK_DIV_BIT          = 4,
		LCDCLK_DIV_MASK         = 0x30,
		LCDCLK_MASK             = 0x33,

		CRTCLK_SRC_BIT          = 0,
		CRTCLK_SRC_MASK         = 0x03,
		CRTCLK_DIV_BIT          = 4,
		CRTCLK_DIV_MASK         = 0x30,
		CRTCLK_2X_BIT           = 7,
		CRTCLK_MASK             = 0xb3,

		PLUGCLK_SRC_BIT         = 0,
		PLUGCLK_SRC_MASK        = 0x03,
		PLUGCLK_DIV_BIT         = 4,
		PLUGCLK_DIV_MASK        = 0x30,
		PLUGCLK_MASK            = 0x33,

		CPUWAIT_SEL_BIT         = 0,
		CPUWAIT_SEL_MASK        = 0x03,
		CPUWAIT_MASK            = 0x03,

		MEMCFG_TYPE_BIT         = 0,
		MEMCFG_TYPE_MASK        = 0x03,
		MEMCFG_MASK             = 0x03,

		MEMRFSH_RATE_BIT        = 0,
		MEMRFSH_RATE_MASK       = 0x07,
		MEMRFSH_SEL_BIT         = 6,
		MEMRFSH_SEL_MASK        = 0xc0,
		MEMRFSH_MASK            = 0xc7,

		DRAMTIME1_MASK          = 0x03,

		PTYPE_TFT_PASS_BIT      = 0,
		PTYPE_COUNT_BIT         = 1,
		PTYPE_COLOR_BIT         = 2,
		PTYPE_FORMAT_BIT        = 3,
		PTYPE_WIDTH_BIT         = 4,
		PTYPE_WIDTH_MASK        = 0x30,
		PTYPE_EL_BIT            = 7,
		PTYPE_MASK              = 0xbf,

		MODRATE_MASK            = 0x3f,

		LCDW_MASK               = 0x7f,

		LCDHBL_MASK             = 0x1f,

		TFTFPLS_MASK            = 0x1f,

		TFTFPLW_WIDTH_BIT       = 0,
		TFTFPLW_WIDTH_MASK      = 0x0f,
		TFTFPLW_POL_BIT         = 7,
		TFTFPLW_MASK            = 0x8f,

		LCDH_MASK               = 0x03ff,

		LCDVBL_VALUE_BIT        = 0,
		LCDVBL_VALUE_MASK       = 0x3f,
		LCDVBL_STATUS_BIT       = 7,
		LCDVBL_MASK             = 0x3f,

		TFTFPFS_MASK            = 0x3f,

		TFTFPFW_WIDTH_BIT       = 0,
		TFTFPFW_WIDTH_MASK      = 0x07,
		TFTFPFW_POL_BIT         = 7,
		TFTFPFW_MASK            = 0x87,

		LCDMODE_BPP_BIT         = 0,
		LCDMODE_BPP_MASK        = 0x07,
		LCDMODE_SWIVEN1_BIT     = 4,
		LCDMODE_BLANK_BIT       = 7,
		LCDMODE_MASK            = 0x97,

		LCDMISC_DPDIS_BIT       = 0,
		LCDMISC_DITHDIS_BIT     = 1,
		LCDMISC_MASK            = 0x03,

		LCDDS_MASK              = 0x000fffff,

		LCDMOFS_MASK            = 0x07ff,

		LCDPAN_PAN_BIT          = 0,
		LCDPAN_PAN_MASK         = 0x03,
		LCDPAN_MASK             = 0x03,

		LCDFIFO_MASK            = 0x3f,

		CRTW_MASK               = 0x7f,

		CRTHBL_MASK             = 0x3f,

		CRTHRTS_MASK            = 0x3f,

		CRTHRTW_WIDTH_BIT       = 0,
		CRTHRTW_WIDTH_MASK      = 0x0f,
		CRTHRTW_POL_BIT         = 7,
		CRTHRTW_MASK            = 0x8f,

		CRTH_MASK               = 0x03ff,

		CRTVBL_VALUE_BIT        = 0,
		CRTVBL_VALUE_MASK       = 0x7f,
		CRTVBL_STATUS_BIT       = 7,
		CRTVBL_MASK             = 0xff,

		CRTVRTS_MASK            = 0x7f,

		CRTVRTW_WIDTH_BIT       = 0,
		CRTVRTW_WIDTH_MASK      = 0x07,
		CRTVRTW_POL_BIT         = 7,
		CRTVRTW_MASK            = 0x87,

		CRTCTRL_PAL_BIT         = 0,
		CRTCTRL_SVID_BIT        = 1,
		CRTCTRL_DACLVL_BIT      = 3,
		CRTCTRL_LUMF_BIT        = 4,
		CRTCTRL_CHRF_BIT        = 5,
		CRTCTRL_MASK            = 0x3b,

		CRTMODE_BPP_BIT         = 0,
		CRTMODE_BPP_MASK        = 0x07,
		CRTMODE_BLANK_BIT       = 7,
		CRTMODE_MASK            = 0x87,

		CRTDS_MASK              = 0x000fffff,

		CRTMOFS_MASK            = 0x07ff,

		CRTPAN_PAN_BIT          = 0,
		CRTPAN_PAN_MASK         = 0x03,
		CRTPAN_MASK             = 0x03,

		CRTFIFO_MASK            = 0x3f,

		CURCTRL_MODE_BIT        = 0,
		CURCTRL_MODE_MASK       = 0x03,
		CURCTRL_MASK            = 0x03,

		CURX_SIGN_BIT           = 15,
		CURX_MASK               = 0x83ff,

		CURY_SIGN_BIT           = 15,
		CURY_MASK               = 0x83ff,

		CURB_MASK               = 0x1f,
		CURG_MASK               = 0x3f,
		CURR_MASK               = 0x1f,

		CUR_FIFO_MASK           = 0x0f,

		BBCTRL0_SRCLIN_BIT      = 0,
		BBCTRL0_DSTLIN_BIT      = 1,
		BBCTRL0_FULL_BIT        = 4,
		BBCTRL0_HALF_BIT        = 5,
		BBCTRL0_ANY_BIT         = 6,
		BBCTRL0_ACTIVE_BIT      = 7,
		BBCTRL0_WR_MASK         = 0x03,

		BBCTRL1_COLFMT_BIT      = 0,
		BBCTRL1_FIFO_DEPTH_BIT  = 4,
		BBCTRL1_WR_MASK         = 0x11,

		BBCODE_MASK             = 0x0f,

		BBOP_MASK               = 0x0f,

		BBSRC_MASK              = 0x001fffff,
		BBDST_MASK              = 0x001fffff,

		BBMADR_MASK             = 0x07ff,

		BBWIDTH_MASK            = 0x03ff,

		BBHEIGHT_MASK           = 0x03ff,

		LUTMODE_MODE_BIT        = 0,
		LUTMODE_MODE_MASK       = 0x03,
		LUTMODE_MASK            = 0x03,

		LUTDATA_BIT             = 4,
		LUTDATA_MASK            = 0xf0,

		PWRCFG_ENABLE_BIT       = 0,
		PWRCFG_MASK             = 0x01,

		PWRSTAT_MCPSS_BIT       = 0,
		PWRSTAT_LCDPSS_BIT      = 1,
		PWRSTAT_MASK            = 0x03,

		WATCHDOG_MASK           = 0x3f,

		DISPMODE_MODE_BIT       = 0,
		DISPMODE_MODE_MASK      = 0x07,
		DISPMODE_SWIVEN0_BIT    = 6,
		DISPMODE_MASK           = 0x47
	};

	uint8_t m_misc;
	uint8_t m_gpio_config;
	uint8_t m_gpio_ctrl;
	uint8_t m_md_config[2];

	uint8_t m_memclk_config;
	uint8_t m_lcd_pclk_config;
	uint8_t m_crt_pclk_config;
	uint8_t m_mplug_clk_config;

	uint8_t m_wait_state_sel;
	uint8_t m_mem_config;
	uint8_t m_dram_refresh;
	uint8_t m_dram_timing[2];

	uint8_t m_panel_type;
	uint8_t m_mod_rate;

	uint8_t m_lcd_width;
	uint8_t m_lcd_hblank_period;
	uint8_t m_tft_fpline_start;
	uint8_t m_tft_fpline_width;
	uint16_t m_lcd_height;
	uint8_t m_lcd_vblank_period;
	uint8_t m_tft_fpframe_start;
	uint8_t m_tft_fpframe_width;
	uint8_t m_lcd_mode;
	uint8_t m_lcd_misc;
	uint32_t m_lcd_addr;
	uint16_t m_lcd_mem_offset;
	uint8_t m_lcd_pan;
	uint8_t m_lcd_fifo_hi_thresh;
	uint8_t m_lcd_fifo_lo_thresh;

	uint8_t m_crt_width;
	uint8_t m_crt_hblank_period;
	uint8_t m_crt_hrtc_start;
	uint8_t m_crt_hrtc_width;
	uint16_t m_crt_height;
	uint8_t m_crt_vblank_period;
	uint8_t m_crt_vrtc_start;
	uint8_t m_crt_vrtc_width;
	uint8_t m_crt_output_ctrl;
	uint8_t m_crt_mode;
	uint32_t m_crt_addr;
	uint16_t m_crt_mem_offset;
	uint8_t m_crt_pan;
	uint8_t m_crt_fifo_hi_thresh;
	uint8_t m_crt_fifo_lo_thresh;

	uint8_t m_lcd_cursor_ctrl;
	uint8_t m_lcd_cursor_addr;
	uint16_t m_lcd_cursor_x;
	uint16_t m_lcd_cursor_y;
	uint8_t m_lcd_cursor_color0[3];
	uint8_t m_lcd_cursor_color1[3];
	uint8_t m_lcd_cursor_fifo_thresh;

	uint8_t m_crt_cursor_ctrl;
	uint8_t m_crt_cursor_addr;
	uint16_t m_crt_cursor_x;
	uint16_t m_crt_cursor_y;
	uint8_t m_crt_cursor_color0[3];
	uint8_t m_crt_cursor_color1[3];
	uint8_t m_crt_cursor_fifo_thresh;

	uint8_t m_bitblt_ctrl[2];
	uint8_t m_bitblt_rop;
	uint8_t m_bitblt_op;
	uint32_t m_bitblt_src_addr;
	uint32_t m_bitblt_dst_addr;
	uint16_t m_bitblt_mem_offset;
	uint16_t m_bitblt_width;
	uint16_t m_bitblt_height;
	uint16_t m_bitblt_bgcolor;
	uint16_t m_bitblt_fgcolor;

	uint32_t m_bitblt_curr_src_addr;
	uint32_t m_bitblt_curr_dst_addr;
	uint32_t m_bitblt_x;
	uint32_t m_bitblt_y;

	uint8_t m_lut_mode;
	uint8_t m_lut_addr;
	uint8_t m_lut_data;

	uint8_t m_power_config;
	uint8_t m_power_status;

	uint8_t m_watchdog;

	uint8_t m_display_mode;

	uint16_t m_mplug_lcmd;
	uint16_t m_mplug_resv_lcmd;
	uint16_t m_mplug_cmd;
	uint16_t m_mplug_resv_cmd;

	required_shared_ptr<uint32_t> m_vram;
};


// device type definition
DECLARE_DEVICE_TYPE(SED1356, sed1356_device)

#endif // MAME_VIDEO_SED1356_H
