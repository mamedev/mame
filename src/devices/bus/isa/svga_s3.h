// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_BUS_ISA_SVGA_S3_H
#define MAME_BUS_ISA_SVGA_S3_H

#pragma once

#include "isa.h"
#include "video/pc_vga_s3.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa16_vga_device

class isa16_svga_s3_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_svga_s3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t input_port_0_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<s3trio64_vga_device> m_vga;
	required_device<ibm8514a_device> m_8514;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA16_SVGA_S3,    isa16_svga_s3_device)

#endif // MAME_BUS_ISA_SVGA_S3_H
