// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics vector hardware

*************************************************************************/
#ifndef MAME_CINEMATRONICS_CINEMAT_H
#define MAME_CINEMATRONICS_CINEMAT_H

#pragma once

#include "cpu/ccpu/ccpu.h"
#include "cinemat_a.h"
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
		, m_vector(*this, "vector")
		, m_screen(*this, "screen")
		, m_rambase(*this, "rambase")
		, m_inputs(*this, "INPUTS")
		, m_switches(*this, "SWITCHES")
		, m_wheel(*this, "WHEEL")
		, m_analog_x(*this, "ANALOGX")
		, m_analog_y(*this, "ANALOGY")
		, m_led(*this, "led")
		, m_pressed(*this, "pressed%u", 0U)
		, m_coin_detected(0)
		, m_coin_last_reset(0)
		, m_mux_select(0)
		, m_gear(0)
		, m_vector_color(255, 255, 255)
		, m_lastx(0)
		, m_lasty(0)
	{ }

	required_device<ccpu_cpu_device> m_maincpu;
	optional_device<ay8910_device> m_ay1;
	required_device<ls259_device> m_outlatch;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;
	optional_shared_ptr<s16> m_rambase;

	required_ioport m_inputs;
	required_ioport m_switches;
	optional_ioport m_wheel;
	optional_ioport m_analog_x;
	optional_ioport m_analog_y;

	output_finder<> m_led;
	output_finder<10> m_pressed;

	u8 m_coin_detected;
	u8 m_coin_last_reset;
	u8 m_mux_select;
	u8 m_gear;
	rgb_t m_vector_color;
	s16 m_lastx;
	s16 m_lasty;
	u8 inputs_r(offs_t offset);
	u8 switches_r(offs_t offset);
	u8 coin_input_r();
	void coin_reset_w(int state);
	void mux_select_w(int state);
	u8 speedfrk_wheel_r(offs_t offset);
	u8 speedfrk_gear_r(offs_t offset);
	virtual void vector_control_w(int state);
	u8 joystick_read();
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	void init_speedfrk();
	u32 screen_update_cinemat(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_spacewar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void cinemat_vector_callback(s16 sx, s16 sy, s16 ex, s16 ey, u8 shift);
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

	template<int Index>
	void speedfrk_gear_change_w(int state)
	{
		if (state)
			m_gear = Index;
	}

	ioport_value speedfrk_gear_number_r()
	{
		return m_gear;
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void speedfrk_start_led_w(int state);

	void cinemat_nojmi_4k(machine_config &config);
	void cinemat_jmi_4k(machine_config &config);
	void cinemat_nojmi_8k(machine_config &config);
	void cinemat_jmi_8k(machine_config &config);
	void cinemat_jmi_16k(machine_config &config);
	void cinemat_jmi_32k(machine_config &config);

	void program_map_4k(address_map &map) ATTR_COLD;
	void program_map_8k(address_map &map) ATTR_COLD;
	void program_map_16k(address_map &map) ATTR_COLD;
	void program_map_32k(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


class cinemat_16level_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void init_sundance();

	void sundance(machine_config &config);

protected:
	virtual void vector_control_w(int state) override;
	u8 sundance_inputs_r(offs_t offset);
};


class cinemat_64level_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void solarq(machine_config &config);

	void init_solarq();

protected:
	virtual void vector_control_w(int state) override;
};


class cinemat_color_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void init_boxingb();

	void boxingb(machine_config &config);
	void wotwc(machine_config &config);

protected:
	virtual void vector_control_w(int state) override;
	u8 boxingb_dial_r(offs_t offset);
};


class demon_state : public cinemat_state
{
public:
	using cinemat_state::cinemat_state;

	void demon(machine_config &config);

protected:
	TIMER_CALLBACK_MEMBER(synced_sound_w);
	void demon_sound4_w(int state);
	u8 sound_porta_r();
	u8 sound_portb_r();
	void sound_portb_w(u8 data);
	void sound_output_w(u8 data);

	virtual void sound_start() override;
	virtual void sound_reset() override;

	void demon_sound(machine_config &config);

	void demon_sound_map(address_map &map) ATTR_COLD;
	void demon_sound_ports(address_map &map) ATTR_COLD;

private:
	u8 m_sound_fifo[16]{};
	u8 m_sound_fifo_in = 0U;
	u8 m_sound_fifo_out = 0U;
	u8 m_last_portb_write = 0U;
};


class qb3_state : public demon_state
{
public:
	using demon_state::demon_state;

	void init_qb3();

	void qb3(machine_config &config);

protected:
	virtual void vector_control_w(int state) override;
	u8 qb3_frame_r();
	void qb3_ram_bank_w(u8 data);
	void qb3_sound_fifo_w(u8 data);

	virtual void sound_reset() override;

	void qb3_sound(machine_config &config);

	void data_map_qb3(address_map &map) ATTR_COLD;
	void io_map_qb3(address_map &map) ATTR_COLD;

private:
	int m_qb3_lastx = 0;
	int m_qb3_lasty = 0;
};

#endif // MAME_CINEMATRONICS_CINEMAT_H
