// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2applicard.h

    Implementation of the PCPI AppliCard Z-80 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2APPLICARD_H
#define MAME_BUS_A2BUS_A2APPLICARD_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_applicard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_applicard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( z80_io_r );
	DECLARE_WRITE8_MEMBER( z80_io_w );

protected:
	a2bus_applicard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	required_device<cpu_device> m_z80;
	bool m_bROMAtZ80Zero;
	bool m_z80stat, m_6502stat;
	uint8_t m_toz80, m_to6502;
	uint8_t m_z80ram[64*1024];
	uint8_t *m_z80rom;

	DECLARE_READ8_MEMBER( dma_r );
	DECLARE_WRITE8_MEMBER( dma_w );

	void z80_io(address_map &map);
	void z80_mem(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_APPLICARD, a2bus_applicard_device)

#endif // MAME_BUS_A2BUS_A2APPLICARD_H
