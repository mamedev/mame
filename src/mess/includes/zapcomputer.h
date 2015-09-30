// license:GPL2+
// copyright-holders:FelipeSanches
/*****************************************************************************
 *
 * includes/zapcomputer.h
 *
 ****************************************************************************/

#ifndef __ZAPCOMPUTER__
#define __ZAPCOMPUTER__

#include "emu.h"
#include "cpu/z80/z80.h"

class zapcomp_state : public driver_device
{
public:
	zapcomp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(display_7seg_w);

	virtual void machine_start();
};

#endif // __ZAPCOMPUTER__
