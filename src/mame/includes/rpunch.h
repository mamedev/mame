// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_INCLUDES_RPUNCH_H
#define MAME_INCLUDES_RPUNCH_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/upd7759.h"
#include "video/vsystem_gga.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class rpunch_state : public driver_device
{
public:
	rpunch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gga(*this, "gga"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void svolley(machine_config &config);
	void rpunch(machine_config &config);
	void svolleybl(machine_config &config);

	void init_rabiolep();
	void init_svolley();

	DECLARE_CUSTOM_INPUT_MEMBER(hi_bits_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vsystem_gga_device> m_gga;

	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_spriteram;

	u8 m_upd_rom_bank = 0;
	int m_sprite_palette = 0;
	int m_sprite_xoffs = 0;
	u16 m_videoflags = 0;
	u8 m_sprite_pri = 0;
	u8 m_sprite_num = 0;
	tilemap_t *m_background[2]{};
	std::unique_ptr<bitmap_ind16> m_pixmap;
	emu_timer *m_crtc_timer = nullptr;

	u16 sound_busy_r();
	u8 pixmap_r(offs_t offset);
	void pixmap_w(offs_t offset, u8 data);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void videoreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void scrollreg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void gga_w(offs_t offset, u8 data);
	void gga_data_w(offs_t offset, u8 data);
	void sprite_ctrl_w(offs_t offset, u8 data);
	void upd_control_w(u8 data);
	void upd_data_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);

	DECLARE_VIDEO_START(rpunch);
	DECLARE_VIDEO_START(svolley);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(crtc_interrupt_gen);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_map(address_map &map);
	void rpunch_map(address_map &map);
	void svolley_map(address_map &map);
	void svolleybl_main_map(address_map &map);
	void svolleybl_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_RPUNCH_H
