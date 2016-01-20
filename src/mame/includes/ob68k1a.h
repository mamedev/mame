// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __OB68K1A__
#define __OB68K1A__

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/com8116.h"
#include "machine/ram.h"

#define MC68000L10_TAG  "u50"
#define MC6821_0_TAG    "u32"
#define MC6821_1_TAG    "u33"
#define MC6840_TAG      "u35"
#define MC6850_0_TAG    "u34"
#define MC6850_1_TAG    "u26"
#define COM8116_TAG     "u56"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class ob68k1a_state : public driver_device
{
public:
	ob68k1a_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, MC68000L10_TAG),
			m_dbrg(*this, COM8116_TAG),
			m_acia0(*this, MC6850_0_TAG),
			m_acia1(*this, MC6850_1_TAG),
			m_pia0(*this, MC6821_0_TAG),
			m_pia1(*this, MC6821_1_TAG),
			m_rs232a(*this, RS232_A_TAG),
			m_rs232b(*this, RS232_B_TAG),
			m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<com8116_device> m_dbrg;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<ram_device> m_ram;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( com8116_w );
	DECLARE_READ8_MEMBER( pia_r );
	DECLARE_WRITE8_MEMBER( pia_w );
	DECLARE_WRITE_LINE_MEMBER( rx_tx_0_w );
	DECLARE_WRITE_LINE_MEMBER( rx_tx_1_w );
};

#endif
