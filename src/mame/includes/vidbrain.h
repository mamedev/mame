// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __VIDBRAIN__
#define __VIDBRAIN__

#include "emu.h"
#include "bus/vidbrain/exp.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "video/uv201.h"

#define F3850_TAG           "cd34"
#define F3853_TAG           "cd5"
#define F3870_TAG           "f3870"
#define UV201_TAG           "uv201"
#define SCREEN_TAG          "screen"
#define DISCRETE_TAG        "discrete"
#define DAC_TAG             "dac"

class vidbrain_state : public driver_device
{
public:
	vidbrain_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, F3850_TAG),
			m_smi(*this, F3853_TAG),
			m_uv(*this, UV201_TAG),
			m_discrete(*this, DISCRETE_TAG),
			m_dac(*this, DAC_TAG),
			m_exp(*this, VIDEOBRAIN_EXPANSION_SLOT_TAG),
			m_io00(*this, "IO00"),
			m_io01(*this, "IO01"),
			m_io02(*this, "IO02"),
			m_io03(*this, "IO03"),
			m_io04(*this, "IO04"),
			m_io05(*this, "IO05"),
			m_io06(*this, "IO06"),
			m_io07(*this, "IO07"),
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

	required_device<cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<uv201_device> m_uv;
	required_device<discrete_sound_device> m_discrete;
	required_device<dac_device> m_dac;
	required_device<videobrain_expansion_slot_device> m_exp;
	required_ioport m_io00;
	required_ioport m_io01;
	required_ioport m_io02;
	required_ioport m_io03;
	required_ioport m_io04;
	required_ioport m_io05;
	required_ioport m_io06;
	required_ioport m_io07;
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
	DECLARE_WRITE8_MEMBER( f3853_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_WRITE_LINE_MEMBER( ext_int_w );
	DECLARE_WRITE_LINE_MEMBER( hblank_w );
	DECLARE_READ8_MEMBER(memory_read_byte);

	F3853_INTERRUPT_REQ_CB(f3853_int_req_w);

	IRQ_CALLBACK_MEMBER(vidbrain_int_ack);

	void interrupt_check();

	// F3853 SMI state
	UINT16 m_vector;
	int m_int_enable;
	int m_ext_int_latch;
	int m_timer_int_latch;

	// keyboard state
	UINT8 m_keylatch;
	int m_joy_enable;

	// sound state
	int m_sound_clk;

	// timers
	emu_timer *m_timer_ne555;
};

//----------- defined in video/vidbrain.c -----------

MACHINE_CONFIG_EXTERN( vidbrain_video );

#endif
