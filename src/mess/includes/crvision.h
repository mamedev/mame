// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
#pragma once

#ifndef __CRVISION__
#define __CRVISION__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "bus/crvision/slot.h"
#include "bus/crvision/rom.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "sound/wave.h"
#include "video/tms9928a.h"

#define SCREEN_TAG      "screen"
#define M6502_TAG       "u2"
#define TMS9929_TAG     "u3"
#define PIA6821_TAG     "u21"
#define SN76489_TAG     "u22"
#define CENTRONICS_TAG  "centronics"

#define BANK_ROM1       "bank1"
#define BANK_ROM2       "bank2"

class crvision_state : public driver_device
{
public:
	crvision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_pia(*this, PIA6821_TAG),
		m_psg(*this, SN76489_TAG),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_cent_data_out(*this, "cent_data_out"),
		m_ram(*this, RAM_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<sn76496_base_device> m_psg;
	required_device<cassette_image_device> m_cassette;
	required_device<crvision_cart_slot_device> m_cart;
	required_device<output_latch_device> m_cent_data_out;
	required_device<ram_device> m_ram;

	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	UINT8 read_keyboard(int pa);

	/* keyboard state */
	UINT8 m_keylatch;

	/* joystick state */
	UINT8 m_joylatch;
};

class crvision_pal_state : public crvision_state
{
public:
	crvision_pal_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag)
	{ }

	virtual void machine_start();
};

class laser2001_state : public crvision_state
{
public:
	laser2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: crvision_state(mconfig, type, tag),
			m_centronics(*this, CENTRONICS_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_joy0(*this, "JOY0"),
			m_joy1(*this, "JOY1"),
			m_joy2(*this, "JOY2"),
			m_joy3(*this, "JOY3")
	{ }

	virtual void machine_start();

	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_psg_ready );
	DECLARE_READ8_MEMBER( pia_pa_r );
	DECLARE_WRITE8_MEMBER( pia_pa_w );
	DECLARE_READ8_MEMBER( pia_pb_r );
	DECLARE_WRITE8_MEMBER( pia_pb_w );
	DECLARE_READ_LINE_MEMBER( pia_ca1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_ca2_w );
	DECLARE_READ_LINE_MEMBER( pia_cb1_r );
	DECLARE_WRITE_LINE_MEMBER( pia_cb2_w );

	required_device<centronics_device> m_centronics;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_joy0;
	required_ioport m_joy1;
	required_ioport m_joy2;
	required_ioport m_joy3;
	int m_centronics_busy;
	int m_psg_ready;
};

#endif
