// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_ISA_SVGA_PARADISE_H
#define MAME_BUS_ISA_SVGA_PARADISE_H

#pragma once

#include "isa.h"
#include "video/pc_vga_paradise.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa16_pvga1a_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_pvga1a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void io_isa_map(address_map &map);

private:
	required_device<pvga1a_vga_device> m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_PVGA1A,        isa16_pvga1a_device)

#endif // MAME_BUS_ISA_SVGA_PARADISE_H
