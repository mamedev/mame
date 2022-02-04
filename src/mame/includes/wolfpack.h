// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Wolf Pack (prototype) driver

***************************************************************************/
#ifndef MAME_INCLUDES_WOLFPACK_H
#define MAME_INCLUDES_WOLFPACK_H

#pragma once

#include "sound/s14001a.h"
#include "emupal.h"
#include "screen.h"

class wolfpack_state : public driver_device
{
public:
	wolfpack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_maincpu(*this, "maincpu"),
		m_s14001a(*this, "speech"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_led(*this, "led0")
	{ }

	void wolfpack(machine_config &config);

	template <int Bit> DECLARE_READ_LINE_MEMBER(dial_r);

private:
	enum
	{
		TIMER_PERIODIC
	};

	uint8_t misc_r();
	void high_explo_w(uint8_t data);
	void sonar_ping_w(uint8_t data);
	void sirlat_w(uint8_t data);
	void pt_sound_w(uint8_t data);
	void launch_torpedo_w(uint8_t data);
	void low_explo_w(uint8_t data);
	void screw_cont_w(uint8_t data);
	void lamp_flash_w(uint8_t data);
	void warning_light_w(uint8_t data);
	void audamp_w(uint8_t data);
	void attract_w(uint8_t data);
	void credit_w(uint8_t data);
	void coldetres_w(uint8_t data);
	void ship_size_w(uint8_t data);
	void video_invert_w(uint8_t data);
	void ship_reflect_w(uint8_t data);
	void pt_pos_select_w(uint8_t data);
	void pt_horz_w(uint8_t data);
	void pt_pic_w(uint8_t data);
	void ship_h_w(uint8_t data);
	void torpedo_pic_w(uint8_t data);
	void ship_h_precess_w(uint8_t data);
	void ship_pic_w(uint8_t data);
	void torpedo_h_w(uint8_t data);
	void torpedo_v_w(uint8_t data);
	void word_w(uint8_t data);
	void start_speech_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	void wolfpack_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_CALLBACK_MEMBER(periodic_callback);
	void draw_ship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_torpedo(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_pt(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_water(palette_device &palette, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	// devices, pointers
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_device<cpu_device> m_maincpu;
	required_device<s14001a_device> m_s14001a;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<> m_led;

	bool m_collision;
	unsigned m_current_index;
	uint8_t m_video_invert;
	uint8_t m_ship_reflect;
	uint8_t m_pt_pos_select;
	uint8_t m_pt_horz;
	uint8_t m_pt_pic;
	uint8_t m_ship_h;
	uint8_t m_torpedo_pic;
	uint8_t m_ship_size;
	uint8_t m_ship_h_precess;
	uint8_t m_ship_pic;
	uint8_t m_torpedo_h;
	uint8_t m_torpedo_v;
	std::unique_ptr<uint8_t[]> m_LFSR;
	bitmap_ind16 m_helper;
	emu_timer *m_periodic_timer;
};

#endif // MAME_INCLUDES_WOLFPACK_H
