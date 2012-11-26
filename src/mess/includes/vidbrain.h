#pragma once

#ifndef __VIDBRAIN__
#define __VIDBRAIN__

#include "emu.h"
#include "cpu/f8/f8.h"
#include "imagedev/cartslot.h"
#include "machine/f3853.h"
#include "machine/ram.h"
#include "machine/vidbrain_exp.h"
#include "machine/vb_std.h"
#include "machine/vb_money_minder.h"
#include "machine/vb_timeshare.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "video/uv201.h"

#define F3850_TAG			"cd34"
#define F3853_TAG			"cd5"
#define F3870_TAG			"f3870"
#define UV201_TAG			"uv201"
#define SCREEN_TAG			"screen"
#define DISCRETE_TAG		"discrete"
#define DAC_TAG				"dac"

class vidbrain_state : public driver_device
{
public:
	vidbrain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, F3850_TAG),
		  m_smi(*this, F3853_TAG),
		  m_uv(*this, UV201_TAG),
		  m_discrete(*this, DISCRETE_TAG),
	      m_dac(*this, DAC_TAG),
	      m_exp(*this, VIDEOBRAIN_EXPANSION_SLOT_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<f3853_device> m_smi;
	required_device<uv201_device> m_uv;
	required_device<discrete_sound_device> m_discrete;
	required_device<dac_device> m_dac;
	required_device<videobrain_expansion_slot_device> m_exp;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void machine_reset();

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
