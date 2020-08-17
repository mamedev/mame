// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#ifndef MAME_INCLUDES_SSPEEDR_H
#define MAME_INCLUDES_SSPEEDR_H

#pragma once

#include "emupal.h"
#include "machine/netlist.h"

class sspeedr_state : public driver_device
{
public:
	sspeedr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_digits(*this, "digit%u", 0U)
		, m_pedal_bit0(*this, "sound_nl:pedal_bit0")
		, m_pedal_bit1(*this, "sound_nl:pedal_bit1")
		, m_pedal_bit2(*this, "sound_nl:pedal_bit2")
		, m_pedal_bit3(*this, "sound_nl:pedal_bit3")
		, m_hi_shift(*this, "sound_nl:hi_shift")
		, m_lo_shift(*this, "sound_nl:lo_shift")
		, m_boom(*this, "sound_nl:boom")
		, m_engine_sound_off(*this, "sound_nl:engine_sound_off")
		, m_noise_cr_1(*this, "sound_nl:noise_cr_1")
		, m_noise_cr_2(*this, "sound_nl:noise_cr_2")
		, m_silence(*this, "sound_nl:silence")
	{ }

	void sspeedr(machine_config &config);

private:
	void sspeedr_int_ack_w(uint8_t data);
	void sspeedr_lamp_w(uint8_t data);
	void sspeedr_time_w(offs_t offset, uint8_t data);
	void sspeedr_score_w(offs_t offset, uint8_t data);
	void sspeedr_sound1_w(uint8_t data);
	void sspeedr_sound2_w(uint8_t data);
	void sspeedr_driver_horz_w(uint8_t data);
	void sspeedr_driver_horz_2_w(uint8_t data);
	void sspeedr_driver_vert_w(uint8_t data);
	void sspeedr_driver_pic_w(uint8_t data);
	void sspeedr_drones_horz_w(uint8_t data);
	void sspeedr_drones_horz_2_w(uint8_t data);
	void sspeedr_drones_mask_w(uint8_t data);
	void sspeedr_drones_vert_w(offs_t offset, uint8_t data);
	void sspeedr_track_horz_w(uint8_t data);
	void sspeedr_track_horz_2_w(uint8_t data);
	void sspeedr_track_vert_w(offs_t offset, uint8_t data);
	void sspeedr_track_ice_w(uint8_t data);
	void sspeedr_palette(palette_device &palette) const;
	uint32_t screen_update_sspeedr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_sspeedr);
	void sspeedr_io_map(address_map &map);
	void sspeedr_map(address_map &map);

	uint8_t m_led_TIME[2];
	uint8_t m_led_SCORE[24];
	int m_toggle;
	unsigned m_driver_horz;
	unsigned m_driver_vert;
	unsigned m_driver_pic;
	unsigned m_drones_horz;
	unsigned m_drones_vert[3];
	unsigned m_drones_mask;
	unsigned m_track_horz;
	unsigned m_track_vert[2];
	unsigned m_track_ice;
	virtual void video_start() override;
	virtual void machine_start() override { m_digits.resolve(); }
	void draw_track(bitmap_ind16 &bitmap);
	void draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<26> m_digits;
	required_device<netlist_mame_logic_input_device> m_pedal_bit0;
	required_device<netlist_mame_logic_input_device> m_pedal_bit1;
	required_device<netlist_mame_logic_input_device> m_pedal_bit2;
	required_device<netlist_mame_logic_input_device> m_pedal_bit3;
	required_device<netlist_mame_logic_input_device> m_hi_shift;
	required_device<netlist_mame_logic_input_device> m_lo_shift;
	required_device<netlist_mame_logic_input_device> m_boom;
	required_device<netlist_mame_logic_input_device> m_engine_sound_off;
	required_device<netlist_mame_logic_input_device> m_noise_cr_1;
	required_device<netlist_mame_logic_input_device> m_noise_cr_2;
	required_device<netlist_mame_logic_input_device> m_silence;
};

#endif // MAME_INCLUDES_SSPEEDR_H
