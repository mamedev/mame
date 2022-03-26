// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
#ifndef MAME_INCLUDES_TEHKANWC_H
#define MAME_INCLUDES_TEHKANWC_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class tehkanwc_state : public driver_device
{
public:
	tehkanwc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void tehkanwcb(machine_config &config);
	void tehkanwc(machine_config &config);

	void init_teedoff();

protected:
	enum
	{
		TIMER_RESET
	};

	virtual void machine_start() override;
	virtual void video_start() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	output_finder<2> m_digits;

	int m_track0[2]{};
	int m_track1[2]{};
	int m_msm_data_offs = 0;
	int m_toggle = 0;
	uint8_t m_scroll_x[2]{};
	uint8_t m_led0 = 0;
	uint8_t m_led1 = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	emu_timer *m_reset_timer = nullptr;

	void sub_cpu_halt_w(uint8_t data);
	uint8_t track_0_r(offs_t offset);
	uint8_t track_1_r(offs_t offset);
	void track_0_reset_w(offs_t offset, uint8_t data);
	void track_1_reset_w(offs_t offset, uint8_t data);
	void sound_command_w(uint8_t data);
	void sound_answer_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void scroll_x_w(offs_t offset, uint8_t data);
	void scroll_y_w(uint8_t data);
	void flipscreen_x_w(uint8_t data);
	void flipscreen_y_w(uint8_t data);
	void gridiron_led0_w(uint8_t data);
	void gridiron_led1_w(uint8_t data);
	uint8_t portA_r();
	uint8_t portB_r();
	void portA_w(uint8_t data);
	void portB_w(uint8_t data);
	void msm_reset_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gridiron_draw_led(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t led,int player);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_mem(address_map &map);
	void sound_mem(address_map &map);
	void sound_port(address_map &map);
	void sub_mem(address_map &map);
};

#endif // MAME_INCLUDES_TEHKANWC_H
