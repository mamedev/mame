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
		m_bitmapram(*this, "bitmapram"),
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
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<vsystem_gga_device> m_gga;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_bitmapram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint8_t m_upd_rom_bank;
	int m_sprite_palette;
	int m_sprite_xoffs;
	uint16_t m_videoflags;
	uint8_t m_bins;
	uint8_t m_gins;
	tilemap_t *m_background[2];
	emu_timer *m_crtc_timer;

	DECLARE_READ16_MEMBER(sound_busy_r);
	DECLARE_WRITE16_MEMBER(rpunch_videoram_w);
	DECLARE_WRITE16_MEMBER(rpunch_videoreg_w);
	DECLARE_WRITE16_MEMBER(rpunch_scrollreg_w);
	DECLARE_WRITE8_MEMBER(rpunch_gga_w);
	DECLARE_WRITE8_MEMBER(rpunch_gga_data_w);
	DECLARE_WRITE16_MEMBER(rpunch_ins_w);
	DECLARE_WRITE8_MEMBER(upd_control_w);
	DECLARE_WRITE8_MEMBER(upd_data_w);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);

	DECLARE_VIDEO_START(rpunch);
	DECLARE_VIDEO_START(svolley);

	uint32_t screen_update_rpunch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(crtc_interrupt_gen);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int stop);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_RPUNCH_H
