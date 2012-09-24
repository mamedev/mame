#pragma once

#ifndef __COLECO__
#define __COLECO__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "machine/coleco.h"
#include "imagedev/cartslot.h"

#define Z80_TAG		"z80"

class coleco_state : public driver_device
{
public:
	coleco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG)
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( paddle_1_r );
	DECLARE_READ8_MEMBER( paddle_2_r );
	DECLARE_WRITE8_MEMBER( paddle_off_w );
	DECLARE_WRITE8_MEMBER( paddle_on_w );

	int m_joy_mode;
	int m_last_nmi_state;

	// analog controls
	attotime m_joy_pulse_reload[2];
	emu_timer *m_joy_pulse_timer[2];
	emu_timer *m_joy_irq_timer[2];
	emu_timer *m_joy_d7_timer[2];
	int m_joy_irq_state[2];
	int m_joy_d7_state[2];
	UINT8 m_joy_analog_state[2];
	UINT8 m_joy_analog_reload[2];
	TIMER_CALLBACK_MEMBER(paddle_d7reset_callback);
	TIMER_CALLBACK_MEMBER(paddle_irqreset_callback);
	TIMER_CALLBACK_MEMBER(paddle_pulse_callback);
};

#endif
