// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __POLY880__
#define __POLY880__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "imagedev/cassette.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "i1"
#define Z80CTC_TAG      "i4"
#define Z80PIO1_TAG     "i2"
#define Z80PIO2_TAG     "i3"

class poly880_state : public driver_device
{
public:
	poly880_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_cassette(*this, "cassette"),
			m_ki1(*this, "KI1"),
			m_ki2(*this, "KI2"),
			m_ki3(*this, "KI3")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_ki1;
	required_ioport m_ki2;
	required_ioport m_ki3;

	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( cldig_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE8_MEMBER( pio1_pa_w );
	DECLARE_READ8_MEMBER( pio1_pb_r );
	DECLARE_WRITE8_MEMBER( pio1_pb_w );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	void update_display();

	/* display state */
	UINT8 m_digit;
	UINT8 m_segment;
};

#endif
