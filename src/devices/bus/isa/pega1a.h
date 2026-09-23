// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    WD Paradise PEGA1A

    Single chip multimode EGA controller: IBM EGA plus CGA, MDA,
    Hercules and Plantronics COLORPLUS emulation modes, selected
    through a block of extended registers (3DB/3BB, 3BF, 3DD, 3DF).

    Abstract base for boards built around the chip; the boot ROM, the
    I/O decode and the configuration switches are the subclass's.

**********************************************************************/

#ifndef MAME_BUS_ISA_PEGA1A_H
#define MAME_BUS_ISA_PEGA1A_H

#pragma once

#include "ega.h"

#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pega1a_device

class pega1a_device : public isa8_ega_device
{
public:
	uint8_t pega_3b0_r(offs_t offset);
	void pega_3b0_w(offs_t offset, uint8_t data);
	uint8_t pega_3d0_r(offs_t offset);
	void pega_3d0_w(offs_t offset, uint8_t data);
	void pega_3c0_w(offs_t offset, uint8_t data);

protected:
	pega1a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// the board supplies the switches, so the inherited generic-EGA "config"
	// port would only show up as a mislabeled duplicate
	virtual ioport_constructor device_input_ports() const override ATTR_COLD { return nullptr; }

	virtual void install_banks() override;

	virtual uint8_t *font_base() const override { return m_plane[BIT(ext_mode(), 3) ? 3 : 2]; }

	void set_monitor_palette(bool mono);

private:
	// write-only; a read returns the floating bus
	void ext_mode_color_w(uint8_t data);  // 3DB - Extended Mode Control, colour range
	void ext_mode_mono_w(uint8_t data);   // 3BB - Extended Mode Control, mono range
	void herc_control_w(uint8_t data);    // 3BF - Hercules Control
	void plantronics_w(uint8_t data);     // 3DD - Plantronics Control
	void ext_3df_w(uint8_t data);         // 3DF - AutoSwitch Control
	void ext_mode_w(uint8_t old_mode);
	void mode_control_w(uint8_t data);      // 3D8 - CGA Mode Control
	void color_select_w(uint8_t data);      // 3D9 - CGA Color Select
	void mode_control_mono_w(uint8_t data); // 3B8 - MDA/Hercules Mode Control

	void autoswitch_mono();

	// bit 6 gates every emulation mode: PEGA.EXE writes 3DB <- 0x64 for CGA and
	// 3BB <- 0x44 for MDA or Hercules, with the manual's bit 0 clear in both
	bool cga_emulation() const { return BIT(m_ext_mode_color, 6); }
	bool mono_emulation() const { return BIT(m_ext_mode_mono, 6); }

	uint8_t ext_mode() const { return (mono_emulation() || !BIT(m_misc_output, 0)) ? m_ext_mode_mono : m_ext_mode_color; }

	bool border_blanked() const { return BIT(ext_mode(), 7); }
	bool palette_locked() const { return BIT(ext_mode(), 5); }
	bool crtc_timing_locked() const { return BIT(ext_mode(), 4); }
	bool blanking_disabled() const { return BIT(ext_mode(), 2); }

	// mode_control_mono_w() has already applied 3BF's mask
	bool herc_gfx_mode() const { return BIT(m_mode_control_mono, 1); }

	// the boot ROM leaves 3DF at 0x10 and every "Set ... Mode" entry in
	// PEGA.EXE leaves it at 0x40, so bit 6 is the AutoSwitch lock
	bool autoswitch_locked() const { return BIT(m_ext_3df, 6); }

	// in text modes the same bit is the blink enable
	bool cga_hires_gfx() const { return BIT(m_mode_control, 1) && BIT(m_mode_control, 4); }

	uint8_t mono_pen(int i) const;
	rgb_t border_pen() const;
	void set_palette_luts();
	int color_hpixels_per_column() const;
	void update_mode_timing();
	void update_mono_timing();
	void update_color_timing();

	uint8_t cga_status_r();
	uint8_t mono_status_r();
	void emu_hsync_changed(int state) { m_emu_hsync = state; }
	void emu_vsync_changed(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	CRTC_EGA_RECONFIGURE(ega_reconfigure);
	CRTC_EGA_PIXEL_UPDATE(ega_update_row);
	void ega_plantronics_gfx_4bpp(bitmap_ind16 &bitmap, uint16_t ma, uint16_t y, uint8_t x);
	void ega_plantronics_gfx_2bpp(bitmap_ind16 &bitmap, uint16_t ma, uint16_t y, uint8_t x);

	MC6845_RECONFIGURE(emu_reconfigure);
	MC6845_BEGIN_UPDATE(emu_begin_update);
	MC6845_UPDATE_ROW(emu_update_row);
	void cga_text(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x);
	void cga_gfx_1bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void cga_gfx_2bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void plantronics_gfx_4bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void plantronics_gfx_2bpp(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);
	void mda_text(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count, int8_t cursor_x);
	void herc_gfx(bitmap_rgb32 &bitmap, uint16_t ma, uint8_t ra, uint16_t y, uint8_t x_count);

	// a CRTC hands over the active display as its visible area; the screen is
	// configured with the border around it instead, so each timing also carries
	// where the active rectangle sits inside that larger frame
	struct screen_timing
	{
		int width = 0;
		int height = 0;
		rectangle visarea;
		rectangle active;
		attotime frame_period;
		int x_off = 0;
		int y_off = 0;
	};

	screen_timing bordered_timing(int width, int height, const rectangle &active, attotime frame_period,
			int hsync_on, int hsync_off, int vsync_on, int vsync_off) const;

	required_device<mc6845_device> m_crtc_emu;

	// the EGA core renders indexed and the MC6845 direct-colour, and a screen
	// can only have one: the screen is direct-colour and the EGA path is
	// composited through this and converted
	bitmap_ind16 m_ega_bitmap;

	screen_timing m_ega_timing;
	screen_timing m_emu_timing;

	uint8_t m_ext_mode_color;
	uint8_t m_ext_mode_mono;
	uint8_t m_herc_control;
	uint8_t m_plantronics;
	uint8_t m_ext_3df;
	bool m_mono_monitor;
	uint8_t m_mode_control;
	uint8_t m_color_select;
	uint8_t m_mode_control_mono;
	uint8_t m_crtc_index;

	uint8_t m_palette_lut_2bpp[4];
	int m_emu_x_off;
	uint8_t m_emu_frame_cnt;
	int m_emu_hsync;
	int m_emu_vsync;
};


#endif // MAME_BUS_ISA_PEGA1A_H
