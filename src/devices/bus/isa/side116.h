// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Acculogic sIDE-1/16

    IDE Disk Controller for IBM PC, XT and compatibles

***************************************************************************/

#ifndef MAME_BUS_ISA_SIDE116_H
#define MAME_BUS_ISA_SIDE116_H

#pragma once

#include "machine/ataintf.h"
#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> side116_device

class side116_device : public device_t, public device_isa8_card_interface
{
public:
	// construction/destruction
	side116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt );

	required_device<ata_interface_device> m_ata;
	required_ioport m_config;
	uint8_t m_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_SIDE116, side116_device)

#endif // MAME_BUS_ISA_SIDE116_H
