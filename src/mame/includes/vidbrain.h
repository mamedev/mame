// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef MAME_INCLUDES_VIDBRAIN_H
#define MAME_INCLUDES_VIDBRAIN_H

#include "bus/vidbrain/exp.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "video/uv201.h"

#define F3850_TAG           "cd34"
#define F3853_TAG           "cd5"
#define F3870_TAG           "f3870"
#define UV201_TAG           "uv201"
#define SCREEN_TAG          "screen"

class vidbrain_state : public driver_device
{
public:
	vidbrain_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, F3850_TAG),
		m_smi(*this, F3853_TAG),
		m_uv(*this, UV201_TAG),
		m_dac(*this, "dac"),
		m_exp(*this, VIDEOBRAIN_EXPANSION_SLOT_TAG),
		m_io(*this, "IO%02u", 0),
		m_uv201_31(*this, "UV201-31"),
		m_joy_r(*this, "JOY-R"),
		m_joy1_x(*this, "JOY1-X"),
		m_joy1_y(*this, "JOY1-Y"),
		m_joy2_x(*this, "JOY2-X"),
		m_joy2_y(*this, "JOY2-Y"),
		m_joy3_x(*this, "JOY3-X"),
		m_joy3_y(*this, "JOY3-Y"),
		m_joy4_x(*this, "JOY4-X"),
		m_joy4_y(*this, "JOY4-Y")
	{ }

	void vidbrain(machine_config &config);
	void vidbrain_video(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

private:
	required_device<cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<uv201_device> m_uv;
	required_device<dac_byte_interface> m_dac;
	required_device<videobrain_expansion_slot_device> m_exp;
	required_ioport_array<8> m_io;
	required_ioport m_uv201_31;
	required_ioport m_joy_r;
	required_ioport m_joy1_x;
	required_ioport m_joy1_y;
	required_ioport m_joy2_x;
	required_ioport m_joy2_y;
	required_ioport m_joy3_x;
	required_ioport m_joy3_y;
	required_ioport m_joy4_x;
	required_ioport m_joy4_y;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	enum
	{
		TIMER_JOYSTICK
	};

	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( sound_w );

	DECLARE_WRITE_LINE_MEMBER( ext_int_w );
	DECLARE_WRITE_LINE_MEMBER( hblank_w );
	DECLARE_READ8_MEMBER(memory_read_byte);

	// keyboard state
	uint8_t m_keylatch;
	int m_joy_enable;

	// sound state
	int m_sound_clk;

	// timers
	emu_timer *m_timer_ne555;
	void vidbrain_io(address_map &map);
	void vidbrain_mem(address_map &map);
};

#endif
