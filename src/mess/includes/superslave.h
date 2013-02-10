#pragma once

#ifndef __SUPERSLAVE__
#define __SUPERSLAVE__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/terminal.h"


#define Z80_TAG     "z80"

class superslave_state : public driver_device
{
public:
	superslave_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;

	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port1f_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;

};

#endif
