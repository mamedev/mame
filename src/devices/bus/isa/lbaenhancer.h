// license:BSD-3-Clause
// copyright-holders:Bavarese
/**********************************************************************

    VS Systems LBA Enhancer (ISA; 1995).

 **********************************************************************/

#ifndef MAME_BUS_ISA_LBA_ENHANCER_H
#define MAME_BUS_ISA_LBA_ENHANCER_H

#pragma once

#include "isa.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class lba_enhancer_device : public device_t,
			   public device_isa8_card_interface
{
public:
	// construction/destruction
	lba_enhancer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
private:
	uint32_t m_current_rom_start;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_LBA_ENHANCER, lba_enhancer_device)

#endif // MAME_BUS_ISA_LBA_ENHANCER_H
