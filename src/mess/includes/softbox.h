#pragma once

#ifndef __SOFTBOX__
#define __SOFTBOX__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/cbmipt.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ieee488.h"
#include "machine/serial.h"

#define Z80_TAG         "z80"
#define I8251_TAG       "i8251"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define COM8116_TAG     "ic14"
#define RS232_TAG       "rs232"

class softbox_state : public driver_device
{
public:
	softbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_usart(*this, I8251_TAG),
			m_dbrg(*this, COM8116_TAG),
			m_ieee(*this, IEEE488_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<com8116_device> m_dbrg;
	required_device<ieee488_device> m_ieee;

	DECLARE_WRITE8_MEMBER( dbrg_w );
	DECLARE_READ8_MEMBER( pia0_pa_r );
	DECLARE_WRITE8_MEMBER( pia0_pb_w );
	DECLARE_READ8_MEMBER( pia1_pa_r );
	DECLARE_WRITE8_MEMBER( pia1_pb_w );
	DECLARE_READ8_MEMBER( pia1_pc_r );
	DECLARE_WRITE8_MEMBER( pia1_pc_w );
	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( ft_w );
	DECLARE_WRITE8_MEMBER( dummy_w );

	enum
	{
		LED_A,
		LED_B,
		LED_READY
	};
};

#endif
