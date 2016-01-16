// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __SUPERSLAVE__
#define __SUPERSLAVE__

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/com8116.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"

#define Z80_TAG         "u45"
#define Z80DART_0_TAG   "u14"
#define Z80DART_1_TAG   "u30"
#define Z80PIO_TAG      "u43"
#define AM9519_TAG      "u13"
#define BR1941_TAG      "u12"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define RS232_C_TAG     "rs232c"
#define RS232_D_TAG     "rs232d"

class superslave_state : public driver_device
{
public:
	superslave_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_dart0(*this, Z80DART_0_TAG),
			m_dart1(*this, Z80DART_1_TAG),
			m_dbrg(*this, BR1941_TAG),
			m_ram(*this, RAM_TAG),
			m_rs232a(*this, RS232_A_TAG),
			m_rs232b(*this, RS232_B_TAG),
			m_rs232c(*this, RS232_C_TAG),
			m_rs232d(*this, RS232_D_TAG),
			m_rom(*this, Z80_TAG),
			m_memctrl(0x01),
			m_cmd(0x01)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80dart_device> m_dart0;
	required_device<z80dart_device> m_dart1;
	required_device<com8116_device> m_dbrg;
	required_device<ram_device> m_ram;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<rs232_port_device> m_rs232c;
	required_device<rs232_port_device> m_rs232d;
	required_memory_region m_rom;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( baud_w );
	DECLARE_WRITE8_MEMBER( memctrl_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( cmd_w );

	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( ft_w );

	UINT8 m_memctrl;
	UINT8 m_cmd;
};

#endif
