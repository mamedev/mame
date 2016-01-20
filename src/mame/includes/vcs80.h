// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
#pragma once

#ifndef __VCS80__
#define __VCS80__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define Z80PIO_TAG      "z80pio"

class vcs80_state : public driver_device
{
public:
	vcs80_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_pio(*this, Z80PIO_TAG),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( pio_r );
	DECLARE_WRITE8_MEMBER( pio_w );
	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_WRITE8_MEMBER( pio_pb_w );

	DECLARE_DIRECT_UPDATE_MEMBER(vcs80_direct_update_handler);
	/* keyboard state */
	int m_keylatch;
	int m_keyclk;
	DECLARE_DRIVER_INIT(vcs80);
	TIMER_DEVICE_CALLBACK_MEMBER(vcs80_keyboard_tick);
};

#endif
