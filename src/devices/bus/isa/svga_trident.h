// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * svga_trident.h
 *
 *  Created on: 6/09/2014
 */

#ifndef MAME_BUS_ISA_SVGA_TRIDENT_H
#define MAME_BUS_ISA_SVGA_TRIDENT_H

#include "isa.h"
#include "video/pc_vga.h"
#include "bus/isa/trident.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa16_svga_tgui9680_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_tgui9680_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(input_port_0_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<trident_vga_device> m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA16_SVGA_TGUI9680, isa16_svga_tgui9680_device)


#endif // MAME_BUS_ISA_SVGA_TRIDENT_H
