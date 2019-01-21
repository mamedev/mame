// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
#pragma once

#ifndef MAME_INCLUDES_VCS80_H
#define MAME_INCLUDES_VCS80_H


#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/ram.h"
#include "machine/bankdev.h"
#include "machine/timer.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define Z80PIO_TAG      "z80pio"

class vcs80_state : public driver_device
{
public:
	vcs80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_pio(*this, Z80PIO_TAG)
		, m_y0(*this, "Y0")
		, m_y1(*this, "Y1")
		, m_y2(*this, "Y2")
		, m_bdmem(*this, "bdmem")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void vcs80(machine_config &config);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_device<address_map_bank_device> m_bdmem;
	output_finder<9> m_digits;

	virtual void machine_start() override;

	DECLARE_READ8_MEMBER( pio_r );
	DECLARE_WRITE8_MEMBER( pio_w );
	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_WRITE8_MEMBER( pio_pb_w );

	uint8_t mem_r(offs_t offset)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		return m_bdmem->read8(offset);
	}

	void mem_w(offs_t offset, uint8_t data)
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		m_bdmem->write8(offset, data);
	}

	DECLARE_READ8_MEMBER( io_r )
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		if ((offset >= 4) && (offset <= 7))
		{
			return pio_r(space, offset-4);
		}
		return 0xff;
	}

	DECLARE_WRITE8_MEMBER( io_w )
	{
		m_pio->port_b_write((!BIT(offset, 0)) << 7);
		if ((offset >= 4) && (offset <= 7))
		{
			pio_w(space, offset-4, data);
		}
	}

	/* keyboard state */
	int m_keylatch;
	int m_keyclk;
	void init_vcs80();
	TIMER_DEVICE_CALLBACK_MEMBER(vcs80_keyboard_tick);

	void vcs80_bd_mem(address_map &map);
	void vcs80_io(address_map &map);
	void vcs80_mem(address_map &map);
};

#endif
