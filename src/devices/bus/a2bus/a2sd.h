// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2sd.h

    Implementation of the AppleIISD card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2SD_H
#define MAME_BUS_A2BUS_A2SD_H

#pragma once

#include "a2bus.h"
#include "machine/at28c64b.h"
#include "machine/spi_sdcard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_a2sd_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_a2sd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_a2sd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;
	virtual u8 read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, u8 data) override;

	// SPI 4-wire interface
	void spi_miso_w(int state) { m_in_bit = state; }

	TIMER_CALLBACK_MEMBER(shift_tick);
private:
	required_device<at28c64b_device> m_flash;
	required_device<spi_sdcard_device> m_sdcard;

	u8 m_datain, m_in_latch, m_out_latch;
	u8 m_c0n1, m_c0n3;
	int m_in_bit;

	int m_shift_count;
	emu_timer *m_shift_timer;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_A2SD, a2bus_a2sd_device)

#endif // MAME_BUS_A2BUS_A2SD_H
