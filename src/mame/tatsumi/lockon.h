// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    Lock-On hardware

*************************************************************************/
#ifndef MAME_TATSUMI_LOCKON_H
#define MAME_TATSUMI_LOCKON_H

#pragma once

#include "machine/watchdog.h"
#include "sound/flt_vol.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

/* Calculated from CRT controller writes */
#define PIXEL_CLOCK            (XTAL(21'000'000) / 3)
#define FRAMEBUFFER_CLOCK      XTAL(10'000'000)
#define HBSTART                320
#define HBEND                  0
#define HTOTAL                 448
#define VBSTART                240
#define VBEND                  0
#define VTOTAL                 280


class lockon_state : public driver_device
{
public:
	lockon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_char_ram(*this, "char_ram")
		, m_hud_ram(*this, "hud_ram")
		, m_scene_ram(*this, "scene_ram")
		, m_ground_ram(*this, "ground_ram")
		, m_object_ram(*this, "object_ram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ground(*this, "ground")
		, m_object(*this, "object")
		, m_watchdog(*this, "watchdog")
		, m_f2203_1l(*this, "f2203.1l")
		, m_f2203_2l(*this, "f2203.2l")
		, m_f2203_3l(*this, "f2203.3l")
		, m_f2203_1r(*this, "f2203.1r")
		, m_f2203_2r(*this, "f2203.2r")
		, m_f2203_3r(*this, "f2203.3r")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_lamp(*this, "lamp1")
	{ }

	void lockon(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_char_ram;
	required_shared_ptr<uint16_t> m_hud_ram;
	required_shared_ptr<uint16_t> m_scene_ram;
	required_shared_ptr<uint16_t> m_ground_ram;
	required_shared_ptr<uint16_t> m_object_ram;

	/* video-related */
	tilemap_t       *m_tilemap = nullptr;
	uint8_t           m_ground_ctrl = 0U;
	uint16_t          m_scroll_h = 0U;
	uint16_t          m_scroll_v = 0U;
	std::unique_ptr<bitmap_ind16> m_front_buffer{};
	std::unique_ptr<bitmap_ind16> m_back_buffer{};
	emu_timer       *m_bufend_timer = nullptr;
	emu_timer       *m_cursor_timer = nullptr;

	/* Rotation Control */
	uint16_t      m_xsal = 0U;
	uint16_t      m_x0ll = 0U;
	uint16_t      m_dx0ll = 0U;
	uint16_t      m_dxll = 0U;
	uint16_t      m_ysal = 0U;
	uint16_t      m_y0ll = 0U;
	uint16_t      m_dy0ll = 0U;
	uint16_t      m_dyll = 0U;

	/* Object palette RAM control */
	uint32_t      m_iden = 0U;
	std::unique_ptr<uint8_t[]>       m_obj_pal_ram{};
	uint32_t      m_obj_pal_latch = 0U;
	uint32_t      m_obj_pal_addr = 0U;

	/* misc */
	uint8_t       m_ctrl_reg = 0U;
	uint32_t      m_main_inten = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_ground;
	required_device<cpu_device> m_object;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<filter_volume_device> m_f2203_1l;
	required_device<filter_volume_device> m_f2203_2l;
	required_device<filter_volume_device> m_f2203_3l;
	required_device<filter_volume_device> m_f2203_1r;
	required_device<filter_volume_device> m_f2203_2r;
	required_device<filter_volume_device> m_f2203_3r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<> m_lamp;

	uint16_t lockon_crtc_r();
	void lockon_crtc_w(offs_t offset, uint16_t data);
	void lockon_char_w(offs_t offset, uint16_t data);
	void lockon_scene_h_scr_w(uint16_t data);
	void lockon_scene_v_scr_w(uint16_t data);
	void lockon_ground_ctrl_w(uint16_t data);
	void lockon_tza112_w(offs_t offset, uint16_t data);
	uint16_t lockon_obj_4000_r();
	void lockon_obj_4000_w(uint16_t data);
	void lockon_fb_clut_w(offs_t offset, uint16_t data);
	void lockon_rotate_w(offs_t offset, uint16_t data);
	void adrst_w(uint16_t data);
	uint16_t main_gnd_r(offs_t offset);
	void main_gnd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t main_obj_r(offs_t offset);
	void main_obj_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tst_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t main_z80_r(offs_t offset);
	void main_z80_w(offs_t offset, uint16_t data);
	void inten_w(uint16_t data);
	void emres_w(uint16_t data);
	void sound_vol(uint8_t data);
	void ym2203_out_b(uint8_t data);
	TILE_GET_INFO_MEMBER(get_lockon_tile_info);
	void lockon_palette(palette_device &palette) const;
	uint32_t screen_update_lockon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_lockon(int state);
	TIMER_CALLBACK_MEMBER(cursor_callback);
	TIMER_CALLBACK_MEMBER(bufend_callback);
	void scene_draw();
	void ground_draw();
	void objects_draw();
	void rotate_draw( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void hud_draw( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ym2203_irq(int state);
	void ground_v30(address_map &map) ATTR_COLD;
	void main_v30(address_map &map) ATTR_COLD;
	void object_v30(address_map &map) ATTR_COLD;
	void sound_io(address_map &map) ATTR_COLD;
	void sound_prg(address_map &map) ATTR_COLD;
};

#endif // MAME_TATSUMI_LOCKON_H
