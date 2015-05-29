// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Acculogic sIDE-1/16

    IDE Disk Controller for IBM PC, XT and compatibles

***************************************************************************/

#pragma once

#ifndef __ISA_SIDE116_H__
#define __ISA_SIDE116_H__

#include "emu.h"
#include "machine/ataintf.h"
#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> side116_device

class side116_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	side116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;
	virtual const rom_entry *device_rom_region() const;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "side116"; }

private:
	required_device<ata_interface_device> m_ata;
	required_ioport m_config;
	UINT8 m_latch;
};


// device type definition
extern const device_type ISA8_SIDE116;

#endif
