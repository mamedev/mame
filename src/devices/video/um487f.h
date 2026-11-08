// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote
/***************************************************************************

    UMC UM487F HCGA (Hercules + CGA) single-chip video controller

***************************************************************************/

#ifndef MAME_VIDEO_UM487F_H
#define MAME_VIDEO_UM487F_H

#pragma once

#include "video/mc6845.h"


class um487f_device : public device_t,
					  public device_video_interface,
					  public device_palette_interface
{
public:
	// construction/destruction
	// clock is the CGA base clock fed to the OSC pin (14.31818 MHz)
	um487f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	// MGA base clock fed to the MOSC pin (16.257 MHz), 0 if the pin is not connected
	void set_mga_clock(uint32_t clock) { m_mga_clock = clock; }
	void set_mga_clock(const XTAL &clock) { m_mga_clock = clock.value(); }
	// state of the SWS/SWR mode straps at power on (true = MGA mode)
	void set_mga_mode(bool mga) { m_strap_mga = mga; }
	// SW3: allow switching between CGA and MGA modes by software
	void set_mode_change_enable(bool enable) { m_strap_change_enable = enable; }
	// external character generator ROM (8 KiB, 2764 style) fed through CD0-CD7
	template <typename T> void set_chargen(T &&tag) { m_chargen.set_tag(std::forward<T>(tag)); }
	// RGBI (bit 3 = IOUT ... bit 0 = BOUT) to monitor color conversion done by the board
	using rgbi_delegate = device_delegate<rgb_t (uint8_t rgbi)>;
	template <typename... T> void set_rgbi_callback(T &&... args) { m_rgbi_cb.set(std::forward<T>(args)...); }

	auto hsync_callback() { return m_hsync_cb.bind(); }
	auto vsync_callback() { return m_vsync_cb.bind(); }

	// CPU interface
	// I/O space 0x3b0-0x3df (offset 0x00-0x2f), A0-A9 are decoded and AEN must be active
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	// video memory window 0xb0000-0xbffff (A0-A15, while MEMSELB is active)
	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	// device_palette_interface implementation
	virtual uint32_t palette_entries() const noexcept override { return 16 + 3; }

private:
	enum : uint8_t
	{
		// mode control register (0x3d8 / 0x3b8)
		MODE_HIRES       = 0x01, // CGA: 80x25 text (high clock)
		MODE_GRAPHICS    = 0x02,
		MODE_BW          = 0x04, // CGA: black/white (alternate palette)
		MODE_VIDEO       = 0x08, // video enable
		MODE_640         = 0x10, // CGA: 640x200 graphics
		MODE_BLINK       = 0x20, // blink enable
		MODE_CHANGE      = 0x40, // enable mode change
		MODE_PAGE1       = 0x80, // MGA: display page 1

		// configuration register (0x3bf)
		CONFIG_MGA_GFX   = 0x01, // enable MGA graphics
		CONFIG_MGA_PAGE1 = 0x02, // enable MGA page 1
		CONFIG_CGA       = 0x40, // select CGA (1) or MGA (0) mode

		// status register (0x3da / 0x3ba)
		STATUS_CGA_NODISP = 0x01, // CGA: non display period
		STATUS_MGA_HSYNC  = 0x01, // MGA: horizontal sync period
		STATUS_LPEN_TRIG  = 0x02, // light pen latched
		STATUS_LPEN_SW    = 0x04, // light pen switch on
		STATUS_CGA_VSYNC  = 0x08, // CGA: vertical sync period
		STATUS_MGA_VIDEO  = 0x08, // MGA: video dot
		STATUS_NOVSYNC    = 0x80  // vertical sync period when clear
	};

	enum : uint8_t
	{
		PEN_MGA_BLACK = 16,
		PEN_MGA_NORMAL,
		PEN_MGA_BRIGHT
	};

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_RECONFIGURE(crtc_reconfigure);
	void crtc_hsync_w(int state);
	void crtc_vsync_w(int state);

	void cga_text_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x);
	void cga_gfx_2bpp_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void cga_gfx_1bpp_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void mga_text_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x);
	void mga_gfx_row(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);

	uint8_t chargen_r(uint8_t chr, uint8_t ra) const;
	uint8_t status_r();
	void mode_w(uint8_t data);
	void color_w(uint8_t data);
	void config_w(uint8_t data);
	void update_crtc_clock();
	void update_palette_lut();
	bool mga_graphics() const { return m_mga && (m_mode & MODE_GRAPHICS) && (m_config & CONFIG_MGA_GFX); }
	bool mga_page1() const { return m_mga && (m_mode & MODE_PAGE1) && (m_config & CONFIG_MGA_PAGE1); }

	required_device<mc6845_device> m_crtc;
	optional_region_ptr<uint8_t> m_chargen;
	memory_share_creator<uint8_t> m_vram;

	devcb_write_line m_hsync_cb;
	devcb_write_line m_vsync_cb;
	rgbi_delegate m_rgbi_cb;

	// configuration
	uint32_t m_mga_clock;
	bool m_strap_mga;
	bool m_strap_change_enable;

	// internal state
	bool m_mga;             // current display mode
	uint8_t m_mode;
	uint8_t m_color;
	uint8_t m_config;
	bool m_lpen_latched;
	uint8_t m_video_dot;    // fake MGA video dot stream
	uint8_t m_framecnt;     // for cursor and character blinking
	int m_vsync_on_pos;
	uint8_t m_palette_lut_2bpp[4];
};

DECLARE_DEVICE_TYPE(UM487F, um487f_device)

#endif // MAME_VIDEO_UM487F_H
