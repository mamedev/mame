// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Alesis HR-16 and SR-16 drum machines

****************************************************************************/

#ifndef MAME_ALESIS_ALESIS_H
#define MAME_ALESIS_ALESIS_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/hd44780.h"
#include "imagedev/cassette.h"
#include "emupal.h"


// ======================> alesis_dm3ag_device

class alesis_dm3ag_device : public device_t
{
public:
	// construction/destruction
	alesis_dm3ag_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device interface
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(dac_update);

private:
	required_device<dac_word_interface> m_dac;
	required_region_ptr<int8_t> m_samples;

	emu_timer * m_dac_update_timer;
	bool        m_output_active;
	int         m_count;
	int         m_shift;
	uint32_t    m_cur_sample;
	uint8_t     m_cmd[5]{};
};


// ======================> alesis_state

class alesis_state : public driver_device
{
public:
	alesis_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_lcdc(*this, "hd44780"),
		m_cassette(*this, "cassette"),
		m_maincpu(*this, "maincpu"),
		m_col(*this, "COL%u", 1U),
		m_select(*this, "SELECT"),
		m_digit(*this, "digit%u", 0U),
		m_pattern(*this, "pattern"),
		m_track_led(*this, "track_led%u", 1U),
		m_patt_led(*this, "patt_led"),
		m_song_led(*this, "song_led"),
		m_play_led(*this, "play_led"),
		m_record_led(*this, "record_led"),
		m_voice_led(*this, "voice_led"),
		m_tune_led(*this, "tune_led"),
		m_mix_led(*this, "mix_led"),
		m_tempo_led(*this, "tempo_led"),
		m_midi_led(*this, "midi_led"),
		m_part_led(*this, "part_led"),
		m_edit_led(*this, "edit_led"),
		m_echo_led(*this, "echo_led"),
		m_loop_led(*this, "loop_led"),
		m_a_next(*this, "a_next"),
		m_b_next(*this, "b_next"),
		m_fill_next(*this, "fill_next"),
		m_user_next(*this, "user_next"),
		m_play(*this, "play"),
		m_record(*this, "record"),
		m_compose(*this, "compose"),
		m_perform(*this, "perform"),
		m_song(*this, "song"),
		m_b(*this, "b"),
		m_a(*this, "a"),
		m_fill(*this, "fill"),
		m_user(*this, "user"),
		m_edited(*this, "edited"),
		m_set(*this, "set"),
		m_drum(*this, "drum"),
		m_press_play(*this, "press_play"),
		m_metronome(*this, "metronome"),
		m_tempo(*this, "tempo"),
		m_page(*this, "page"),
		m_step_edit(*this, "step_edit"),
		m_swing_off(*this, "swing_off"),
		m_swing_62(*this, "swing_62"),
		m_click_l1(*this, "click_l1"),
		m_click_note(*this, "click_note"),
		m_click_l2(*this, "click_l2"),
		m_click_3(*this, "click_3"),
		m_backup(*this, "backup"),
		m_drum_set(*this, "drum_set"),
		m_swing(*this, "swing"),
		m_swing_58(*this, "swing_58"),
		m_click_off(*this, "click_off"),
		m_click(*this, "click"),
		m_quantize_off(*this, "quantize_off"),
		m_quantize_3(*this, "quantize_3"),
		m_midi_setup(*this, "midi_setup"),
		m_record_setup(*this, "record_setup"),
		m_quantize(*this, "quantize"),
		m_swing_54(*this, "swing_54"),
		m_quantize_l1(*this, "quantize_l1"),
		m_quantize_l2(*this, "quantize_l2"),
		m_quantize_l3(*this, "quantize_l3"),
		m_quantize_note(*this, "quantize_note"),
		m_setup(*this, "setup")
	{ }

	void init_hr16();
	void mmt8(machine_config &config);
	void hr16(machine_config &config);
	void sr16(machine_config &config);

protected:
	void alesis_palette(palette_device &palette) const;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void update_lcd_symbols(bitmap_ind16 &bitmap, uint8_t pos, uint8_t y, uint8_t x, int state);
	void led_w(uint8_t data);
	void mmt8_led_w(uint8_t data);
	uint8_t mmt8_led_r();
	void track_led_w(uint8_t data);
	void kb_matrix_w(uint8_t data);
	uint8_t kb_r();
	uint8_t p3_r();
	void p3_w(uint8_t data);
	uint8_t mmt8_p3_r();
	void mmt8_p3_w(uint8_t data);
	void sr16_lcd_w(uint8_t data);
	HD44780_PIXEL_UPDATE(sr16_pixel_update);

	void hr16_io(address_map &map) ATTR_COLD;
	void hr16_mem(address_map &map) ATTR_COLD;
	void mmt8_io(address_map &map) ATTR_COLD;
	void sr16_io(address_map &map) ATTR_COLD;
	void sr16_mem(address_map &map) ATTR_COLD;

private:
	uint8_t       m_kb_matrix = 0;
	uint8_t       m_leds = 0;
	uint8_t       m_lcd_digits[5]{};

	required_device<hd44780_device> m_lcdc;
	optional_device<cassette_image_device> m_cassette;
	required_device<mcs51_cpu_device> m_maincpu;

	required_ioport_array<6> m_col;
	optional_ioport m_select;
	output_finder<5> m_digit;
	output_finder<> m_pattern;
	output_finder<8> m_track_led;
	output_finder<> m_patt_led;
	output_finder<> m_song_led;
	output_finder<> m_play_led;
	output_finder<> m_record_led;
	output_finder<> m_voice_led;
	output_finder<> m_tune_led;
	output_finder<> m_mix_led;
	output_finder<> m_tempo_led;
	output_finder<> m_midi_led;
	output_finder<> m_part_led;
	output_finder<> m_edit_led;
	output_finder<> m_echo_led;
	output_finder<> m_loop_led;
	output_finder<> m_a_next;
	output_finder<> m_b_next;
	output_finder<> m_fill_next;
	output_finder<> m_user_next;
	output_finder<> m_play;
	output_finder<> m_record;
	output_finder<> m_compose;
	output_finder<> m_perform;
	output_finder<> m_song;
	output_finder<> m_b;
	output_finder<> m_a;
	output_finder<> m_fill;
	output_finder<> m_user;
	output_finder<> m_edited;
	output_finder<> m_set;
	output_finder<> m_drum;
	output_finder<> m_press_play;
	output_finder<> m_metronome;
	output_finder<> m_tempo;
	output_finder<> m_page;
	output_finder<> m_step_edit;
	output_finder<> m_swing_off;
	output_finder<> m_swing_62;
	output_finder<> m_click_l1;
	output_finder<> m_click_note;
	output_finder<> m_click_l2;
	output_finder<> m_click_3;
	output_finder<> m_backup;
	output_finder<> m_drum_set;
	output_finder<> m_swing;
	output_finder<> m_swing_58;
	output_finder<> m_click_off;
	output_finder<> m_click;
	output_finder<> m_quantize_off;
	output_finder<> m_quantize_3;
	output_finder<> m_midi_setup;
	output_finder<> m_record_setup;
	output_finder<> m_quantize;
	output_finder<> m_swing_54;
	output_finder<> m_quantize_l1;
	output_finder<> m_quantize_l2;
	output_finder<> m_quantize_l3;
	output_finder<> m_quantize_note;
	output_finder<> m_setup;
};

// device type definition
DECLARE_DEVICE_TYPE(ALESIS_DM3AG, alesis_dm3ag_device)

#endif  // MAME_ALESIS_ALESIS_H
