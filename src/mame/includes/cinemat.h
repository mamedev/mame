// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/
#ifndef MAME_INCLUDES_CINEMAT_H
#define MAME_INCLUDES_CINEMAT_H

#pragma once

#include "cpu/ccpu/ccpu.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "video/vector.h"
#include "screen.h"

class cinemat_state : public driver_device
{
public:
	cinemat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay1(*this, "ay1")
		, m_outlatch(*this, "outlatch")
		, m_samples(*this, "samples")
		, m_vector(*this, "vector")
		, m_screen(*this, "screen")
		, m_rambase(*this, "rambase")
		, m_inputs(*this, "INPUTS")
		, m_switches(*this, "SWITCHES")
		, m_gear_input(*this, "GEAR")
		, m_wheel(*this, "WHEEL")
		, m_analog_x(*this, "ANALOGX")
		, m_analog_y(*this, "ANALOGY")
		, m_led(*this, "led")
		, m_pressed(*this, "pressed%u", 0U)
	{ }

	required_device<ccpu_cpu_device> m_maincpu;
	optional_device<ay8910_device> m_ay1;
	required_device<ls259_device> m_outlatch;
	optional_device<samples_device> m_samples;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint16_t> m_rambase;

	required_ioport m_inputs;
	required_ioport m_switches;
	optional_ioport m_gear_input;
	optional_ioport m_wheel;
	optional_ioport m_analog_x;
	optional_ioport m_analog_y;

	output_finder<> m_led;
	output_finder<10> m_pressed;

	uint32_t m_current_shift;
	uint32_t m_last_shift;
	uint32_t m_last_shift2;
	uint32_t m_current_pitch;
	uint32_t m_last_frame;
	uint8_t m_coin_detected;
	uint8_t m_coin_last_reset;
	uint8_t m_mux_select;
	int m_gear;
	rgb_t m_vector_color;
	int16_t m_lastx;
	int16_t m_lasty;
	DECLARE_READ8_MEMBER(inputs_r);
	DECLARE_READ8_MEMBER(switches_r);
	DECLARE_READ8_MEMBER(coin_input_r);
	WRITE_LINE_MEMBER(coin_reset_w);
	WRITE_LINE_MEMBER(mux_select_w);
	DECLARE_READ8_MEMBER(speedfrk_wheel_r);
	DECLARE_READ8_MEMBER(speedfrk_gear_r);
	virtual DECLARE_WRITE_LINE_MEMBER(vector_control_w);
	DECLARE_READ8_MEMBER(joystick_read);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	void init_speedfrk();
	uint32_t screen_update_cinemat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spacewar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void cinemat_vector_callback(int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint8_t shift);
	void ripoff(machine_config &config);
	void wotw(machine_config &config);
	void speedfrk(machine_config &config);
	void starcas(machine_config &config);
	void spacewar(machine_config &config);
	void tailg(machine_config &config);
	void warrior(machine_config &config);
	void starhawk(machine_config &config);
	void barrier(machine_config &config);
	void armora(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void sound_reset() override;

	DECLARE_WRITE_LINE_MEMBER(spacewar_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(spacewar_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(barrier_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_start_led_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(speedfrk_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(starhawk_sound7_w);
	DECLARE_WRITE_LINE_MEMBER(tailg_sound_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(warrior_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(armora_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(ripoff_sound7_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(starcas_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(wotw_sound4_w);

	void cinemat_nojmi_4k(machine_config &config);
	void cinemat_jmi_4k(machine_config &config);
	void cinemat_nojmi_8k(machine_config &config);
	void cinemat_jmi_8k(machine_config &config);
	void cinemat_jmi_16k(machine_config &config);
	void cinemat_jmi_32k(machine_config &config);

	void spacewar_sound(machine_config &config);
	void barrier_sound(machine_config &config);
	void speedfrk_sound(machine_config &config);
	void starhawk_sound(machine_config &config);
	void tailg_sound(machine_config &config);
	void warrior_sound(machine_config &config);
	void armora_sound(machine_config &config);
	void ripoff_sound(machine_config &config);
	void starcas_sound(machine_config &config);
	void wotw_sound(machine_config &config);

	void program_map_4k(address_map &map);
	void program_map_8k(address_map &map);
	void program_map_16k(address_map &map);
	void program_map_32k(address_map &map);
	void data_map(address_map &map);
	void io_map(address_map &map);
};


class cinemat_16level_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void init_sundance();

	void sundance(machine_config &config);

protected:
	virtual DECLARE_WRITE_LINE_MEMBER(vector_control_w) override;
	DECLARE_READ8_MEMBER(sundance_inputs_r);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound4_w);
	DECLARE_WRITE_LINE_MEMBER(sundance_sound7_w);

	void sundance_sound(machine_config &config);
};


class cinemat_64level_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void solarq(machine_config &config);

	void init_solarq();

protected:
	virtual DECLARE_WRITE_LINE_MEMBER(vector_control_w) override;
	DECLARE_WRITE_LINE_MEMBER(solarq_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(solarq_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(solarq_sound4_w);

	void solarq_sound(machine_config &config);

private:
	float m_target_volume;
	float m_current_volume;
};


class cinemat_color_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void init_boxingb();

	void boxingb(machine_config &config);
	void wotwc(machine_config &config);

protected:
	virtual DECLARE_WRITE_LINE_MEMBER(vector_control_w) override;
	DECLARE_READ8_MEMBER(boxingb_dial_r);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound0_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound1_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound2_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound3_w);
	DECLARE_WRITE_LINE_MEMBER(boxingb_sound4_w);

	void boxingb_sound(machine_config &config);
};


class demon_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void demon(machine_config &config);

protected:
	TIMER_CALLBACK_MEMBER(synced_sound_w);
	DECLARE_WRITE_LINE_MEMBER(demon_sound4_w);
	DECLARE_READ8_MEMBER(sound_porta_r);
	DECLARE_READ8_MEMBER(sound_portb_r);
	DECLARE_WRITE8_MEMBER(sound_portb_w);
	DECLARE_WRITE8_MEMBER(sound_output_w);

	virtual void sound_start() override;
	virtual void sound_reset() override;

	void demon_sound(machine_config &config);

	void demon_sound_map(address_map &map);
	void demon_sound_ports(address_map &map);

private:
	uint8_t m_sound_fifo[16];
	uint8_t m_sound_fifo_in;
	uint8_t m_sound_fifo_out;
	uint8_t m_last_portb_write;
};


class qb3_state : public demon_state
{
public:
	using demon_state::demon_state;

	void init_qb3();

	void qb3(machine_config &config);

protected:
	virtual DECLARE_WRITE_LINE_MEMBER(vector_control_w) override;
	DECLARE_READ8_MEMBER(qb3_frame_r);
	DECLARE_WRITE8_MEMBER(qb3_ram_bank_w);
	DECLARE_WRITE8_MEMBER(qb3_sound_fifo_w);

	virtual void sound_reset() override;

	void qb3_sound(machine_config &config);

	void data_map_qb3(address_map &map);
	void io_map_qb3(address_map &map);

private:
	int m_qb3_lastx;
	int m_qb3_lasty;
};

#endif // MAME_INCLUDES_CINEMAT_H
