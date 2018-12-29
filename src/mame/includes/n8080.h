// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_INCLUDES_N8080_H
#define MAME_INCLUDES_N8080_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/sn76477.h"
#include "emupal.h"
#include "screen.h"

class n8080_state : public driver_device
{
public:
	n8080_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_n8080_dac(*this, "n8080_dac"),
		m_helifire_dac(*this, "helifire_dac"),
		m_sn(*this, "snsnd"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void sheriff(machine_config &config);
	void sheriff_sound(machine_config &config);
	void westgun2(machine_config &config);
	void helifire(machine_config &config);
	void helifire_sound(machine_config &config);
	void spacefev(machine_config &config);
	void spacefev_sound(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;      // for helifire

	/* video-related */
	emu_timer* m_cannon_timer;
	int m_spacefev_red_screen;
	int m_spacefev_red_cannon;
	int m_sheriff_color_mode;
	int m_sheriff_color_data;
	int m_helifire_flash;
	uint8_t m_helifire_LSFR[63];
	unsigned m_helifire_mv;
	unsigned m_helifire_sc; /* IC56 */

	/* sound-related */
	int m_n8080_hardware;
	emu_timer* m_sound_timer[3];
	int m_helifire_dac_phase;
	double m_helifire_dac_volume;
	double m_helifire_dac_timing;
	uint16_t m_prev_sound_pins;
	uint16_t m_curr_sound_pins;
	int m_mono_flop[3];
	uint8_t m_prev_snd_data;

	/* other */
	unsigned m_shift_data;
	unsigned m_shift_bits;
	int m_inte;

	/* devices */
	required_device<i8080_cpu_device> m_maincpu;
	required_device<i8035_device> m_audiocpu;
	optional_device<dac_bit_interface> m_n8080_dac;
	optional_device<dac_8bit_r2r_device> m_helifire_dac;
	optional_device<sn76477_device> m_sn;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(n8080_shift_bits_w);
	DECLARE_WRITE8_MEMBER(n8080_shift_data_w);
	DECLARE_READ8_MEMBER(n8080_shift_r);
	DECLARE_WRITE8_MEMBER(n8080_video_control_w);
	DECLARE_WRITE8_MEMBER(n8080_sound_1_w);
	DECLARE_WRITE8_MEMBER(n8080_sound_2_w);
	DECLARE_READ8_MEMBER(n8080_8035_p1_r);
	DECLARE_READ_LINE_MEMBER(n8080_8035_t0_r);
	DECLARE_READ_LINE_MEMBER(n8080_8035_t1_r);
	DECLARE_READ_LINE_MEMBER(helifire_8035_t0_r);
	DECLARE_READ_LINE_MEMBER(helifire_8035_t1_r);
	DECLARE_READ8_MEMBER(helifire_8035_external_ram_r);
	DECLARE_READ8_MEMBER(helifire_8035_p2_r);
	DECLARE_WRITE8_MEMBER(n8080_dac_w);
	DECLARE_WRITE8_MEMBER(helifire_sound_ctrl_w);
	DECLARE_WRITE_LINE_MEMBER(n8080_inte_callback);
	DECLARE_WRITE8_MEMBER(n8080_status_callback);
	DECLARE_MACHINE_RESET(spacefev);
	DECLARE_VIDEO_START(spacefev);
	void n8080_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(sheriff);
	DECLARE_VIDEO_START(sheriff);
	DECLARE_MACHINE_RESET(helifire);
	DECLARE_VIDEO_START(helifire);
	void helifire_palette(palette_device &palette) const;
	DECLARE_SOUND_START(spacefev);
	DECLARE_SOUND_RESET(spacefev);
	DECLARE_SOUND_START(sheriff);
	DECLARE_SOUND_RESET(sheriff);
	DECLARE_SOUND_START(helifire);
	DECLARE_SOUND_RESET(helifire);
	DECLARE_MACHINE_START(n8080);
	DECLARE_MACHINE_RESET(n8080);
	uint32_t screen_update_spacefev(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sheriff(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_helifire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_helifire);
	TIMER_CALLBACK_MEMBER(spacefev_stop_red_cannon);
	TIMER_DEVICE_CALLBACK_MEMBER(rst1_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(rst2_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(spacefev_vco_voltage_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(helifire_dac_volume_timer);
	void spacefev_start_red_cannon(  );
	void helifire_next_line(  );
	void spacefev_update_SN76477_status();
	void sheriff_update_SN76477_status();
	void update_SN76477_status();
	void start_mono_flop( int n, const attotime &expire );
	void stop_mono_flop( int n );
	TIMER_CALLBACK_MEMBER( stop_mono_flop_callback );
	void spacefev_sound_pins_changed();
	void sheriff_sound_pins_changed();
	void helifire_sound_pins_changed();
	void sound_pins_changed();
	void delayed_sound_1( int data );
	TIMER_CALLBACK_MEMBER( delayed_sound_1_callback );
	void delayed_sound_2( int data );
	TIMER_CALLBACK_MEMBER( delayed_sound_2_callback );

	void helifire_main_cpu_map(address_map &map);
	void helifire_sound_io_map(address_map &map);
	void main_cpu_map(address_map &map);
	void main_io_map(address_map &map);
	void n8080_sound_cpu_map(address_map &map);
};

#endif // MAME_INCLUDES_N8080_H
