// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef MAME_BUS_ISA_SVGA_TSENG_H
#define MAME_BUS_ISA_SVGA_TSENG_H

#pragma once

#include "isa.h"

#include "video/pc_vga_tseng.h"

#include "screen.h"


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

	uint8_t input_port_0_r();

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	isa8_svga_et4k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	void map_io();
	void map_ram();
	void map_rom();
	required_device<tseng_vga_device> m_vga;
};

class isa8_svga_et4k_kasan16_device :
		public isa8_svga_et4k_device
{
public:
	isa8_svga_et4k_kasan16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_hangul_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(ISA8_SVGA_ET4K,         isa8_svga_et4k_device)
DECLARE_DEVICE_TYPE(ISA8_SVGA_ET4K_KASAN16, isa8_svga_et4k_kasan16_device)

#endif // MAME_BUS_ISA_SVGA_TSENG_H
