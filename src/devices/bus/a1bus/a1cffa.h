// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cffa.h

    Rich Dreher's Compact Flash for Apple I

*********************************************************************/

#ifndef MAME_BUS_A1BUS_A1CFFA_H
#define MAME_BUS_A1BUS_A1CFFA_H

#pragma once

#include "a1bus.h"
#include "machine/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_cffa_device:
		public device_t,
		public device_a1bus_card_interface
{
public:
	// construction/destruction
	a1bus_cffa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(cffa_r);
	DECLARE_WRITE8_MEMBER(cffa_w);

protected:
	a1bus_cffa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<ata_interface_device> m_ata;

private:
	required_region_ptr<uint8_t> m_rom;
	uint16_t m_lastdata;
	bool m_writeprotect;
};

// device type definition
DECLARE_DEVICE_TYPE(A1BUS_CFFA, a1bus_cffa_device)

#endif  // MAME_BUS_A1BUS_A1CFFA_H
