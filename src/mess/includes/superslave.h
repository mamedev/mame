#pragma once

#ifndef __SUPERSLAVE__
#define __SUPERSLAVE__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"

#define Z80_TAG     "z80"

class superslave_state : public driver_device
{
public:
	superslave_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG)
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	virtual void machine_reset();
};

#endif
