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
	vcs80_state(const machine_config &mconfig, device_type type, const char *tag)
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

	uint8_t pio_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	offs_t vcs80_direct_update_handler(direct_read_data &direct, offs_t address);
	/* keyboard state */
	int m_keylatch;
	int m_keyclk;
	void init_vcs80();
	void vcs80_keyboard_tick(timer_device &timer, void *ptr, int32_t param);
};

#endif
