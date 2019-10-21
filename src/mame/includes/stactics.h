// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/
#ifndef MAME_INCLUDES_STACTICS_H
#define MAME_INCLUDES_STACTICS_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"

class stactics_state : public driver_device
{
public:
	stactics_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_display_buffer(*this, "display_buffer"),
		m_videoram_b(*this, "videoram_b"),
		m_videoram_d(*this, "videoram_d"),
		m_videoram_e(*this, "videoram_e"),
		m_videoram_f(*this, "videoram_f"),
		m_base_lamps(*this, "base_lamp%u", 0U),
		m_beam_leds_left(*this, "beam_led_left%u", 0U),
		m_beam_leds_right(*this, "beam_led_right%u", 0U),
		m_score_digits(*this, "digit%u", 0U),
		m_credit_leds(*this, "credit_led%u", 0U),
		m_barrier_leds(*this, "barrier_led%u", 0U),
		m_round_leds(*this, "round_led%u", 0U),
		m_barrier_lamp(*this, "barrier_lamp"),
		m_start_lamp(*this, "start_lamp"),
		m_sight_led(*this, "sight_led"),
		m_in3(*this, "IN3"),
		m_fake(*this, "FAKE")
	{ }

	void stactics(machine_config &config);

	DECLARE_READ_LINE_MEMBER(frame_count_d3_r);
	DECLARE_READ_LINE_MEMBER(shot_standby_r);
	DECLARE_READ_LINE_MEMBER(not_shot_arrive_r);
	DECLARE_READ_LINE_MEMBER(motor_not_ready_r);
	DECLARE_CUSTOM_INPUT_MEMBER(get_rng);

private:
	DECLARE_READ8_MEMBER(vert_pos_r);
	DECLARE_READ8_MEMBER(horiz_pos_r);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_2_w);
	DECLARE_WRITE_LINE_MEMBER(palette_bank_w);
	DECLARE_WRITE8_MEMBER(scroll_ram_w);
	DECLARE_WRITE8_MEMBER(speed_latch_w);
	DECLARE_WRITE8_MEMBER(shot_trigger_w);
	DECLARE_WRITE8_MEMBER(shot_flag_clear_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);

	INTERRUPT_GEN_MEMBER(interrupt);

	DECLARE_WRITE_LINE_MEMBER(barrier_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(start_lamp_w);
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(base_lamp_w) { m_base_lamps[N] = state; }
	DECLARE_WRITE_LINE_MEMBER(base_2_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(base_3_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(base_4_lamp_w);
	DECLARE_WRITE_LINE_MEMBER(base_5_lamp_w);

	virtual void machine_start() override;
	virtual void video_start() override;
	void stactics_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_beam();
	inline int get_pixel_on_plane(uint8_t *videoram, uint8_t y, uint8_t x, uint8_t y_scroll);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <unsigned N> void set_indicator_leds(unsigned offset, output_finder<N> &outputs, int base_index);
	void update_artwork();
	void move_motor();

	void stactics_video(machine_config &config);
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_outlatch;

	required_shared_ptr<uint8_t> m_display_buffer;
	required_shared_ptr<uint8_t> m_videoram_b;
	required_shared_ptr<uint8_t> m_videoram_d;
	required_shared_ptr<uint8_t> m_videoram_e;
	required_shared_ptr<uint8_t> m_videoram_f;

	output_finder<5> m_base_lamps;
	output_finder<0x40> m_beam_leds_left;
	output_finder<0x40> m_beam_leds_right;
	output_finder<6> m_score_digits;
	output_finder<8> m_credit_leds;
	output_finder<12> m_barrier_leds;
	output_finder<16> m_round_leds;
	output_finder<> m_barrier_lamp;
	output_finder<> m_start_lamp;
	output_finder<> m_sight_led;

	required_ioport m_in3;
	required_ioport m_fake;

	/* machine state */
	int    m_vert_pos;
	int    m_horiz_pos;
	bool   m_motor_on;

	/* video state */
	uint8_t  m_y_scroll_d;
	uint8_t  m_y_scroll_e;
	uint8_t  m_y_scroll_f;
	uint8_t  m_frame_count;
	uint8_t  m_shot_standby;
	uint8_t  m_shot_arrive;
	uint16_t m_beam_state;
	uint16_t m_old_beam_state;
	uint16_t m_beam_states_per_frame;
	uint8_t  m_palette_bank;
};

#endif // MAME_INCLUDES_STACTICS_H
