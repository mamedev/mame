// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
#ifndef MAME_MIDWAY_SSPEEDR_H
#define MAME_MIDWAY_SSPEEDR_H

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
		, m_lampgo(*this, "lampGO")
		, m_lampep(*this, "lampEP")
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
		, m_track(*this, "track")
	{ }

	void sspeedr(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void int_ack_w(uint8_t data);
	void lamp_w(uint8_t data);
	void time_w(offs_t offset, uint8_t data);
	void score_w(offs_t offset, uint8_t data);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void driver_horz_w(uint8_t data);
	void driver_horz_2_w(uint8_t data);
	void driver_vert_w(uint8_t data);
	void driver_pic_w(uint8_t data);
	void drones_horz_w(uint8_t data);
	void drones_horz_2_w(uint8_t data);
	void drones_mask_w(uint8_t data);
	void drones_vert_w(offs_t offset, uint8_t data);
	void track_horz_w(uint8_t data);
	void track_horz_2_w(uint8_t data);
	void track_vert_w(offs_t offset, uint8_t data);
	void track_ice_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;

	uint8_t m_led_time[2]{};
	uint8_t m_led_score[24]{};
	uint8_t m_toggle = 0;
	uint16_t m_driver_horz = 0;
	uint8_t m_driver_vert = 0;
	uint8_t m_driver_pic = 0;
	uint16_t m_drones_horz = 0;
	uint8_t m_drones_vert[3]{};
	uint8_t m_drones_mask = 0;
	uint16_t m_track_horz = 0;
	uint8_t m_track_vert[2]{};
	uint8_t m_track_ice = 0;

	void draw_track(bitmap_ind16 &bitmap);
	void draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<26> m_digits;
	output_finder<> m_lampgo;
	output_finder<> m_lampep;
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
	required_region_ptr<uint8_t> m_track;
};

#endif // MAME_MIDWAY_SSPEEDR_H
