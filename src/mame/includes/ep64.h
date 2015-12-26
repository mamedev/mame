// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight emulation

**********************************************************************/

#pragma once

#ifndef __EP64__
#define __EP64__

#include "audio/dave.h"
#include "bus/rs232/rs232.h"
#include "bus/ep64/exp.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/ram.h"
#include "video/nick.h"

#define Z80_TAG         "u1"
#define DAVE_TAG        "u3"
#define NICK_TAG        "u4"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"
#define CASSETTE1_TAG   "cassette1"
#define CASSETTE2_TAG   "cassette2"
#define SCREEN_TAG      "screen"

class ep64_state : public driver_device
{
public:
	ep64_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_dave(*this, DAVE_TAG),
			m_nick(*this, NICK_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_rs232(*this, RS232_TAG),
			m_cassette1(*this, CASSETTE1_TAG),
			m_cassette2(*this, CASSETTE2_TAG),
			m_cart(*this, "cartslot"),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_y8(*this, "Y8"),
			m_y9(*this, "Y9")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<dave_device> m_dave;
	required_device<nick_device> m_nick;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<generic_slot_device> m_cart;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( rd0_r );
	DECLARE_WRITE8_MEMBER( wr0_w );
	DECLARE_READ8_MEMBER( rd1_r );
	DECLARE_WRITE8_MEMBER( wr2_w );

	UINT8 m_key;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	int m_centronics_busy;
};

#endif
