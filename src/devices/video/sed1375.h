// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Epson SED1375 LCD Controller emulation skeleton

    - Currently hard-coded for use with the Palm IIIc driver.
    - TODO: Find more test-case systems.

**********************************************************************/

#ifndef MAME_VIDEO_SED1375_H
#define MAME_VIDEO_SED1375_H

#pragma once

class sed1375_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	sed1375_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u32 get_pixel(int screen_idx, int x, int y);

	void vram_w(offs_t offset, u8 data);
	u8 vram_r(offs_t offset);

	void mode0_w(u8 data);
	void mode1_w(u8 data);
	void mode2_w(u8 data);
	void panel_hsize_w(u8 data);
	void panel_vsize_lsb_w(u8 data);
	void panel_vsize_msb_w(u8 data);
	void fpline_start_w(u8 data);
	void hblank_w(u8 data);
	void fpframe_start_w(u8 data);
	void vblank_w(u8 data);
	void mod_rate_w(u8 data);
	void screen1_start_lsb_w(u8 data);
	void screen1_start_msb_w(u8 data);
	void screen2_start_lsb_w(u8 data);
	void screen2_start_msb_w(u8 data);
	void screen1_start_ovf_w(u8 data);
	void mem_addr_offset_w(u8 data);
	void screen1_vsize_lsb_w(u8 data);
	void screen1_vsize_msb_w(u8 data);
	void lut_addr_w(u8 data);
	void lut_data_w(u8 data);
	void gpio_config_w(u8 data);
	void gpio_w(u8 data);
	void scratch_w(u8 data);
	void swivel_mode_w(u8 data);
	void swivel_bytecnt_w(u8 data);

	u8 revision_r();
	u8 mode0_r();
	u8 mode1_r();
	u8 mode2_r();
	u8 panel_hsize_r();
	u8 panel_vsize_lsb_r();
	u8 panel_vsize_msb_r();
	u8 fpline_start_r();
	u8 hblank_r();
	u8 fpframe_start_r();
	u8 vblank_r();
	u8 mod_rate_r();
	u8 screen1_start_lsb_r();
	u8 screen1_start_msb_r();
	u8 screen2_start_lsb_r();
	u8 screen2_start_msb_r();
	u8 screen1_start_ovf_r();
	u8 mem_addr_offset_r();
	u8 screen1_vsize_lsb_r();
	u8 screen1_vsize_msb_r();
	u8 lut_addr_r();
	u8 lut_data_r();
	u8 gpio_config_r();
	u8 gpio_r();
	u8 scratch_r();
	u8 swivel_mode_r();
	u8 swivel_bytecnt_r();

	enum : u8
	{
		MODE0_MASK              = 0xff,
		MODE0_WIDTH_MASK        = 0x03,
		MODE0_FPSHIFT_BIT       = 2,
		MODE0_FPFRAME_POL_BIT   = 3,
		MODE0_FPLINE_POL_BIT    = 4,
		MODE0_COLOR_BIT         = 5,
		MODE0_DUAL_BIT          = 6,
		MODE0_TFT_BIT           = 7,

		MODE1_MASK              = 0xff,
		MODE1_SWINVERT_BIT      = 0,
		MODE1_HWINVERT_BIT      = 1,
		MODE1_FRREPEAT_BIT      = 2,
		MODE1_BLANK_BIT         = 3,
		MODE1_CLKDIV_BIT        = 4,
		MODE1_HIPERF_BIT        = 5,
		MODE1_BPP_MASK          = 0xc0,
		MODE1_BPP_SHIFT         = 6,

		MODE2_MASK              = 0x0f,
		MODE2_SWPWRSAVE_MASK    = 0x03,
		MODE2_SWPWRSAVE_SHIFT   = 0,
		MODE2_HWPWRSAVE_BIT     = 2,
		MODE2_LCDPWR_OVR_BIT    = 3,

		PANEL_HSIZE_MASK        = 0x7f,

		FPLINE_START_MASK       = 0x1f,

		HBLANK_MASK             = 0x1f,

		FPFRAME_START_MASK      = 0x3f,

		VBLANK_MASK             = 0x3f,
		VBLANK_VBL_BIT          = 7,

		MODRATE_MASK            = 0x3f,

		START_OVF_MASK          = 0x01,

		MEM_ADDR_OFFSET_MASK    = 0xff,

		LUT_ADDR_MASK           = 0xff,

		LUT_DATA_MASK           = 0xf0,
		LUT_DATA_SHIFT          = 4,

		GPIO_CONFIG_MASK        = 0x1f,
		GPIO_STATUS_MASK        = 0x1f,

		SCRATCH_MASK            = 0xff,

		SWIVEL_MODE_MASK        = 0xc3,
		SWIVEL_CLKSEL_MASK      = 0x03,
		SWIVEL_MODE_BIT         = 6,
		SWIVEL_ENABLE_BIT       = 7,

		SWIVEL_BYTECNT_MASK     = 0xff
	};

	enum : u16
	{
		PANEL_VSIZE_MASK        = 0x03ff,

		SCREEN1_START_MASK      = 0xffff,
		SCREEN2_START_MASK      = 0xffff,

		SCREEN1_VSIZE_MASK      = 0x03ff
	};

	memory_share_creator<u8> m_vram;

	u8  m_revision;
	u8  m_mode[3];
	u8  m_panel_hsize;
	u16 m_panel_vsize;
	u8  m_fpline_start;
	u8  m_hblank_period;
	u8  m_fpframe_start;
	u8  m_vblank_period;
	u8  m_mod_rate;
	u16 m_screen_start[2];
	u8  m_screen_start_ovf;
	u8  m_mem_addr_offset;
	u16 m_screen1_vsize;
	u8  m_lut_addr;
	u8  m_lut_index;
	u8  m_red_lut[256];
	u8  m_green_lut[256];
	u8  m_blue_lut[256];
	u8  m_gpio_config;
	u8  m_gpio_status;
	u8  m_scratch;
	u8  m_swivel_mode;
	u8  m_swivel_bytecnt;
};


// device type definition
DECLARE_DEVICE_TYPE(SED1375, sed1375_device)

#endif // MAME_VIDEO_SED1375_H
