// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_BUS_ISA_SVGA_TSENG_H
#define MAME_BUS_ISA_SVGA_TSENG_H

#pragma once

#include "isa.h"
#include "video/pc_vga.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_vga_device

class isa8_svga_et4k_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(input_port_0_r);

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void map_io();
	void map_ram();
	void map_rom();
	tseng_vga_device *m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_SVGA_ET4K, isa8_svga_et4k_device)

#endif // MAME_BUS_ISA_SVGA_TSENG_H
